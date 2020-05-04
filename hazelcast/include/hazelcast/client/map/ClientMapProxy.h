/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <stdexcept>
#include <climits>
#include <assert.h>

#include "hazelcast/client/monitor/LocalMapStats.h"
#include "hazelcast/client/monitor/impl/NearCacheStatsImpl.h"
#include "hazelcast/client/monitor/impl/LocalMapStatsImpl.h"
#include "hazelcast/client/proxy/IMapImpl.h"
#include "hazelcast/client/impl/EntryEventHandler.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/EntryView.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/impl/ClientMessageDecoder.h"
#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace map {
            class HAZELCAST_API ClientMapProxy : public proxy::IMapImpl {
            public:
                ClientMapProxy(const std::string &instanceName, spi::ClientContext *context)
                        : proxy::IMapImpl(instanceName, context) {}

                template<typename K>
                boost::future<bool> containsKey(const K &key) {
                    return containsKeyInternal(toData(key));
                }

                template<typename V>
                boost::future<bool> containsValue(const V &value) {
                    return proxy::IMapImpl::containsValue(toData(value));
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> get(const K &key) {
                    return toObject<V>(getInternal(toData(key)));
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> put(const K &key, const V &value) {
                    return put(key, value, UNSET);
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> put(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                    return toObject<V>(putInternal(toData(key), toData(value), ttl));
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> remove(const K &key) {
                    return toObject<V>(removeInternal(toData(key)));
                }

                template<typename K, typename V>
                boost::future<bool> remove(const K &key, const V &value) {
                    return removeInternal(toData(key), toData(value));
                }

                template <typename P>
                boost::future<void> removeAll(const P &predicate) {
                    return toVoidFuture(removeAllInternal(toData<P>(predicate)));
                }

                template <typename K>
                boost::future<void> deleteEntry(const K &key) {
                    return toVoidFuture(deleteInternal(toData(key)));
                }

                template<typename K>
                boost::future<bool> tryRemove(const K &key, std::chrono::steady_clock::duration timeout) {
                    return tryRemoveInternal(toData(key), timeout);
                }

                template<typename K, typename V>
                boost::future<bool> tryPut(const K &key, const V &value, std::chrono::steady_clock::duration timeout) {
                    return tryPutInternal(toData(key), toData(value), timeout);
                }

                template<typename K, typename V>
                boost::future<void> putTransient(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                    return toVoidFuture(tryPutTransientInternal(toData(key), toData(value), ttl));
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value) {
                    return putIfAbsent(key, value, UNSET);
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                    return toObject<V>(putIfAbsentInternal(toData(key), toData(value), ttl));
                }

                template<typename K, typename V>
                boost::future<bool> replace(const K &key, const V &oldValue, const V &newValue) {
                    return replaceIfSameInternal(toData(key), toData(oldValue), toData(newValue));
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> replace(const K &key, const V &value) {
                    return toObject<V>(replaceInternal(toData(key), toData(value)));
                }

                template<typename K, typename V>
                boost::future<void> set(const K &key, const V &value) {
                    return toVoidFuture(set(key, value, UNSET));
                }

                template<typename K, typename V>
                boost::future<void> set(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                    return toVoidFuture(setInternal(toData(key), toData(value), ttl));
                }

                template<typename K>
                boost::future<void> lock(const K &key) {
                    return toVoidFuture(lock(key, UNSET));
                }

                template<typename K>
                boost::future<void> lock(const K &key, std::chrono::steady_clock::duration leaseTime) {
                    return toVoidFuture(proxy::IMapImpl::lock(toData(key), leaseTime));
                }

                template<typename K>
                boost::future<bool> isLocked(const K &key) {
                    return proxy::IMapImpl::isLocked(toData(key));
                }

                template<typename K>
                boost::future<bool> tryLock(const K &key) {
                    return tryLock(key, std::chrono::milliseconds(0));
                }

                template<typename K>
                boost::future<bool> tryLock(const K &key, std::chrono::steady_clock::duration timeout) {
                    return proxy::IMapImpl::tryLock(toData(key), timeout);
                }

                template<typename K>
                boost::future<void> unlock(const K &key) {
                    return toVoidFuture(proxy::IMapImpl::unlock(toData(key)));
                }

                template<typename K>
                boost::future<void> forceUnlock(const K &key) {
                    return toVoidFuture(proxy::IMapImpl::forceUnlock(toData(key)));
                }

                template<typename MapInterceptor>
                boost::future<std::string> addInterceptor(MapInterceptor &interceptor) {
                    return proxy::IMapImpl::addInterceptor(toData(interceptor));
                }

                template<typename Listener>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(
                                    new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                            getName(), getContext().getClientClusterService(),
                                            getContext().getSerializationService(),
                                            listener,
                                            includeValue, getContext().getLogger())), includeValue);
                }

                template<typename Listener, typename P>
                std::string
                addEntryListener(Listener &&listener, const P &predicate, bool includeValue) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(
                                    new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                            getName(), getContext().getClientClusterService(),
                                            getContext().getSerializationService(),
                                            listener,
                                            includeValue, getContext().getLogger())), toData<P>(predicate), includeValue);
                }

                template<typename Listener, typename K>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue, const K &key) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(
                                    new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                            getName(), getContext().getClientClusterService(),
                                            getContext().getSerializationService(),
                                            listener,
                                            includeValue, getContext().getLogger())), includeValue, toData<K>(key));
                }

                template<typename K, typename V>
                boost::future<boost::optional<EntryView<K, V>>> getEntryView(const K &key) {
                    toObject(proxy::IMapImpl::getEntryViewData(toData(key))).then([=] (boost::future<boost::optional<DataEntryView>> f) {
                        auto dataView = f.get();
                        if (!dataView.has_value()) {
                            return boost::none;
                        }
                        auto v = toObject<V>(dataView.value().getValue());
                        return boost::make_optional(EntryView<K, V>(key, v, dataView.value()));
                    });
                }

                template<typename K>
                boost::future<bool> evict(const K &key) {
                    return evictInternal(toData(key));
                }

                template<typename K, typename V>
                boost::future<std::unordered_map<K, boost::optional<V>>> getAll(const std::set<K> &keys) {
                    if (keys.empty()) {
                        return boost::none;
                    }

                    std::unordered_map<int, std::vector<serialization::pimpl::Data>> partitionToKeyData;
                    // group the request per parition id
                    for (auto &key : keys) {
                        auto keyData = toData<K>(key);
                        auto partitionId = getPartitionId(keyData);
                        partitionToKeyData[partitionId].push_back(std::move(keyData));
                    }

                    std::vector<boost::future<EntryVector>> futures;
                    for (auto &entry : partitionToKeyData) {
                        futures.push_back(getAllInternal(entry.first, std::move(entry.second)));
                    }

                    return boost::when_all(futures.begin(), futures.end()).then(boost::launch::deferred,
                                                                                [=](std::vector<boost::future<EntryVector>> &resultsData) {
                        std::unordered_map<K, boost::optional<V>> result;
                        for (auto &entryVectorFuture : resultsData) {
                            for(auto &entry : entryVectorFuture.get()) {
                                result[toObject<K>(entry.first).value()] = toObject<V>(entry.second);
                            }
                        }
                        return result;
                    });
                }

                template<typename K>
                boost::future<std::vector<K>> keySet() {
                    return toObjectVector<K>(proxy::IMapImpl::keySetData());
                }

                template<typename K, typename P>
                boost::future<std::vector<K>> keySet(const P &predicate) {
                    return toObjectVector<K>(proxy::IMapImpl::keySetData(toData(predicate)));
                }

                template<typename K, typename V>
                boost::future<std::vector<K>> keySet(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::IterationType::KEY);
                    return toObjectVector<K>(keySetForPagingPredicateData(toData(predicate))).then(boost::launch::deferred, [&](boost::future<std::vector<K>> f) {
                       auto keys = f.get();
                        std::vector<std::pair<K, boost::optional<V>>> entries;
                        for (auto &key : keys) {
                            entries.push_back(std::make_pair(std::move(key), boost::none));
                        }
                       auto resultEntries = sortAndGet(predicate, query::IterationType::KEY, entries);
                        std::vector<K> result;
                        result.reserve(resultEntries.size());
                        for (auto &entry : resultEntries) {
                            result.push_back(std::move(entry.first));
                        }
                        return result;
                    });
                }

                template<typename V>
                boost::future<std::vector<boost::optional<V>>> values() {
                    return toObjectVector<V>(proxy::IMapImpl::valuesData());
                }

                template<typename V, typename P>
                boost::future<std::vector<boost::optional<V>>> values(const P &predicate) {
                    return toObjectVector<V>(proxy::IMapImpl::valuesData(toData(predicate)));
                }

                template<typename K, typename V>
                boost::future<std::vector<boost::optional<V>>> values(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::IterationType::VALUE);
                    return toEntryObjectVector<K, V>(valuesForPagingPredicateData(toData(predicate))).then(boost::launch::deferred, [&](boost::future<EntryVector> f) {
                        auto entries = f.get();
                        auto resultEntries = sortAndGet(predicate, query::IterationType::VALUE, entries);
                        std::vector<V> result;
                        result.reserve(resultEntries.size());
                        for (auto &entry : resultEntries) {
                            result.push_back(std::move(entry.second));
                        }
                        return result;
                    });
                }

                template<typename K, typename V>
                boost::future<std::vector<std::pair<K, boost::optional<V>>>> entrySet() {
                    return toEntryObjectVector<K,V>(proxy::IMapImpl::entrySetData());
                }

                template<typename K, typename V, typename P>
                boost::future<std::vector<std::pair<K, boost::optional<V>>>> entrySet(const P &predicate) {
                    return toEntryObjectVector<K,V>(proxy::IMapImpl::entrySetData(toData(predicate)));
                }

                template<typename K, typename V>
                boost::future<std::vector<std::pair<K, boost::optional<V>>>> entrySet(query::PagingPredicate<K, V> &predicate) {
                    return toSortedEntryObjectVector<K,V>(proxy::IMapImpl::entrySetForPagingPredicateData(toData(predicate)));
                }

                boost::future<void> addIndex(const std::string &attribute, bool ordered) {
                    return toVoidFuture(proxy::IMapImpl::addIndex(attribute, ordered));
                }

                boost::future<void> clear() {
                    return toVoidFuture(proxy::IMapImpl::clear());
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<boost::optional<ResultType>> executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                    return toObject<ResultType>(executeOnKeyInternal(toData(key), toData(entryProcessor)));
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<boost::optional<ResultType>>
                submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                    return toObject<ResultType>(submitToKeyInternal(toData(key), toData(entryProcessor)));
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<std::unordered_map<K, boost::optional<ResultType>>>
                executeOnKeys(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    return toObjectMap<K, ResultType>(executeOnKeysInternal<EntryProcessor>(keys, entryProcessor));
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<std::unordered_map<K, boost::optional<ResultType>>> executeOnEntries(const EntryProcessor &entryProcessor) {
                    return toObjectMap<K, ResultType>(proxy::IMapImpl::executeOnEntriesData(toData(entryProcessor)));
                }

                template<typename K, typename ResultType, typename EntryProcessor, typename P>
                boost::future<std::unordered_map<K, boost::optional<ResultType>>>
                executeOnEntries(const EntryProcessor &entryProcessor, const P &predicate) {
                    toObjectMap<K, ResultType>(proxy::IMapImpl::executeOnEntriesData(toData(entryProcessor),
                                                                                toData(predicate)));
                }

                template<typename K, typename V>
                boost::future<void> putAll(const std::unordered_map<K, V> &entries) {
                    std::unordered_map<int, EntryVector> entryMap;
                    for (auto &entry : entries) {
                        serialization::pimpl::Data keyData = toData(entry.first);
                        int partitionId = getPartitionId(keyData);
                        entryMap[partitionId].push_back(std::make_pair(keyData, toData(entry.second)));
                    }

                    std::unordered_map<int, boost::future<protocol::ClientMessage>> resultFutures;
                    for (auto &partitionEntry : entryMap) {
                        auto partitionId = partitionEntry.first;
                        resultFutures[partitionId] = putAllInternal(partitionId, partitionEntry.second);
                    }
                    return boost::when_all(resultFutures.begin(), resultFutures.end()).then(boost::launch::deferred,
                                                                                            [](std::vector<boost::future<protocol::ClientMessage>> futures) {
                        for (auto &f : futures) {
                            f.get();
                        }
                    });
                }

                monitor::LocalMapStats &getLocalMapStats() {
                    return localMapStats;
                }
            protected:
                /**
                 * Default TTL value of a record.
                 */
                static constexpr std::chrono::milliseconds UNSET{-1};

                virtual boost::future<serialization::pimpl::Data> getInternal(serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::getData(keyData);
                }

                virtual boost::future<bool> containsKeyInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::containsKey(keyData);
                }

                virtual boost::future<serialization::pimpl::Data> removeInternal(
                        const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::removeData(keyData);
                }

                virtual boost::future<bool> removeInternal(
                        const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData) {
                    return proxy::IMapImpl::remove(keyData, valueData);
                }

                virtual boost::future<protocol::ClientMessage> removeAllInternal(const serialization::pimpl::Data &predicateData) {
                    return proxy::IMapImpl::removeAll(predicateData);
                }

                virtual boost::future<protocol::ClientMessage> deleteInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::deleteEntry(keyData);
                }

                virtual boost::future<bool> tryRemoveInternal(const serialization::pimpl::Data &keyData, std::chrono::steady_clock::duration timeout) {
                    return proxy::IMapImpl::tryRemove(keyData, timeout);
                }

                virtual boost::future<bool> tryPutInternal(const serialization::pimpl::Data &keyData,
                                            const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration timeout) {
                    return proxy::IMapImpl::tryPut(keyData, valueData, timeout);
                }

                virtual boost::future<serialization::pimpl::Data> putInternal(const serialization::pimpl::Data &keyData,
                                                                              const serialization::pimpl::Data &valueData,
                                                                              std::chrono::steady_clock::duration ttl) {
                    return proxy::IMapImpl::putData(keyData, valueData, ttl);
                }

                virtual void tryPutTransientInternal(const serialization::pimpl::Data &keyData,
                                                     const serialization::pimpl::Data &valueData, std::chrono::steady_clock::duration ttl) {
                    proxy::IMapImpl::putTransient(keyData, valueData, ttl);
                }

                virtual boost::future<serialization::pimpl::Data>
                putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    std::chrono::steady_clock::duration ttl) {
                    return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttl);
                }

                virtual boost::future<bool> replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                   const serialization::pimpl::Data &valueData,
                                                   const serialization::pimpl::Data &newValueData) {
                    return proxy::IMapImpl::replace(keyData, valueData, newValueData);
                }

                virtual boost::future<serialization::pimpl::Data>
                replaceInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) {
                    return proxy::IMapImpl::replaceData(keyData, valueData);

                }

                virtual boost::future<protocol::ClientMessage>
                setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            std::chrono::steady_clock::duration ttl) {
                    return proxy::IMapImpl::set(keyData, valueData, ttl);
                }

                virtual boost::future<bool> evictInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::evict(keyData);
                }

                virtual boost::future<EntryVector>
                getAllInternal(int partitionId, std::vector<serialization::pimpl::Data> &&partitionKeys) {
                    return proxy::IMapImpl::getAllData(partitionId,
                            std::forward<std::vector<serialization::pimpl::Data>>(partitionKeys));
                }

                virtual boost::future<serialization::pimpl::Data>
                executeOnKeyInternal(const serialization::pimpl::Data &keyData,
                                     const serialization::pimpl::Data &processor) {
                    return proxy::IMapImpl::executeOnKeyData(keyData, processor);
                }

                boost::future<serialization::pimpl::Data>
                submitToKeyInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &processor) {
                    return submitToKeyData(keyData, processor);
                }

                template<typename K, typename EntryProcessor>
                boost::future<EntryVector> executeOnKeysInternal(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    if (keys.empty()) {
                        return boost::make_ready_future(EntryVector());
                    }
                    std::vector<serialization::pimpl::Data> keysData;
                    std::for_each(keys.begin(), keys.end(), [&](const K &key) { keysData.push_back(toData<K>(key)); });
                    return proxy::IMapImpl::executeOnKeysData(keysData, toData<EntryProcessor>(entryProcessor));
                }

                virtual boost::future<protocol::ClientMessage>
                putAllInternal(int partitionId, const EntryVector &entries) {
                    return proxy::IMapImpl::putAllData(partitionId, entries);
                }

            private:
                monitor::impl::LocalMapStatsImpl localMapStats;

                template<typename K, typename V>
                boost::future<std::vector<std::pair<K, boost::optional<V>>>> toSortedEntryObjectVector(query::PagingPredicate<K, V> &predicate, query::IterationType iterationType) {
                    return toEntryObjectVector<K, V>(proxy::IMapImpl::entrySetForPagingPredicateData(toData(predicate))).then(
                            boost::launch::deferred, [&](boost::future<std::vector<std::pair<K, boost::optional<V>>>> f) {
                                return sortAndGet<K, V>(predicate, iterationType, f.get());
                            });
                }

                template<typename K, typename V>
                std::vector<std::pair<K, boost::optional<V>>> sortAndGet(query::PagingPredicate<K, V> &predicate, query::IterationType iterationType, std::vector<std::pair<K, boost::optional<V>>> entries) {
                    std::sort(entries.begin(), entries.end(), [&] (const std::pair<K, boost::optional<V>> &lhs, const std::pair<K, boost::optional<V>> &rhs) {
                        auto comparator = predicate.getComparator();
                        if (!comparator) {
                            switch(predicate.getIterationType()) {
                                case query::IterationType::VALUE:
                                    return lhs.second < rhs.second;
                                default:
                                    return lhs.first < rhs.first;
                            }
                        }

                        std::pair<const K *, const V *> leftVal(&lhs.first, &lhs.second);
                        std::pair<const K *, const V *> rightVal(&rhs.first, &rhs.second);
                        int result = comparator->compare(&leftVal, &rightVal);
                        if (0 != result) {
                            // std sort: comparison function object returns ​true if the first argument is less
                            // than (i.e. is ordered before) the second.
                            return result < 0;
                        }

                        return lhs.first < rhs.first;
                    });

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, iterationType);

                    std::vector<std::pair<K, V>> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        std::pair<const K *, const V *> entry = entries[i];
                        result.push_back(std::pair<K, V>(*entry.first, *entry.second));
                    }
                    return result;
                }
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

