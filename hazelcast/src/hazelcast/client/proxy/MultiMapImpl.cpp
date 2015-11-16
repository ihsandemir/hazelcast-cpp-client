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
#include "hazelcast/client/protocol/codec/MultiMapPutCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapGetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapKeySetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapValuesCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapEntrySetCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsKeyCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsValueCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapContainsEntryCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapSizeCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapClearCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapCountCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapAddEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapLockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapTryLockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapIsLockedCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapForceUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapRemoveEntryCodec.h"
#include "hazelcast/client/protocol/codec/MultiMapValueCountCodec.h"

namespace hazelcast {
    namespace client {
        namespace proxy {
            MultiMapImpl::MultiMapImpl(const std::string& instanceName, spi::ClientContext *context)
            : ProxyImpl("hz:impl:multiMapService", instanceName, context) {

            }

            bool MultiMapImpl::put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapPutCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::get(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapGetCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            bool MultiMapImpl::remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveEntryCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::remove(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapRemoveCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request, partitionId);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::keySet() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapKeySetCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            std::auto_ptr<protocol::DataArray> MultiMapImpl::values() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValuesCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            void MultiMapImpl::entrySet(std::auto_ptr<protocol::DataArray> &resultKeys,
                                    std::auto_ptr<protocol::DataArray> &resultValues) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapEntrySetCodec::RequestParameters::encode(getName());

                std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                resultKeys = response->getDataList();
                resultValues = response->getDataList();
            }

            bool MultiMapImpl::containsKey(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsKeyCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::containsValue(const serialization::pimpl::Data& value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsValueCodec::RequestParameters::encode(getName(), value);

                return invokeAndGetResult<bool>(request);
            }

            bool MultiMapImpl::containsEntry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapContainsEntryCodec::RequestParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            int MultiMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MultiMapSizeCodec::RequestParameters::encode(getName());

                return invokeAndGetResult<int>(request);
            }

            void MultiMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapClearCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            int MultiMapImpl::valueCount(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapValueCountCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<int>(request, partitionId);
            }

            std::auto_ptr<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapAddEntryListenerCodec::RequestParameters::encode(getName(), includeValue);

                return registerListener(request, entryEventHandler);
            }

            std::auto_ptr<std::string> MultiMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                   const serialization::pimpl::Data& key, bool includeValue) {

                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapAddEntryListenerToKeyCodec::RequestParameters::encode(getName(), key, includeValue);

                return registerListener(request, partitionId, entryEventHandler, 0, NULL);
            }

            bool MultiMapImpl::removeEntryListener(const std::string& registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MultiMapRemoveEntryListenerCodec::RequestParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), -1);

                invoke(request, partitionId);
            }

            void MultiMapImpl::lock(const serialization::pimpl::Data& key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), leaseTime);

                invoke(request, partitionId);
            }


            bool MultiMapImpl::isLocked(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapIsLockedCodec::RequestParameters::encode(getName(), key);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), 0);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool MultiMapImpl::tryLock(const serialization::pimpl::Data& key, long timeInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(), timeInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void MultiMapImpl::unlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapUnlockCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void MultiMapImpl::forceUnlock(const serialization::pimpl::Data& key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MultiMapForceUnlockCodec::RequestParameters::encode(getName(), key);

                invoke(request, partitionId);
            }
        }
    }
}

