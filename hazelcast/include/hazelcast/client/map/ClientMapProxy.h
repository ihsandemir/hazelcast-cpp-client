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
#ifndef HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_
#define HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_

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
                        : proxy::IMapImpl(instanceName, context) {
                }

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
                    serialization::pimpl::Data keyData = toData(key);
                    return getInternal(keyData);
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> put(const K &key, const V &value) {
                    return put(key, value, std::chrono::milliseconds(-1));
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
                    removeAllInternal(toData<P>(predicate));
                }

                template <typename K>
                boost::future<void> deleteEntry(const K &key) {
                    return deleteInternal(toData(key));
                }

                boost::future<void> flush() {
                    return proxy::IMapImpl::flush();
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
                    tryPutTransientInternal(toData(key), toData(value), ttl);
                }

                template<typename K, typename V>
                boost::future<boost::optional<V>> putIfAbsent(const K &key, const V &value) {
                    return putIfAbsent(key, value, std::chrono::milliseconds(-1));
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
                    return set(key, value, -1);
                }

                template<typename K, typename V>
                boost::future<void> set(const K &key, const V &value, std::chrono::steady_clock::duration ttl) {
                    return setInternal(toData(key), toData(value), ttl);
                }

                template<typename K>
                boost::future<void> lock(const K &key) {
                    return lock(key, std::chrono::milliseconds(-1));
                }

                template<typename K>
                boost::future<void> lock(const K &key, std::chrono::steady_clock::duration leaseTime) {
                    return proxy::IMapImpl::lock(toData(key), leaseTime);
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
                    return proxy::IMapImpl::unlock(toData(key));
                }

                template<typename K>
                boost::future<void> forceUnlock(const K &key) {
                    proxy::IMapImpl::forceUnlock(toData(key));
                }

                template<typename MapInterceptor>
                boost::future<std::string> addInterceptor(MapInterceptor &interceptor) {
                    return proxy::IMapImpl::addInterceptor(toData(interceptor));
                }

                boost::future<void> removeInterceptor(const std::string &id) {
                    proxy::IMapImpl::removeInterceptor(id);
                }

                template<typename Listener>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(
                                    new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                                            getName(), getContext().getClientClusterService(),
                                            getContext().getSerializationService(),
                                            listener,
                                            includeValue)), includeValue);
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
                                            includeValue)), toData<P>(predicate), includeValue);
                }

                template<typename Listener, typename K>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue, const K &key) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(
                                    new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                            getName(), getContext().getClientClusterService(),
                                            getContext().getSerializationService(),
                                            listener,
                                            includeValue)), includeValue, toData<K>(key));
                }

                boost::future<bool> removeEntryListener(const std::string &registrationId) {
                    return proxy::IMapImpl::removeEntryListener(registrationId);
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

                boost::future<void> evictAll() {
                    return proxy::IMapImpl::evictAll();
                }

                template<typename K, typename V>
                boost::future<std::unordered_map<K, V>> getAll(const std::set<K> &keys) {
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
                        std::unordered_map<K, V> result;
                        for (auto &entryVectorFuture : resultsData) {
                            for(auto &entry : entryVectorFuture.get()) {
                                result[toObject<K>(entry.first).value()] = toObject<V>(entry.second).value();
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
                    predicate.setIterationType(query::KEY);
                    keySetForPagingPredicateData(toData(predicate)).then([] () {});

                    EntryVector entryResult;
                    for (std::vector<serialization::pimpl::Data>::iterator it = dataResult.begin();
                         it != dataResult.end(); ++it) {
                        entryResult.push_back(std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(*it,
                                                                                                                serialization::pimpl::Data()));
                    }

                    client::impl::EntryArrayImpl<K, V> entries(entryResult, getContext().getSerializationService());
                    entries.sort(query::KEY, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::KEY);

                    std::vector<K> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        result.push_back(*entries.getKey(i));
                    }

                    return result;
                }

                template<typename V>
                boost::future<std::vector<V>> values() {
                    return toObjectVector<V>(proxy::IMapImpl::valuesData());
                }

                template<typename V, typename P>
                boost::future<std::vector<V>> values(const P &predicate) {
                    return toObjectVector<V>(proxy::IMapImpl::valuesData(toData(predicate)));
                }

                template<typename K, typename V>
                std::vector<V> values(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::VALUE);
                    EntryVector dataResult = proxy::IMapImpl::valuesForPagingPredicateData(toData(predicate));

                    client::impl::EntryArrayImpl<K, V> entries(dataResult, getContext().getSerializationService());

                    entries.sort(query::VALUE, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::VALUE);

                    std::vector<V> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        result.push_back(*entries.getValue(i));
                    }
                    return result;
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
                std::vector<std::pair<K, V> > entrySet(query::PagingPredicate<K, V> &predicate) {
                    auto dataResult = proxy::IMapImpl::entrySetForPagingPredicateData(toData(predicate));

                    std::vector<std::pair<K, V> > entries;
                    std::for_each(dataResult.begin(), dataResult.end(),
                                  [&](const std::pair<serialization::pimpl::Data, serialization::pimpl::Data> &entry) {
                        entries.push_back(std::make_pair(toObject<K>(entry.first).value(), toObject<V>(entry.second).value()));
                    });
                    std::sort(entries.begin(), entries.end(), [&] (const std::pair<K, V> &lhs, const std::pair<K, V> &rhs) {
                        auto comparator = predicate.getComparator();
                        if (!comparator) {
                            switch(predicate.getIterationType()) {
                                case query::VALUE:
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

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::ENTRY);

                    std::vector<std::pair<K, V> > result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        std::pair<const K *, const V *> entry = entries[i];
                        result.push_back(std::pair<K, V>(*entry.first, *entry.second));
                    }
                    return result;
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<boost::optional<ResultType>> executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                    return toObject<ResultType>(executeOnKeyInternal(toData(key), toData(entryProcessor)));
                }

                template<typename K, typename ResultType, typename EntryProcessor>
                boost::future<boost::optional<ResultType>>
                submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                    return toObject<ResultType>(submitToKeyInternal<ResultType>(toData(key), toData(entryProcessor)));
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

                /**
                * @deprecated This API is deprecated in favor of
                * @sa{executeOnEntries(const EntryProcessor &entryProcessor, const query::Predicate &predicate)}
                *
                * Applies the user defined EntryProcessor to the all entries in the map.
                * Returns the results mapped by each key in the map.
                *
                *
                * EntryProcessor should extend either Portable or IdentifiedSerializable.
                * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
                * with same classId and factoryId.
                *
                * @tparam ResultType that entry processor will return
                * @tparam EntryProcessor type of entry processor class
                * @tparam predicate The filter to apply for selecting the entries at the server side.
                * @param entryProcessor that will be applied
                */
                template<typename ResultType, typename EntryProcessor>
                std::map<K, std::shared_ptr<ResultType> > executeOnEntries(const EntryProcessor &entryProcessor,
                                                                             const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return executeOnEntries<ResultType, EntryProcessor>(entryProcessor, *p);
                }

                /**
                * Applies the user defined EntryProcessor to the all entries in the map.
                * Returns the results mapped by each key in the map.
                *
                *
                * EntryProcessor should extend either Portable or IdentifiedSerializable.
                * Notice that map EntryProcessor runs on the nodes. Because of that, same class should be implemented in java side
                * with same classId and factoryId.
                *
                * @tparam ResultType that entry processor will return
                * @tparam EntryProcessor type of entry processor class
                * @tparam predicate The filter to apply for selecting the entries at the server side.
                * @param entryProcessor that will be applied
                */
                template<typename ResultType, typename EntryProcessor>
                std::map<K, std::shared_ptr<ResultType> >
                executeOnEntries(const EntryProcessor &entryProcessor, const query::Predicate &predicate) {
                    EntryVector entries = proxy::IMapImpl::executeOnEntriesData<EntryProcessor>(entryProcessor,
                                                                                                predicate);
                    std::map<K, std::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::unique_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::unique_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = std::move(resObj);
                    }
                    return result;
                }

                /**
                * Returns the number of key-value mappings in this map.  If the
                * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
                * <tt>Integer.MAX_VALUE</tt>.
                *
                * @return the number of key-value mappings in this map
                */
                int size() {
                    return proxy::IMapImpl::size();
                }

                /**
                * Returns <tt>true</tt> if this map contains no key-value mappings.
                *
                * @return <tt>true</tt> if this map contains no key-value mappings
                */
                boost::future<bool> isEmpty() {
                    return proxy::IMapImpl::isEmpty();
                }


                /**
                * Copies all of the mappings from the specified map to this map
                * (optional operation).  The effect of this call is equivalent to that
                * of calling put(k, v) on this map once
                * for each mapping from key <tt>k</tt> to value <tt>v</tt> in the
                * specified map.  The behavior of this operation is undefined if the
                * specified map is modified while the operation is in progress.
                *
                * @param m mappings to be stored in this map
                */
                boost::future<void> putAll(const std::map<K, V> &entries) {
                    std::map<int, EntryVector> entryMap;

                    for (typename std::map<K, V>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
                        serialization::pimpl::Data keyData = toData(it->first);
                        serialization::pimpl::Data valueData = toData(it->second);

                        int partitionId = getPartitionId(keyData);

                        entryMap[partitionId].push_back(std::make_pair(keyData, valueData));
                    }

                    putAllInternal(entryMap);
                }

                /**
                * Removes all of the mappings from this map (optional operation).
                * The map will be empty after this call returns.
                */
                boost::future<void> clear() {
                    proxy::IMapImpl::clear();
                }

                monitor::LocalMapStats &getLocalMapStats() {
                    return stats;
                }

            protected:
                typedef std::pair<const K *, std::shared_ptr<serialization::pimpl::Data> > KEY_DATA_PAIR;

                /**
                 * Default TTL value of a record.
                 */
                static int64_t DEFAULT_TTL;

                /**
                 * Default Max Idle value of a record.
                 */
                static int64_t DEFAULT_MAX_IDLE;

                static const std::shared_ptr<client::impl::ClientMessageDecoder<V> > &GET_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapGetCodec, V>::instance();
                }

                static const std::shared_ptr<client::impl::ClientMessageDecoder<V> > &PUT_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapPutCodec, V>::instance();
                }

                static const std::shared_ptr<client::impl::ClientMessageDecoder<void> > &
                SET_ASYNC_RESPONSE_DECODER() {
                    return client::impl::VoidMessageDecoder::instance();
                }

                static const std::shared_ptr<client::impl::ClientMessageDecoder<V> > &
                REMOVE_ASYNC_RESPONSE_DECODER() {
                    return client::impl::DataMessageDecoder<protocol::codec::MapRemoveCodec, V>::instance();
                }

                virtual std::shared_ptr<V> getInternal(serialization::pimpl::Data &keyData) {
                    return std::shared_ptr<V>(std::move(toObject<V>(proxy::IMapImpl::getData(keyData))));
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

                virtual boost::future<std::shared_ptr<V>>
                removeAsyncInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        auto request = protocol::codec::MapRemoveCodec::encodeRequest(name, keyData,
                                                                                      util::getCurrentThreadId());
                        auto future = invokeOnKeyOwner(request, keyData);
                        return future.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                            return REMOVE_ASYNC_RESPONSE_DECODER()->decodeClientMessage(f.get(),
                                                                                        getSerializationService());
                        });
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return boost::future<std::shared_ptr<V>>();
                }

                virtual void removeAllInternal(const serialization::pimpl::Data &predicateData) {
                    return proxy::IMapImpl::removeAll(predicateData);
                }

                virtual boost::future<void> deleteInternal(const serialization::pimpl::Data &keyData) {
                    return proxy::IMapImpl::deleteEntry(keyData);
                }

                virtual boost::future<bool> tryRemoveInternal(const serialization::pimpl::Data &keyData, std::chrono::steady_clock::duration timeout) {
                    return proxy::IMapImpl::tryRemove(keyData, timeout);
                }

                virtual bool tryPutInternal(const serialization::pimpl::Data &keyData,
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

                virtual std::unique_ptr<serialization::pimpl::Data>
                putIfAbsentInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &valueData,
                                    std::chrono::steady_clock::duration ttl) {
                    return proxy::IMapImpl::putIfAbsentData(keyData, valueData, ttl);
                }

                virtual bool replaceIfSameInternal(const serialization::pimpl::Data &keyData,
                                                   const serialization::pimpl::Data &valueData,
                                                   const serialization::pimpl::Data &newValueData) {
                    return proxy::IMapImpl::replace(keyData, valueData, newValueData);
                }

                virtual std::unique_ptr<serialization::pimpl::Data>
                replaceInternal(const serialization::pimpl::Data &keyData,
                                const serialization::pimpl::Data &valueData) {
                    return proxy::IMapImpl::replaceData(keyData, valueData);

                }

                virtual void
                setInternal(const serialization::pimpl::Data &keyData, const serialization::pimpl::Data &valueData,
                            std::chrono::steady_clock::duration ttl) {
                    proxy::IMapImpl::set(keyData, valueData, ttl);
                }

                virtual bool evictInternal(const serialization::pimpl::Data &keyData) {
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

                template<typename ResultType>
                boost::future<std::shared_ptr<ResultType>>
                submitToKeyInternal(const serialization::pimpl::Data &keyData,
                                    const serialization::pimpl::Data &processor) {
                    int partitionId = getPartitionId(keyData);

                    auto request = protocol::codec::MapSubmitToKeyCodec::encodeRequest(getName(),
                                                                                processor,
                                                                                keyData,
                                                                                util::getCurrentThreadId());

                    auto future = invokeAndGetFuture(request, partitionId);
                    return future.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                        auto data = protocol::codec::MapSubmitToKeyCodec::ResponseParameters::decode(f.get()).response;
                        return getSerializationService().template toSharedObject<ResultType>(data);
                    });
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

                virtual void
                putAllInternal(const std::map<int, EntryVector> &entries) {
                    proxy::IMapImpl::putAllData(entries);
                }

                std::shared_ptr<serialization::pimpl::Data> toShared(const serialization::pimpl::Data &data) {
                    return std::shared_ptr<serialization::pimpl::Data>(new serialization::pimpl::Data(data));
                }

                virtual boost::future<protocol::ClientMessage> getAsyncInternal(const K &key) {
                    try {
                        serialization::pimpl::Data keyData = toData<K>(key);
                        return getAsyncInternal(keyData);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return boost::future<protocol::ClientMessage>();
                }

                virtual boost::future<protocol::ClientMessage>
                getAsyncInternal(const serialization::pimpl::Data &keyData) {
                    try {
                        auto request = protocol::codec::MapGetCodec::encodeRequest(
                                name, keyData, util::getCurrentThreadId());
                        return invokeOnKeyOwner(request, keyData);
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return boost::future<protocol::ClientMessage>();
                }

                virtual boost::future<std::shared_ptr<V>>
                putAsyncInternal(std::chrono::steady_clock::duration ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData,
                                 const V &value) {
                    try {
                        serialization::pimpl::Data valueData = toData<V>(value);
                        auto future = putAsyncInternalData(ttl, ttlUnit, maxIdle, maxIdleUnit, keyData, valueData);
                        return future.then(boost::launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                            return PUT_ASYNC_RESPONSE_DECODER()->decodeClientMessage(f.get(),
                                                                                     getSerializationService());
                        });
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return boost::future<std::shared_ptr<V>>();
                }

                virtual boost::future<void>
                setAsyncInternal(std::chrono::steady_clock::duration ttl, const util::concurrent::TimeUnit &ttlUnit, int64_t *maxIdle,
                                 const util::concurrent::TimeUnit &maxIdleUnit,
                                 const serialization::pimpl::Data &keyData, const V &value) {
                    try {
                        serialization::pimpl::Data valueData = toData<V>(value);
                        return setAsyncInternalData(ttl, ttlUnit, maxIdle, maxIdleUnit, keyData, valueData).then(
                                boost::launch::sync,
                                [](boost::future<protocol::ClientMessage> f) { f.get(); });
                    } catch (exception::IException &e) {
                        util::ExceptionUtil::rethrow(std::current_exception());
                    }
                    return boost::future<void>();
                }

            private:
                monitor::impl::LocalMapStatsImpl stats;
            };

            template<typename K, typename V>
            int64_t ClientMapProxy<K, V>::DEFAULT_TTL = -1L;

            template<typename K, typename V>
            int64_t ClientMapProxy<K, V>::DEFAULT_MAX_IDLE = -1L;

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_MAP_CLIENTMAPPROXY_H_ */

