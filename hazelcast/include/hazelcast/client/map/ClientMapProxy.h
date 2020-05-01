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
#include <map>
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

                /**
                * Releases the lock for the specified key regardless of the lock owner.
                * It always successfully unlocks the key, never blocks
                * and returns immediately.
                *
                *
                * @param key key to lock.
                */
                boost::future<void> forceUnlock(const K &key) {
                    proxy::IMapImpl::forceUnlock(toData(key));
                }

                /**
                * Adds an interceptor for this map. Added interceptor will intercept operations
                * and execute user defined methods and will cancel operations if user defined method throw exception.
                *
                *
                * Interceptor should extend either Portable or IdentifiedSerializable.
                * Notice that map interceptor runs on the nodes. Because of that same class should be implemented in java side
                * with same classId and factoryId.
                * @param interceptor map interceptor
                * @return id of registered interceptor
                */
                template<typename MapInterceptor>
                boost::future<std::string> addInterceptor(MapInterceptor &interceptor) {
                    return proxy::IMapImpl::addInterceptor<MapInterceptor>(toData(interceptor));
                }

                /**
                * Removes the given interceptor for this map. So it will not intercept operations anymore.
                *
                *
                * @param id registration id of map interceptor
                */
                boost::future<void> removeInterceptor(const std::string &id) {
                    proxy::IMapImpl::removeInterceptor(id);
                }

                /**
                * Adds an entry listener for this map.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener     entry listener
                * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
                *                     contain the value.
                *
                * @return registrationId of added listener that can be used to remove the entry listener.
                */
                template<typename Listener>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerCodec::AbstractEventHandler>(
                            getName(), getContext().getClientClusterService(),
                            getContext().getSerializationService(),
                            listener,
                            includeValue)), includeValue);
                }

                /**
                * Adds an entry listener for this map.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener     entry listener
                * @param predicate The query filter to use when returning the events to the user.
                * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
                *                     contain the value.
                *
                * @return registrationId of added listener that can be used to remove the entry listener.
                */
                template<typename Listener, typename P>
                std::string
                addEntryListener(Listener &&listener, const P &predicate, bool includeValue) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerWithPredicateCodec::AbstractEventHandler>(
                                    getName(), getContext().getClientClusterService(),
                                    getContext().getSerializationService(),
                                    listener,
                                    includeValue)), toData<P>(predicate), includeValue);
                }

                /**
                * Adds the specified entry listener for the specified key.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param listener     entry listener
                * @param key          key to listen
                * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
                *                     contain the value.
                */
                template<typename Listener, typename K>
                boost::future<std::string> addEntryListener(Listener &&listener, bool includeValue, const K &key) {
                    return proxy::IMapImpl::addEntryListener(
                            std::unique_ptr<impl::BaseEventHandler>(new impl::EntryEventHandler<Listener, protocol::codec::MapAddEntryListenerToKeyCodec::AbstractEventHandler>(
                                    getName(), getContext().getClientClusterService(),
                                    getContext().getSerializationService(),
                                    listener,
                                    includeValue)), includeValue, toData<K>(key));
                }


                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                *
                * @param registrationId id of registered listener
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeEntryListener(const std::string &registrationId) {
                    return proxy::IMapImpl::removeEntryListener(registrationId);
                }

                /**
                * Returns the <tt>EntryView</tt> for the specified key.
                *
                *
                * @param key key of the entry
                * @return <tt>EntryView</tt> of the specified key
                * @see EntryView
                */
                EntryView<K, V> getEntryView(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);
                    std::unique_ptr<map::DataEntryView> dataEntryView = proxy::IMapImpl::getEntryViewData(keyData);
                    std::unique_ptr<V> v = toObject<V>(dataEntryView->getValue());
                    EntryView<K, V> view(key, *v, *dataEntryView);
                    return view;
                }

                /**
                * Evicts the specified key from this map. If
                * a <tt>MapStore</tt> defined for this map, then the entry is not
                * deleted from the underlying <tt>MapStore</tt>, evict only removes
                * the entry from the memory.
                *
                *
                * @param key key to evict
                * @return <tt>true</tt> if the key is evicted, <tt>false</tt> otherwise.
                */
                boost::future<bool> evict(const K &key) {
                    serialization::pimpl::Data keyData = toData(key);

                    return evictInternal(keyData);
                }

                /**
                * Evicts all keys from this map except locked ones.
                * <p/>
                * If a <tt>MapStore</tt> is defined for this map, deleteAll is <strong>not</strong> called by this method.
                * If you do want to deleteAll to be called use the #clear() method.
                * <p/>
                * The EVICT_ALL event is fired for any registered listeners.
                * See EntryListener#mapEvicted(MapEvent)}.
                *
                * @see #clear()
                */
                boost::future<void> evictAll() {
                    proxy::IMapImpl::evictAll();
                }

                /**
                * Returns the entries for the given keys.
                *
                * @param keys keys to get
                * @return map of entries
                */
                std::map<K, V> getAll(const std::set<K> &keys) {
                    if (keys.empty()) {
                        return std::map<K, V>();
                    }

                    std::map<int, std::vector<KEY_DATA_PAIR> > partitionToKeyData;
                    // group the request per parition id
                    for (typename std::set<K>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
                        serialization::pimpl::Data keyData = toData<K>(*it);

                        int partitionId = getPartitionId(keyData);

                        partitionToKeyData[partitionId].push_back(std::make_pair(&(*it), toShared(keyData)));
                    }

                    std::map<K, V> result;
                    getAllInternal(partitionToKeyData, result);
                    return result;
                }

                /**
                * Returns a vector clone of the keys contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the vector, and vice-versa.
                *
                * @return a vector clone of the keys contained in this map
                */
                std::vector<K> keySet() {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData();
                    size_t size = dataResult.size();
                    std::vector<K> keys(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<K> key = toObject<K>(dataResult[i]);
                        keys[i] = *key;
                    }
                    return keys;
                }

                /**
                  * @deprecated This API is deprecated in favor of @sa{keySet(const query::Predicate &predicate)}
                  *
                  * Queries the map based on the specified predicate and
                  * returns the keys of matching entries.
                  *
                  * Specified predicate runs on all members in parallel.
                  *
                  *
                  * @param predicate query criteria
                  * @return result key set of the query
                  */
                std::vector<K> keySet(const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return keySet(*p);
                }

                /**
                  *
                  * Queries the map based on the specified predicate and
                  * returns the keys of matching entries.
                  *
                  * Specified predicate runs on all members in parallel.
                  *
                  *
                  * @param predicate query criteria
                  * @return result key set of the query
                  */
                std::vector<K> keySet(const query::Predicate &predicate) {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::keySetData(predicate);
                    size_t size = dataResult.size();
                    std::vector<K> keys(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<K> key = toObject<K>(dataResult[i]);
                        keys[i] = *key;
                    }
                    return keys;
                }

                /**
                  *
                  * Queries the map based on the specified predicate and
                  * returns the keys of matching entries.
                  *
                  * Specified predicate runs on all members in parallel.
                  *
                  *
                  * @param predicate query criteria
                  * @return result key set of the query
                  */
                std::vector<K> keySet(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::KEY);

                    std::vector<serialization::pimpl::Data> dataResult = keySetForPagingPredicateData(predicate);

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

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values() {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData();
                    size_t size = dataResult.size();
                    std::vector<V> values(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<V> value = toObject<V>(dataResult[i]);
                        values[i] = *value;
                    }
                    return values;
                }

                /**
                * @deprecated This API is deprecated in favor of @sa{values(const query::Predicate &predicate)}
                *
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values(const serialization::IdentifiedDataSerializable &predicate) {
                    const query::Predicate *p = (const query::Predicate *) (&predicate);
                    return values(*p);
                }

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values(const query::Predicate &predicate) {
                    std::vector<serialization::pimpl::Data> dataResult = proxy::IMapImpl::valuesData(predicate);
                    size_t size = dataResult.size();
                    std::vector<V> values;
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<V> value = toObject<V>(dataResult[i]);
                        values.push_back(*value);
                    }
                    return values;
                }

                /**
                * Returns a vector clone of the values contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
                *
                *
                * @param predicate the criteria for values to match
                * @return a vector clone of the values contained in this map
                */
                std::vector<V> values(query::PagingPredicate<K, V> &predicate) {
                    predicate.setIterationType(query::VALUE);

                    EntryVector dataResult = proxy::IMapImpl::valuesForPagingPredicateData(predicate);

                    client::impl::EntryArrayImpl<K, V> entries(dataResult, getContext().getSerializationService());

                    entries.sort(query::VALUE, predicate.getComparator());

                    std::pair<size_t, size_t> range = updateAnchor<K, V>(entries, predicate, query::VALUE);

                    std::vector<V> result;
                    for (size_t i = range.first; i < range.second; ++i) {
                        result.push_back(*entries.getValue(i));
                    }
                    return result;
                }

                /**
                * Returns a std::vector< std::pair<K, V> > clone of the mappings contained in this map.
                * The vector is <b>NOT</b> backed by the map,
                * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
                *
                * @return a vector clone of the keys mappings in this map
                */
                std::vector<std::pair<K, V> > entrySet() {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData();
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::unique_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
                }

                /**
                * @deprecated This API is deprecated in favor of @sa{entrySet(const query::Predicate &predicate)}
                *
                * Queries the map based on the specified predicate and
                * returns the matching entries.
                *
                * Specified predicate runs on all members in parallel.
                *
                *
                * @param predicate query criteria
                * @return result entry vector of the query
                */
                std::vector<std::pair<K, V> > entrySet(const serialization::IdentifiedDataSerializable &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                            predicate);
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::unique_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
                }

                std::vector<std::pair<K, V> > entrySet(const query::Predicate &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetData(
                            predicate);
                    size_t size = dataResult.size();
                    std::vector<std::pair<K, V> > entries(size);
                    for (size_t i = 0; i < size; ++i) {
                        std::unique_ptr<K> key = toObject<K>(dataResult[i].first);
                        std::unique_ptr<V> value = toObject<V>(dataResult[i].second);
                        entries[i] = std::make_pair(*key, *value);
                    }
                    return entries;
                }

                std::vector<std::pair<K, V> > entrySet(query::PagingPredicate<K, V> &predicate) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataResult = proxy::IMapImpl::entrySetForPagingPredicateData(
                            predicate);

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

                boost::future<void> addIndex(const std::string &attribute, bool ordered) {
                    proxy::IMapImpl::addIndex(attribute, ordered);
                }

                template<typename ResultType, typename EntryProcessor>
                std::shared_ptr<ResultType> executeOnKey(const K &key, const EntryProcessor &entryProcessor) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data processorData = toData(entryProcessor);

                    std::unique_ptr<serialization::pimpl::Data> response = executeOnKeyInternal(keyData, processorData);

                    return std::shared_ptr<ResultType>(toObject<ResultType>(response).release());
                }

                template<typename ResultType, typename EntryProcessor>
                boost::future<std::shared_ptr<ResultType>>
                submitToKey(const K &key, const EntryProcessor &entryProcessor) {
                    serialization::pimpl::Data keyData = toData(key);
                    serialization::pimpl::Data processorData = toData(entryProcessor);

                    return submitToKeyInternal<ResultType>(keyData, processorData);
                }

                template<typename ResultType, typename EntryProcessor>
                std::map<K, std::shared_ptr<ResultType> >
                executeOnKeys(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    EntryVector entries = executeOnKeysInternal<EntryProcessor>(keys, entryProcessor);

                    std::map<K, std::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::unique_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::unique_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = std::move(resObj);
                    }
                    return result;
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
                * @param entryProcessor that will be applied
                */
                template<typename ResultType, typename EntryProcessor>
                std::map<K, std::shared_ptr<ResultType> > executeOnEntries(const EntryProcessor &entryProcessor) {
                    EntryVector entries = proxy::IMapImpl::executeOnEntriesData<EntryProcessor>(toData(entryProcessor));
                    std::map<K, std::shared_ptr<ResultType> > result;
                    for (size_t i = 0; i < entries.size(); ++i) {
                        std::unique_ptr<K> keyObj = toObject<K>(entries[i].first);
                        std::unique_ptr<ResultType> resObj = toObject<ResultType>(entries[i].second);
                        result[*keyObj] = std::move(resObj);
                    }
                    return result;
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

                virtual EntryVector getAllInternal(
                        const std::map<int, std::vector<KEY_DATA_PAIR> > &partitionToKeyData,
                        std::map<K, V> &result) {

                    /**
                     * This map is needed so that we do not deserialize the response data keys but just use
                     * the keys at the original request
                     */
                    std::map<std::shared_ptr<serialization::pimpl::Data>, const K *> dataKeyPairMap;

                    std::map<int, std::vector<serialization::pimpl::Data> > partitionKeys;

                    for (typename std::map<int, std::vector<KEY_DATA_PAIR> >::const_iterator
                                 it = partitionToKeyData.begin(); it != partitionToKeyData.end(); ++it) {
                        for (typename std::vector<KEY_DATA_PAIR>::const_iterator
                                     keyIt = it->second.begin(); keyIt != it->second.end(); ++keyIt) {
                            partitionKeys[it->first].push_back(serialization::pimpl::Data(*(*keyIt).second));

                            dataKeyPairMap[(*keyIt).second] = (*keyIt).first;
                        }
                    }
                    EntryVector allData = proxy::IMapImpl::getAllData(partitionKeys);
                    EntryVector responseEntries;
                    for (EntryVector::iterator it = allData.begin(); it != allData.end(); ++it) {
                        std::unique_ptr<V> value = toObject<V>(it->second);
                        std::shared_ptr<serialization::pimpl::Data> keyPtr = std::shared_ptr<serialization::pimpl::Data>(
                                new serialization::pimpl::Data(it->first));
                        const K *&keyObject = dataKeyPairMap[keyPtr];
                        assert(keyObject != 0);
                        // Use insert method instead of '[]' operator to prevent the need for
                        // std::is_default_constructible requirement for key and value
                        result.insert(std::make_pair(*keyObject, *value));
                        responseEntries.push_back(std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(
                                *keyPtr, it->second));
                    }

                    return responseEntries;
                }

                virtual std::unique_ptr<serialization::pimpl::Data>
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

                template<typename EntryProcessor>
                EntryVector executeOnKeysInternal(const std::set<K> &keys, const EntryProcessor &entryProcessor) {
                    if (keys.empty()) {
                        return EntryVector();
                    }

                    std::vector<serialization::pimpl::Data> keysData;
                    for (typename std::set<K>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
                        keysData.push_back(toData<K>(*it));
                    }

                    serialization::pimpl::Data entryProcessorData = toData<EntryProcessor>(entryProcessor);

                    return proxy::IMapImpl::executeOnKeysData(keysData, entryProcessorData);
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

