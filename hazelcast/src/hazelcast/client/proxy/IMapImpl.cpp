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
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/util/Util.h"

// Includes for parameters classes
#include "hazelcast/client/protocol/codec/MapPutCodec.h"
#include "hazelcast/client/protocol/codec/MapGetCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MapReplaceCodec.h"
#include "hazelcast/client/protocol/codec/MapReplaceIfSameCodec.h"
#include "hazelcast/client/protocol/codec/MapPutAsyncCodec.h"
#include "hazelcast/client/protocol/codec/MapGetAsyncCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveAsyncCodec.h"
#include "hazelcast/client/protocol/codec/MapContainsKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapContainsValueCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveIfSameCodec.h"
#include "hazelcast/client/protocol/codec/MapDeleteCodec.h"
#include "hazelcast/client/protocol/codec/MapFlushCodec.h"
#include "hazelcast/client/protocol/codec/MapTryRemoveCodec.h"
#include "hazelcast/client/protocol/codec/MapTryPutCodec.h"
#include "hazelcast/client/protocol/codec/MapPutTransientCodec.h"
#include "hazelcast/client/protocol/codec/MapPutIfAbsentCodec.h"
#include "hazelcast/client/protocol/codec/MapSetCodec.h"
#include "hazelcast/client/protocol/codec/MapLockCodec.h"
#include "hazelcast/client/protocol/codec/MapTryLockCodec.h"
#include "hazelcast/client/protocol/codec/MapIsLockedCodec.h"
#include "hazelcast/client/protocol/codec/MapUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MapForceUnlockCodec.h"
#include "hazelcast/client/protocol/codec/MapAddInterceptorCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveInterceptorCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerToKeyWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapAddEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapAddNearCacheEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapRemoveEntryListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapAddPartitionLostListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapRemovePartitionLostListenerCodec.h"
#include "hazelcast/client/protocol/codec/MapGetEntryViewCodec.h"
#include "hazelcast/client/protocol/codec/MapEvictCodec.h"
#include "hazelcast/client/protocol/codec/MapEvictAllCodec.h"
#include "hazelcast/client/protocol/codec/MapLoadAllCodec.h"
#include "hazelcast/client/protocol/codec/MapLoadGivenKeysCodec.h"
#include "hazelcast/client/protocol/codec/MapKeySetCodec.h"
#include "hazelcast/client/protocol/codec/MapGetAllCodec.h"
#include "hazelcast/client/protocol/codec/MapValuesCodec.h"
#include "hazelcast/client/protocol/codec/MapEntrySetCodec.h"
#include "hazelcast/client/protocol/codec/MapKeySetWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapValuesWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapEntriesWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapAddIndexCodec.h"
#include "hazelcast/client/protocol/codec/MapSizeCodec.h"
#include "hazelcast/client/protocol/codec/MapIsEmptyCodec.h"
#include "hazelcast/client/protocol/codec/MapPutAllCodec.h"
#include "hazelcast/client/protocol/codec/MapClearCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapSubmitToKeyCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnAllKeysCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteWithPredicateCodec.h"
#include "hazelcast/client/protocol/codec/MapExecuteOnKeysCodec.h"

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
                        protocol::codec::MapContainsKeyCodec::RequestParameters::encode(getName(), key,
                                                                                        util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapContainsKeyCodec>(request, partitionId);
            }

            bool IMapImpl::containsValue(const serialization::pimpl::Data &value) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapContainsValueCodec::RequestParameters::encode(getName(), value);

                return invokeAndGetResult<bool, protocol::codec::MapContainsValueCodec>(request);
            }

            serialization::pimpl::Data IMapImpl::get(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapGetCodec>(
                        request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data &key,
                                                     const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutCodec::RequestParameters::encode(getName(), key, value,
                                                                                util::getThreadId(), 0);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapPutCodec::ResponseParameters>(
                        request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::remove(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapRemoveCodec>(
                        request, partitionId);
            }

            bool IMapImpl::remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveIfSameCodec::RequestParameters::encode(getName(), key, value,
                                                                                         util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapRemoveIfSameCodec>(request, partitionId);
            }

            void IMapImpl::deleteEntry(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapDeleteCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void IMapImpl::flush() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapFlushCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            bool IMapImpl::tryRemove(const serialization::pimpl::Data &key, long timeoutInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryRemoveCodec::RequestParameters::encode(getName(), key,
                                                                                      util::getThreadId(),
                                                                                      timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryRemoveCodec::ResponseParameters>(request,
                                                                                                        partitionId);
            }

            bool IMapImpl::tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  long timeoutInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryPutCodec::RequestParameters::encode(getName(), key, value,
                                                                                   util::getThreadId(),
                                                                                   timeoutInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryPutCodec::ResponseParameters>(request,
                                                                                                     partitionId);
            }

            serialization::pimpl::Data IMapImpl::put(const serialization::pimpl::Data &key,
                                                     const serialization::pimpl::Data &value, long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutCodec::RequestParameters::encode(getName(), key, value,
                                                                                util::getThreadId(),
                                                                                ttlInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapPutCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IMapImpl::putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                        long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutTransientCodec::RequestParameters::encode(getName(), key, value,
                                                                                         util::getThreadId(),
                                                                                         ttlInMillis);

                invoke(request, partitionId);
            }

            serialization::pimpl::Data IMapImpl::putIfAbsent(const serialization::pimpl::Data &key,
                                                             const serialization::pimpl::Data &value,
                                                             long ttlInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapPutIfAbsentCodec::RequestParameters::encode(getName(), key, value,
                                                                                        util::getThreadId(),
                                                                                        ttlInMillis);

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapPutIfAbsentCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IMapImpl::replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                                   const serialization::pimpl::Data &newValue) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceIfSameCodec::RequestParameters::encode(getName(), key, oldValue,
                                                                                          newValue,
                                                                                          util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapReplaceIfSameCodec::ResponseParameters>(request,
                                                                                                            partitionId);
            }

            serialization::pimpl::Data IMapImpl::replace(const serialization::pimpl::Data &key,
                                                         const serialization::pimpl::Data &value) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapReplaceCodec::RequestParameters::encode(getName(), key, value,
                                                                                    util::getThreadId());

                return *invokeAndGetResult<std::auto_ptr<serialization::pimpl::Data>, protocol::codec::MapReplaceCodec::ResponseParameters>(
                        request, partitionId);
            }

            void IMapImpl::set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                               long ttl) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapSetCodec::RequestParameters::encode(getName(), key, value,
                                                                                util::getThreadId(), ttl);

                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                 -1);

                invoke(request, partitionId);
            }

            void IMapImpl::lock(const serialization::pimpl::Data &key, long leaseTime) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                 leaseTime);

                invoke(request, partitionId);
            }

            bool IMapImpl::isLocked(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapIsLockedCodec::RequestParameters::encode(getName(), key);

                return invokeAndGetResult<bool, protocol::codec::MapIsLockedCodec::ResponseParameters>(request,
                                                                                                       partitionId);
            }

            bool IMapImpl::tryLock(const serialization::pimpl::Data &key, long timeInMillis) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapTryLockCodec::RequestParameters::encode(getName(), key, util::getThreadId(),
                                                                                    -1, timeInMillis);

                return invokeAndGetResult<bool, protocol::codec::MapTryLockCodec::ResponseParameters>(request,
                                                                                                      partitionId);
            }

            void IMapImpl::unlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapUnlockCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                invoke(request, partitionId);
            }

            void IMapImpl::forceUnlock(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapForceUnlockCodec::RequestParameters::encode(getName(), key);

                invoke(request, partitionId);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue) {
                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                std::auto_ptr<protocol::codec::IAddListenerCodec> codec(
                        new protocol::codec::MapAddEntryListenerCodec(getName(), includeValue, EntryEventType::ALL,
                                                                      false));

                return registerListener(codec, entryEventHandler);
            }

            bool IMapImpl::removeEntryListener(const std::string &registrationId) {
                protocol::codec::MapRemoveEntryListenerCodec codec(getName(), registrationId);

                return context->getServerListenerService().deRegisterListener(codec);
            }

            std::string IMapImpl::addEntryListener(impl::BaseEventHandler *handler,
                                                   const serialization::pimpl::Data &key, bool includeValue) {

                int partitionId = getPartitionId(key);

                // TODO: Use appropriate flags for the event type as implemented in Java instead of EntryEventType::ALL
                std::auto_ptr<protocol::codec::IAddListenerCodec> codec(
                        new protocol::codec::MapAddEntryListenerToKeyCodec(getName(), key, includeValue,
                                                                           EntryEventType::ALL, false));

                return registerListener(codec, partitionId, handler);
            }

            map::DataEntryView IMapImpl::getEntryView(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapGetEntryViewCodec::RequestParameters::encode(getName(), key,
                                                                                         util::getThreadId());

                return invokeAndGetResult<map::DataEntryView, protocol::codec::MapGetEntryViewCodec::ResponseParameters>(
                        request, partitionId);
            }

            bool IMapImpl::evict(const serialization::pimpl::Data &key) {
                int partitionId = getPartitionId(key);

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictCodec::RequestParameters::encode(getName(), key, util::getThreadId());

                return invokeAndGetResult<bool, protocol::codec::MapEvictCodec::ResponseParameters>(request,
                                                                                                    partitionId);
            }

            void IMapImpl::evictAll() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapEvictAllCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            EntryVector IMapImpl::getAll(
                    const std::vector<serialization::pimpl::Data> &keys) {
                std::map<int, std::vector<serialization::pimpl::Data> > partitionedKeys;

                // group the request per parition id
                for (std::vector<serialization::pimpl::Data>::const_iterator it = keys.begin();
                     it != keys.end(); ++it) {
                    int partitionId = getPartitionId(*it);

                    partitionedKeys[partitionId].push_back(*it);
                }

                std::vector<connection::CallFuture> futures;

                for (std::map<int, std::vector<serialization::pimpl::Data> >::const_iterator it = partitionedKeys.begin();
                     it != partitionedKeys.end(); ++it) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapGetAllCodec::RequestParameters::encode(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                EntryVector result;
                // wait for all futures
                for (std::vector<connection::CallFuture>::const_iterator it = futures.begin();
                     it != futures.end(); ++it) {
                    try {
                        std::auto_ptr<protocol::ClientMessage> responseForPartition = it->get();
                        protocol::codec::MapGetAllCodec::ResponseParameters resultForPartition = protocol::codec::MapGetAllCodec::ResponseParameters::decode(
                                *responseForPartition);
                        result.insert(result.end(), resultForPartition.entrySet.begin(),
                                      resultForPartition.entrySet.end());
                    } catch (...) {
                        throw;
                    }
                }

                return result;
            }

            std::vector<serialization::pimpl::Data> IMapImpl::keySet(
                    const std::string &sql) {

                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapKeySetWithPredicateCodec::RequestParameters::encode(getName(),
                                                                                                toData<std::string>(
                                                                                                        sql));

                return *invokeAndGetResult<std::auto_ptr<std::vector<serialization::pimpl::Data> >, protocol::codec::MapKeySetWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::entrySet(
                    const std::string &sql) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapEntriesWithPredicateCodec::RequestParameters::encode(
                        getName(), toData<std::string>(sql));

                return invokeAndGetResult<EntryVector, protocol::codec::MapEntriesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            EntryVector IMapImpl::values(
                    const std::string &sql) {

                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapValuesWithPredicateCodec::RequestParameters::encode(
                        getName(), toData<std::string>(sql));

                // TODO: Check if this does copy or if it optimizes to eliminate copy !!!
                return invokeAndGetResult<EntryVector, protocol::codec::MapValuesWithPredicateCodec::ResponseParameters>(
                        request);
            }

            void IMapImpl::addIndex(const std::string &attribute, bool ordered) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddIndexCodec::RequestParameters::encode(getName(), attribute, ordered);

                invoke(request);
            }

            int IMapImpl::size() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapSizeCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<int, protocol::codec::MapSizeCodec::ResponseParameters>(request);
            }

            bool IMapImpl::isEmpty() {
                std::auto_ptr<protocol::ClientMessage> request = protocol::codec::MapIsEmptyCodec::RequestParameters::encode(
                        getName());

                return invokeAndGetResult<bool>(request);
            }

            void IMapImpl::putAll(const EntryVector& entries) {
                std::map<int, EntryMap> partitionedEntries;

                // group the request per parition id
                for (EntryVector::const_iterator it = entries.begin();
                     it != entries.end(); ++it) {
                    int partitionId = getPartitionId(it->first);

                    partitionedEntries[partitionId][it->first] = it->second;
                }

                std::vector<connection::CallFuture> futures;

                for (std::map<int, EntryMap>::const_iterator it = partitionedEntries.begin();
                     it != partitionedEntries.end(); ++it) {
                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::codec::MapPutAllCodec::RequestParameters::encode(getName(), it->second);

                    futures.push_back(invokeAndGetFuture(request, it->first));
                }

                // wait for all futures
                for (std::vector<connection::CallFuture>::const_iterator it = futures.begin();
                     it != futures.end(); ++it) {
                    try {
                        std::auto_ptr<protocol::ClientMessage> responseForPartition = it->get();
                    } catch (...) {
                        throw;
                    }
                }
            }

            void IMapImpl::clear() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapClearCodec::RequestParameters::encode(getName());

                invoke(request);
            }

            std::string IMapImpl::addInterceptor(serialization::Portable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::RequestParameters::encode(
                                getName(), toData<serialization::Portable>(interceptor));

                return invokeAndGetResult<std::string>(request);
            }

            std::string IMapImpl::addInterceptor(serialization::IdentifiedDataSerializable &interceptor) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapAddInterceptorCodec::RequestParameters::encode(
                                getName(), toData<serialization::IdentifiedDataSerializable>(interceptor));

                return invokeAndGetResult<std::string>(request);
            }

            void IMapImpl::removeInterceptor(const std::string &id) {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::codec::MapRemoveInterceptorCodec::RequestParameters::encode(getName(), id);

                invoke(request);
            }

        }
    }
}

