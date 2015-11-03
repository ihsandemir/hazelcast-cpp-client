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
// Created by sancar koyunlu on 30/09/14.
//

#include "hazelcast/client/proxy/MultiMapImpl.h"
















#include "hazelcast/client/impl/PortableCollection.h"

#include "hazelcast/client/multimap/PortableEntrySetResponse.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/util/Util.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/MultiMapPutParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapGetParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapKeySetParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapValuesParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapEntrySetParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapContainsKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapContainsValueParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapContainsEntryParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapSizeParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapClearParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapCountParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapAddEntryListenerToKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapAddEntryListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapRemoveEntryListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapLockParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapTryLockParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapIsLockedParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapForceUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapRemoveEntryParameters.h"
#include "hazelcast/client/protocol/parameters/MultiMapValueCountParameters.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:multiMapService", instanceName, context) {

            }

            bool MultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapPutParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::get(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapGetParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapRemoveEntryParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::remove(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapRemoveParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::keySet() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapKeySetParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::values() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapValuesParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            void MultiMapImpl::entrySet(std::auto_ptr<protocol::DataArray> &resultKeys,
                                    std::auto_ptr<protocol::DataArray> &resultValues) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapEntrySetParameters::encode(getName());

                std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                resultKeys = response->getDataList();
                resultValues = response->getDataList();
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapContainsKeyParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapContainsValueParameters::encode(getName(), value);

                return invokeAndGetResult<bool>(request);
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapContainsEntryParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            int MultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::MultiMapSizeParameters::encode(getName());

                return invokeAndGetResult<int>(request);
            }

            void MultiMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapClearParameters::encode(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapValueCountParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<int>(request, partitionId);
            }

            std::auto_ptr<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapAddEntryListenerParameters::encode(getName(), includeValue);

                return listen(request, entryEventHandler);
            }

            std::auto_ptr<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                   const serialization::pimpl::Data& key, bool includeValue) {

                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapAddEntryListenerToKeyParameters::encode(getName(), key, includeValue);

                return listen(request, partitionId, entryEventHandler);
            }

            bool MultiMapImpl::removeEntryListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::MultiMapRemoveEntryListenerParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapLockParameters::encode(getName(), key, util::getThreadId(), -1);

                invoke(request, partitionId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapLockParameters::encode(getName(), key, util::getThreadId(), leaseTime);

                invoke(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapIsLockedParameters::encode(getName(), key);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapTryLockParameters::encode(getName(), key, util::getThreadId(), 0);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key, long timeInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapTryLockParameters::encode(getName(), key, util::getThreadId(), timeInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapUnlockParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MultiMapForceUnlockParameters::encode(getName(), key);

                invoke(request, partitionId);
            }
        }
    }
}

