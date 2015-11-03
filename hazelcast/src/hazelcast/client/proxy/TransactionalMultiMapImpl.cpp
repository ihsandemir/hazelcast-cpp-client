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

#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/TransactionalMultiMapImpl.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapPutParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapGetParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapRemoveEntryParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapValueCountParameters.h"
#include "hazelcast/client/protocol/parameters/TransactionalMultiMapSizeParameters.h"

namespace hazelcast {
    namespace client {
        namespace proxy {

            TransactionalMultiMapImpl::TransactionalMultiMapImpl(const std::string& name, txn::TransactionProxy *transactionProxy)
            : TransactionalObject("hz:impl:multiMapService", name, transactionProxy) {

            }

            bool TransactionalMultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapPutParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool>(request);
            }

            std::auto_ptr<protocol::DataArray> TransactionalMultiMapImpl::get(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapGetParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);

            }

            bool TransactionalMultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapRemoveEntryParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key, value);

                return invokeAndGetResult<bool>(request);

            }

            std::auto_ptr<protocol::DataArray> TransactionalMultiMapImpl::remove(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapRemoveParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);

            }

            int TransactionalMultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapValueCountParameters::encode(
                                getName(), getTransactionId(), util::getThreadId(), key);

                return invokeAndGetResult<int>(request);
            }

            int TransactionalMultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::TransactionalMultiMapSizeParameters::encode(
                                getName(), getTransactionId(), util::getThreadId());

                return invokeAndGetResult<int>(request);
            }

        }
    }
}
