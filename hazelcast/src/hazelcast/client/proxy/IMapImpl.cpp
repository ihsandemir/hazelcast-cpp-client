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
// Created by sancar koyunlu on 29/09/14.
//

#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/impl/MapKeySet.h"
#include "hazelcast/client/impl/MapEntrySet.h"
#include "hazelcast/client/impl/MapValueCollection.h"
#include "hazelcast/client/EntryView.h"

#include "hazelcast/client/protocol/ClientMessage.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/parameters/MapPutParameters.h"
#include "hazelcast/client/protocol/parameters/MapGetParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/MapReplaceParameters.h"
#include "hazelcast/client/protocol/parameters/MapReplaceIfSameParameters.h"
#include "hazelcast/client/protocol/parameters/MapPutAsyncParameters.h"
#include "hazelcast/client/protocol/parameters/MapGetAsyncParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemoveAsyncParameters.h"
#include "hazelcast/client/protocol/parameters/MapContainsKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MapContainsValueParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemoveIfSameParameters.h"
#include "hazelcast/client/protocol/parameters/MapDeleteParameters.h"
#include "hazelcast/client/protocol/parameters/MapFlushParameters.h"
#include "hazelcast/client/protocol/parameters/MapTryRemoveParameters.h"
#include "hazelcast/client/protocol/parameters/MapTryPutParameters.h"
#include "hazelcast/client/protocol/parameters/MapPutTransientParameters.h"
#include "hazelcast/client/protocol/parameters/MapPutIfAbsentParameters.h"
#include "hazelcast/client/protocol/parameters/MapSetParameters.h"
#include "hazelcast/client/protocol/parameters/MapLockParameters.h"
#include "hazelcast/client/protocol/parameters/MapTryLockParameters.h"
#include "hazelcast/client/protocol/parameters/MapIsLockedParameters.h"
#include "hazelcast/client/protocol/parameters/MapUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/MapForceUnlockParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddInterceptorParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemoveInterceptorParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddEntryListenerToKeyWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddEntryListenerWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddEntryListenerToKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddEntryListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddNearCacheEntryListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemoveEntryListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddPartitionLostListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MapRemovePartitionLostListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MapGetEntryViewParameters.h"
#include "hazelcast/client/protocol/parameters/MapEvictParameters.h"
#include "hazelcast/client/protocol/parameters/MapEvictAllParameters.h"
#include "hazelcast/client/protocol/parameters/MapLoadAllParameters.h"
#include "hazelcast/client/protocol/parameters/MapLoadGivenKeysParameters.h"
#include "hazelcast/client/protocol/parameters/MapKeySetParameters.h"
#include "hazelcast/client/protocol/parameters/MapGetAllParameters.h"
#include "hazelcast/client/protocol/parameters/MapValuesParameters.h"
#include "hazelcast/client/protocol/parameters/MapEntrySetParameters.h"
#include "hazelcast/client/protocol/parameters/MapKeySetWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapValuesWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapEntriesWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapAddIndexParameters.h"
#include "hazelcast/client/protocol/parameters/MapSizeParameters.h"
#include "hazelcast/client/protocol/parameters/MapIsEmptyParameters.h"
#include "hazelcast/client/protocol/parameters/MapPutAllParameters.h"
#include "hazelcast/client/protocol/parameters/MapClearParameters.h"
#include "hazelcast/client/protocol/parameters/MapExecuteOnKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MapSubmitToKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MapExecuteOnAllKeysParameters.h"
#include "hazelcast/client/protocol/parameters/MapExecuteWithPredicateParameters.h"
#include "hazelcast/client/protocol/parameters/MapExecuteOnKeysParameters.h"

#include <climits>

namespace hazelcast {
    namespace client {
        namespace proxy {
            const std::string IMapImpl::VALUE_ITERATION_TYPE = "VALUE";
            const std::string IMapImpl::KEY_ITERATION_TYPE = "KEY";
            const std::string IMapImpl::ENTRY_ITERATION_TYPE = "ENTRY";
            const std::string IMapImpl::NO_PREDICATE = "";

            IMapImpl::IMapImpl(const std::string &instanceName, spi::ClientContext *context)
                    : ProxyImpl("hz:impl:mapService", instanceName, context) {

            }

            bool IMapImpl::containsKey(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapContainsKeyParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapContainsValueParameters::encode(getName(), value);

                return invokeAndGetResult<bool>(request);
            }

