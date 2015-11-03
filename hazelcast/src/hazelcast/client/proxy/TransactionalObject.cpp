/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 12/11/13.



#include "hazelcast/client/protocol/parameters/DestroyProxyParameters.h"
#include "hazelcast/client/protocol/parameters/DataCollectionResultParameters.h"
#include "hazelcast/client/proxy/TransactionalObject.h"
#include "hazelcast/client/protocol/ClientMessage.h"


namespace hazelcast {
    namespace client {
        namespace proxy {
            #define MILLISECONDS_IN_A_SECOND 1000

            TransactionalObject::TransactionalObject(const std::string& serviceName, const std::string& objectName, txn::TransactionProxy *context)
            : serviceName(serviceName), name(objectName), context(context) {

            }

            TransactionalObject::~TransactionalObject() {

            }

            const std::string& TransactionalObject::getServiceName() {
                return serviceName;
            }

            const std::string& TransactionalObject::getName() {
                return name;
            }

            void TransactionalObject::destroy() {
                onDestroy();

                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::DestroyProxyParameters::encode(
                        name, serviceName);

                spi::InvocationService& invocationService = context->getInvocationService();
                invocationService.invokeOnConnection(request, context->getConnection());
            }

            void TransactionalObject::onDestroy() {

            }

            std::string TransactionalObject::getTransactionId() const {
                return context->getTxnId();
            }


            int TransactionalObject::getTimeoutInMilliseconds() const {
                return context->getTimeoutSeconds() * MILLISECONDS_IN_A_SECOND;
            }

            std::auto_ptr<protocol::ClientMessage> TransactionalObject::invoke(
                    std::auto_ptr<protocol::ClientMessage> request) {

                connection::CallFuture future = context->getInvocationService().invokeOnConnection(
                        request, context->getConnection());

                return future.get();
            }

            template<>
            bool TransactionalObject::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getBoolean();
            }

            template<>
            int TransactionalObject::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getInt32();
            }

            template<>
            std::auto_ptr<std::string> TransactionalObject::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getStringUtf8();
            }

            template<>
            std::auto_ptr<serialization::pimpl::Data> TransactionalObject::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                std::auto_ptr<protocol::parameters::GenericResultParameters> resultParameters =
                        protocol::parameters::GenericResultParameters::decode(*response);

                return resultParameters->data;
            }

            template<>
            std::auto_ptr<protocol::DataArray> TransactionalObject::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                std::auto_ptr<protocol::parameters::DataCollectionResultParameters> resultParameters =
                        protocol::parameters::DataCollectionResultParameters::decode(*response);

                return resultParameters->values;
            }
        }
    }
}

