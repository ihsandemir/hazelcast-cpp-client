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
// Created by sancar koyunlu on 01/10/14.
//

#include "hazelcast/client/protocol/parameters/GenericResultParameters.h"
#include "hazelcast/client/protocol/parameters/DestroyProxyParameters.h"
#include "hazelcast/client/protocol/parameters/EntryViewParameters.h"
#include "hazelcast/client/protocol/parameters/DataCollectionResultParameters.h"
#include "hazelcast/client/proxy/ProxyImpl.h"

#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/connection/CallFuture.h"


namespace hazelcast {
    namespace client {
        namespace proxy {

            ProxyImpl::ProxyImpl(const std::string& serviceName, const std::string& objectName, spi::ClientContext *context)
            : DistributedObject(serviceName, objectName)
            , context(context) {

            }

            ProxyImpl::~ProxyImpl() {

            }

            std::auto_ptr<std::string> ProxyImpl::listen(
                    std::auto_ptr<protocol::ClientMessage> registrationRequest, int partitionId,
                    impl::BaseEventHandler *handler) {
                return context->getServerListenerService().listen(registrationRequest, partitionId, handler);
            }

            std::auto_ptr<std::string> ProxyImpl::listen(std::auto_ptr<protocol::ClientMessage> registrationRequest, impl::BaseEventHandler *handler) {
                return context->getServerListenerService().listen(registrationRequest, handler);
            }

            int ProxyImpl::getPartitionId(const serialization::pimpl::Data& key) {
                return context->getPartitionService().getPartitionId(key);
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request, int partitionId) {
                spi::InvocationService& invocationService = context->getInvocationService();
                connection::CallFuture future = invocationService.invokeOnPartitionOwner(request, partitionId);
                return future.get();
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request) {
                connection::CallFuture future = context->getInvocationService().invokeOnRandomTarget(request);
                return future.get();
            }

            void ProxyImpl::destroy() {
                onDestroy();
                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::DestroyProxyParameters::encode(
                        DistributedObject::getName(), DistributedObject::getServiceName());

                context->getInvocationService().invokeOnRandomTarget(request);
            }

            std::auto_ptr<protocol::ClientMessage> ProxyImpl::invoke(std::auto_ptr<protocol::ClientMessage> request,
                                                                     boost::shared_ptr<connection::Connection> conn) {
                connection::CallFuture future = context->getInvocationService().invokeOnConnection(request, conn);
                return future.get();
            }

            std::auto_ptr<protocol::DataArray> ProxyImpl::invokeAndGetResult(
                    std::auto_ptr<protocol::ClientMessage> request) {
                std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                return protocol::parameters::DataCollectionResultParameters::decode(*response)->values;
            }

            template<>
            bool ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getBoolean();
            }

            template<>
            int ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getInt32();
            }

            template<>
            long ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return (long)response->getInt64();
            }

            template<>
            std::auto_ptr<std::string> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                return response->getStringUtf8();
            }

            template<>
            std::auto_ptr<serialization::pimpl::Data> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                std::auto_ptr<protocol::parameters::GenericResultParameters> resultParameters =
                        protocol::parameters::GenericResultParameters::decode(*response);

                std::auto_ptr<serialization::pimpl::Data> result = resultParameters->data;

                return result;
            }

            template<>
            std::auto_ptr<protocol::DataArray> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                std::auto_ptr<protocol::parameters::DataCollectionResultParameters> resultParameters =
                        protocol::parameters::DataCollectionResultParameters::decode(*response);

                std::auto_ptr<protocol::DataArray> result = resultParameters->values;

                return result;
            }

            template<>
            std::auto_ptr<map::DataEntryView> ProxyImpl::getResponseResult(std::auto_ptr<protocol::ClientMessage> response) {
                std::auto_ptr<protocol::parameters::EntryViewParameters> resultParameters =
                        protocol::parameters::EntryViewParameters::decode(*response);

                return resultParameters->entryView;
            }

            std::auto_ptr<protocol::DataArray> ProxyImpl::getDataList(protocol::ClientMessage *response) const {
                return response->getDataList();
            }
        }
    }
}