            serialization::pimpl::Data IMapImpl::get(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapGetParameters::encode(getName(), key, util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data &key,
                                                     const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapPutParameters::encode(getName(), key, value, util::getThreadId(), 0);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::remove(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapRemoveParameters::encode(getName(), key, util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            bool IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapRemoveIfSameParameters::encode(getName(), key, value, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapDeleteParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void IMapImpl::flush() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapFlushParameters::encode(getName());

                invoke(request);
            }

            bool IMapImpl::tryRemove(const serialization::pimpl::Data &key, long timeoutInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapTryRemoveParameters::encode(getName(), key, util::getThreadId(),
                                                                             timeoutInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  long timeoutInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapTryPutParameters::encode(getName(), key, value, util::getThreadId(),
                                                                          timeoutInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data &key,
                                                     const serialization::pimpl::Data &value, long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapPutParameters::encode(getName(), key, value, util::getThreadId(),
                                                                       ttlInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            void IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapPutTransientParameters::encode(getName(), key, value,
                                                                                util::getThreadId(), ttlInMillis);

                invoke(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::putIfAbsent(const serialization::pimpl::Data &key,
                                                             const serialization::pimpl::Data &value,
                                                             long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapPutIfAbsentParameters::encode(getName(), key, value,
                                                                               util::getThreadId(), ttlInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            bool IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapReplaceIfSameParameters::encode(getName(), key, oldValue, newValue,
                                                                                 util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::replace(const serialization::pimpl::Data &key,
                                                         const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapReplaceParameters::encode(getName(), key, value, util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data> >(request, partitionId);
            }

            void IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                               long ttl) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapSetParameters::encode(getName(), key, value, util::getThreadId(), ttl);

                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapLockParameters::encode(getName(), key, util::getThreadId(), -1);

                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapLockParameters::encode(getName(), key, util::getThreadId(), leaseTime);

                invoke(request, partitionId);
            }

            bool IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapIsLockedParameters::encode(getName(), key);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            bool IMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapTryLockParameters::encode(getName(), key, util::getThreadId(),
                                                                        timeInMillis);

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapUnlockParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapForceUnlockParameters::encode(getName(), key);

                invoke(request, partitionId);
            }

            std::auto_ptr<std::string> IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapAddEntryListenerParameters::encode(getName(), includeValue);

                return listen(request, entryEventHandler);
            }

            bool IMapImpl::removeEntryListener(const std::string &registrationId) {
                bool result = false;

                std::string effectiveRegistrationId = registrationId;
                if (context->getServerListenerService().deRegisterListener(effectiveRegistrationId)) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::MapRemoveEntryListenerParameters::encode(getName(), effectiveRegistrationId);

                    result = invokeAndGetResult<bool>(request);
                }

                return result;
            }

            std::auto_ptr<std::string> IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler,
                                                   const serialization::pimpl::Data &key, bool includeValue) {

                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapAddEntryListenerToKeyParameters::encode(getName(), key, includeValue);

                return listen(request, partitionId, entryEventHandler);
            }

            std::auto_ptr<map::DataEntryView> IMapImpl::getEntryView(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapGetEntryViewParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<std::auto_ptr<map::DataEntryView> >(request, partitionId);
            }

            bool IMapImpl::evict(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapEvictParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool>(request, partitionId);
            }

            void IMapImpl::evictAll() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapEvictAllParameters::encode(getName());

                invoke(request);
            }

            void IMapImpl::getAll(const std::vector<serialization::pimpl::Data> &keys,
                                  std::auto_ptr<protocol::DataArray> &resultKeys,
                                  std::auto_ptr<protocol::DataArray> &resultValue) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapGetAllParameters::encode(getName(), keys);

                std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                resultKeys = response->getDataList();
                resultValue = response->getDataList();
            }

            std::auto_ptr<protocol::DataArray> IMapImpl::keySet(const std::string &sql) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapKeySetWithPredicateParameters::encode(getName(),
                                                                                       toData<std::string>(sql));

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            void IMapImpl::entrySet(const std::string &sql,
                                    std::auto_ptr<protocol::DataArray> &resultKeys,
                                    std::auto_ptr<protocol::DataArray> &resultValue) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::MapEntriesWithPredicateParameters::encode(
                        getName(), toData<std::string>(sql));

                std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                resultKeys = response->getDataList();
                resultValue = response->getDataList();
            }

            std::auto_ptr<protocol::DataArray> IMapImpl::values(const std::string &sql) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::MapValuesWithPredicateParameters::encode(
                        getName(), toData<std::string>(sql));

                return invokeAndGetResult<std::auto_ptr<protocol::DataArray> >(request);
            }

            void IMapImpl::addIndex(const std::string &attribute, bool ordered) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapAddIndexParameters::encode(getName(), attribute, ordered);

                invoke(request);
            }

            int IMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::MapSizeParameters::encode(
                        getName());

                return invokeAndGetResult<int>(request);
            }

            bool IMapImpl::isEmpty() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::parameters::MapIsEmptyParameters::encode(
                        getName());

                return invokeAndGetResult<bool>(request);
            }

            void IMapImpl::putAll(const protocol::DataArray &keys,
                                  const protocol::DataArray &values) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapPutAllParameters::encode(getName(), keys, values);

                invoke(request);
            }

            void IMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapClearParameters::encode(getName());

                invoke(request);
            }

            std::auto_ptr<std::string> IMapImpl::addInterceptor(serialization::Portable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapAddInterceptorParameters::encode(
                                getName(), toData<serialization::Portable>(interceptor));

                return invokeAndGetResult<std::auto_ptr<std::string> >(request);
            }

            std::auto_ptr<std::string> IMapImpl::addInterceptor(serialization::IdentifiedDataSerializable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapAddInterceptorParameters::encode(
                                getName(), toData<serialization::IdentifiedDataSerializable>(interceptor));

                return invokeAndGetResult<std::auto_ptr<std::string> >(request);
            }

            void IMapImpl::removeInterceptor(const std::string &id) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::MapRemoveInterceptorParameters::encode(getName(), id);

                invoke(request);
            }

        }
    }
}

