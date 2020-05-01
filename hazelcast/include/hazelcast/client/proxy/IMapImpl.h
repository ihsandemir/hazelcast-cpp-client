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

#include <hazelcast/client/EntryEvent.h>
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/Predicate.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class IdentifiedDataSerializable;
        }
        namespace proxy {
            class HAZELCAST_API IMapImpl : public ProxyImpl {
            protected:
                IMapImpl(const std::string &instanceName, spi::ClientContext *context);

                boost::future<bool> containsKey(const serialization::pimpl::Data &key);

                boost::future<bool> containsValue(const serialization::pimpl::Data &value);

                boost::future<serialization::pimpl::Data> getData(const serialization::pimpl::Data &key);

                boost::future<serialization::pimpl::Data> removeData(const serialization::pimpl::Data &key);

                boost::future<bool> remove(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<void> removeAll(const serialization::pimpl::Data &predicateData);

                boost::future<void> deleteEntry(const serialization::pimpl::Data &key);

                boost::future<void> flush();

                boost::future<bool> tryRemove(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout);

                boost::future<bool> tryPut(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                            std::chrono::steady_clock::duration timeout);

                boost::future<serialization::pimpl::Data> 
                putData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                        std::chrono::steady_clock::duration ttl);

                boost::future<void> putTransient(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                  std::chrono::steady_clock::duration ttl);

                boost::future<serialization::pimpl::Data> 
                putIfAbsentData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value,
                                std::chrono::steady_clock::duration ttl);

                boost::future<bool> replace(const serialization::pimpl::Data &key, const serialization::pimpl::Data &oldValue,
                             const serialization::pimpl::Data &newValue);

                boost::future<serialization::pimpl::Data> 
                replaceData(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                boost::future<void> set(const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, std::chrono::steady_clock::duration ttl);

                boost::future<void> lock(const serialization::pimpl::Data &key);

                boost::future<void> lock(const serialization::pimpl::Data &key, int64_t leaseTime);

                boost::future<bool> isLocked(const serialization::pimpl::Data &key);

                boost::future<bool> tryLock(const serialization::pimpl::Data &key, std::chrono::steady_clock::duration timeout);

                boost::future<void> unlock(const serialization::pimpl::Data &key);

                boost::future<void> forceUnlock(const serialization::pimpl::Data &key);

                boost::future<std::string> addInterceptor(const serialization::pimpl::Data &interceptor);

                virtual boost::future<void> removeInterceptor(const std::string &id);

                std::string
                addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler, bool includeValue);

                std::string
                addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler, Data &&predicate,
                                 bool includeValue);

                std::string
                addEntryListener(std::unique_ptr<impl::BaseEventHandler> &&entryEventHandler, bool includeValue,
                                 Data &&key);

                virtual boost::future<bool> removeEntryListener(const std::string &registrationId);

                std::unique_ptr<map::DataEntryView> getEntryViewData(const serialization::pimpl::Data &key);

                boost::future<bool> evict(const serialization::pimpl::Data &key);

                virtual boost::future<void> evictAll();

                EntryVector
                getAllData(const std::map<int, std::vector<serialization::pimpl::Data> > &partitionToKeyData);

                std::vector<serialization::pimpl::Data> keySetData();

                std::vector<serialization::pimpl::Data> keySetData(const query::Predicate &predicate);

                std::vector<serialization::pimpl::Data>
                keySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetData();

                EntryVector entrySetData(const serialization::IdentifiedDataSerializable &predicate);

                EntryVector entrySetForPagingPredicateData(const serialization::IdentifiedDataSerializable &predicate);

                std::vector<serialization::pimpl::Data> valuesData();

                std::vector<serialization::pimpl::Data> valuesData(const query::Predicate &predicate);

                EntryVector valuesForPagingPredicateData(const query::Predicate &predicate);

                boost::future<void> addIndex(const std::string &attribute, bool ordered);

                int size();

                boost::future<bool> isEmpty();

                boost::future<void> putAllData(const std::map<int, EntryVector> &entries);

                boost::future<void> clear();

                boost::future<serialization::pimpl::Data> executeOnKeyData(const serialization::pimpl::Data &key,
                                                                           const serialization::pimpl::Data &processor);

                EntryVector executeOnKeysData(const std::vector<serialization::pimpl::Data> &keys,
                                              const serialization::pimpl::Data &processor);

                boost::future<EntryVector> executeOnEntriesData(const serialization::pimpl::Data &entryProcessor);

                boost::future<EntryVector>
                executeOnEntriesData(const serialization::pimpl::Data &entryProcessor,
                                     const serialization::pimpl::Data &predicate);

                template<typename K, typename V>
                std::pair<size_t, size_t> updateAnchor(EntryArray<K, V> &entries,
                                                       query::PagingPredicate<K, V> &predicate,
                                                       query::IterationType iterationType) {
                    if (0 == entries.size()) {
                        return std::pair<size_t, size_t>(0, 0);
                    }

                    const std::pair<size_t, std::pair<K *, V *> > *nearestAnchorEntry = predicate.getNearestAnchorEntry();
                    int nearestPage = (NULL == nearestAnchorEntry ? -1 : (int) nearestAnchorEntry->first);
                    size_t page = predicate.getPage();
                    size_t pageSize = predicate.getPageSize();
                    size_t begin = pageSize * (page - nearestPage - 1);
                    size_t size = entries.size();
                    if (begin > size) {
                        return std::pair<size_t, size_t>(0, 0);
                    }
                    size_t end = begin + pageSize;
                    if (end > size) {
                        end = size;
                    }

                    setAnchor(entries, predicate, nearestPage);

                    return std::pair<size_t, size_t>(begin, end);
                }

                template<typename K, typename V>
                static void
                setAnchor(EntryArray<K, V> &entries, query::PagingPredicate<K, V> &predicate, int nearestPage) {
                    if (0 == entries.size()) {
                        return;
                    }

                    size_t size = entries.size();
                    size_t pageSize = (size_t) predicate.getPageSize();
                    int page = (int) predicate.getPage();
                    for (size_t i = pageSize; i <= size && nearestPage < page; i += pageSize) {
                        std::unique_ptr<K> key = entries.releaseKey(i - 1);
                        std::unique_ptr<V> value = entries.releaseValue(i - 1);
                        std::pair<K *, V *> anchor(key.release(), value.release());
                        nearestPage++;
                        predicate.setAnchor((size_t) nearestPage, anchor);
                    }
                }

                void onInitialize() override;

            private:
                class MapEntryListenerWithPredicateMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerWithPredicateMessageCodec(const std::string &name, bool includeValue,
                                                              int32_t listenerFlags,
                                                              serialization::pimpl::Data &&predicate);

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                    serialization::pimpl::Data predicate;
                };

                class MapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerMessageCodec(const std::string &name, bool includeValue, int32_t listenerFlags);

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                };

                class MapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MapEntryListenerToKeyCodec(const std::string &name, bool includeValue, int32_t listenerFlags,
                                               const serialization::pimpl::Data &key);

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                    bool includeValue;
                    int32_t listenerFlags;
                    serialization::pimpl::Data key;
                };

                std::shared_ptr<impl::ClientLockReferenceIdGenerator> lockReferenceIdGenerator;

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, EntryEvent::type listenerFlags);

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, serialization::pimpl::Data &&predicate,
                                            EntryEvent::type listenerFlags);

                std::unique_ptr<spi::impl::ListenerMessageCodec>
                createMapEntryListenerCodec(bool includeValue, EntryEvent::type listenerFlags,
                                            serialization::pimpl::Data &&key);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

