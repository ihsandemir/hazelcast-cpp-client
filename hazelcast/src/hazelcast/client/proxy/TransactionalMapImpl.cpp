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

#include "hazelcast/client/proxy/TransactionalMapImpl.h"

#include "hazelcast/client/spi/ClusterService.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/TransactionalMapContainsKeyParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapGetParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapGetForUpdateParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapSizeParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapIsEmptyParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapPutParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapSetParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapPutIfAbsentParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapReplaceParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapReplaceIfSameParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapDeleteParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapRemoveIfSameParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapKeySetParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapKeySetWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapValuesParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMapValuesWithPredicateParameters.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            TransactionalMapImpl::TransactionalMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:mapService", name, transactionProxy) {

            }

            bool TransactionalMapImpl::containsKey(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapContainsKeyParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<bool>(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::get(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapGetParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request);
            }

            int TransactionalMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapSizeParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<int>(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::put(
                    const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapPutParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value, getTimeoutInMilliseconds());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request);

            }

            void TransactionalMapImpl::set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapSetParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                invoke(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapPutIfAbsentParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapReplaceParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request);
            }

            bool TransactionalMapImpl::replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapReplaceIfSameParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, oldValue, newValue);

                return invokeAndGetResult<bool>(request);
            }

            serialization::pimpl::Data TransactionalMapImpl::remove(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapRemoveParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request);
            }

            void TransactionalMapImpl::deleteEntry(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapDeleteParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                invoke(request);
            }

            bool TransactionalMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapRemoveIfSameParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool>(request);
            }

            std::auto_ptr<protocol::DataArray> TransactionalMapImpl::keySet() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapKeySetParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            std::auto_ptr<protocol::DataArray> TransactionalMapImpl::keySet(const std::string& predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapKeySetWithPredicateParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), toData<std::string>(predicate));

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            std::auto_ptr<protocol::DataArray> TransactionalMapImpl::values() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapValuesParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            std::auto_ptr<protocol::DataArray> TransactionalMapImpl::values(const std::string& predicate) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMapValuesWithPredicateParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), toData<std::string>(predicate));

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }
        }
    }
}

