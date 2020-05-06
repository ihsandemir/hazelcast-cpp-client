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

#include <memory>

#include "hazelcast/util/ExceptionUtil.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientProxy.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"

namespace hazelcast {
    namespace client {
        typedef std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data>> EntryVector;

        namespace proxy {
            class HAZELCAST_API ProxyImpl : public spi::ClientProxy {
            protected:
                ProxyImpl(const std::string &serviceName, const std::string &objectName, spi::ClientContext *context);

                virtual ~ProxyImpl();

                boost::future<protocol::ClientMessage> invoke(std::unique_ptr<protocol::ClientMessage> &request);

                boost::future<protocol::ClientMessage>
                invokeOnPartition(std::unique_ptr<protocol::ClientMessage> &request, int partitionId);

                boost::future<protocol::ClientMessage>
                invokeOnKeyOwner(std::unique_ptr<protocol::ClientMessage> &request,
                                 const serialization::pimpl::Data &keyData);

                boost::future<protocol::ClientMessage> invokeOnAddress(std::unique_ptr<protocol::ClientMessage> &request,
                                                        const Address &address);

                int getPartitionId(const serialization::pimpl::Data &key);

                template<typename T>
                serialization::pimpl::Data toData(const T &object) {
                    return getContext().getSerializationService().template toData<T>(&object);
                }

                template<typename T>
                std::shared_ptr<serialization::pimpl::Data> toSharedData(const T &object) {
                    return toShared(toData<T>(object));
                }

                template<typename T>
                boost::future<boost::optional<T>> toObject(boost::future<serialization::pimpl::Data> &f) {
                    return f.then(boost::launch::deferred, [=] (boost::future<serialization::pimpl::Data> f) {
                        return toObject<T>(f.get());
                    });
                }

                template<typename T>
                inline boost::optional<T> toObject(const serialization::pimpl::Data &data) {
                    return getContext().getSerializationService().template toObject<T>(data);
                }

                template<typename T>
                inline boost::optional<T> toObject(std::unique_ptr<serialization::pimpl::Data> &data) {
                    return toObject<T>(std::move(data));
                }

                template<typename T>
                inline boost::optional<T> toObject(std::unique_ptr<serialization::pimpl::Data> &&data) {
                    if (!data) {
                        return boost::optional<T>();
                    } else {
                        return toObject<T>(*data);
                    }
                }

                template<typename T>
                inline boost::future<std::vector<boost::optional<T>>>
                toObjectVector(boost::future<std::vector<serialization::pimpl::Data>> dataFuture) {
                    return dataFuture.then(boost::launch::deferred,
                                           [=](boost::future<std::vector<serialization::pimpl::Data>> f) {
                                               auto dataResult = f.get();
                                               std::vector<T> result;
                                               std::for_each(dataResult.begin(), dataResult.end(),
                                                             [&](const serialization::pimpl::Data &keyData) {
                                                                 result.push_back(toObject<T>(keyData));
                                                             });
                                               return result;
                                           });
                }

                template<typename K, typename V>
                boost::future<std::unordered_map<K, boost::optional<V>>> toObjectMap(const boost::future<EntryVector> &entriesData) {
                    return entriesData.then(boost::launch::deferred, [=](boost::future<EntryVector> f) {
                        auto entries = f.get();
                        std::unordered_map<K, boost::optional<V>> result;
                        std::for_each(entries.begin(), entries.end(), [&](std::pair<serialization::pimpl::Data, serialization::pimpl::Data> entry) {
                            result[std::move((toObject<K>(entry.first)).value())] = toObject<V>(entry.second);
                        });
                        return result;
                    });
                }

                template<typename K, typename V>
                inline boost::future<std::vector<std::pair<K, boost::optional<V>>>>
                toEntryObjectVector(boost::future<EntryVector> dataFuture) {
                    return dataFuture.then(boost::launch::deferred, [=](boost::future<EntryVector> f) {
                        auto dataEntryVector = f.get();
                        std::vector<K, boost::optional<V>> result;
                        result.reserve(dataEntryVector.size());
                        std::for_each(dataEntryVector.begin(), dataEntryVector.end(),
                                      [&](const std::pair<serialization::pimpl::Data, serialization::pimpl::Data> &entryData) {
                                          result.push_back(
                                                  std::make_pair(std::move(toObject<K>(entryData.first).value()),
                                                                 toObject<V>(entryData.second)));
                                      });
                        return result;
                    });
                }

                boost::future<void> toVoidFuture(boost::future<protocol::ClientMessage> messageFuture) {
                    return messageFuture.then(boost::launch::deferred,
                                              [](boost::future<protocol::ClientMessage> f) { f.get(); });
                }

                template <typename T>
                std::shared_ptr<T> toSharedObject(std::unique_ptr<serialization::pimpl::Data> &data) {
                    return toSharedObject<T>(std::move(data));
                }

                template <typename T>
                std::shared_ptr<T> toSharedObject(std::unique_ptr<serialization::pimpl::Data> &&data) {
                    return std::shared_ptr<T>(toObject<T>(data).release());
                }

                template<typename V>
                std::vector<V> toObjectCollection(const std::vector<serialization::pimpl::Data> &collection) {
                    size_t size = collection.size();
                    std::vector<V> objectArray(size);
                    for (size_t i = 0; i < size; i++) {
                        std::unique_ptr<V> v = toObject<V>(collection[i]);
                        objectArray[i] = *v;
                    }
                    return objectArray;
                }

                template<typename T>
                const std::vector<serialization::pimpl::Data> toDataCollection(const std::vector<T> &elements) {
                    size_t size = elements.size();
                    std::vector<serialization::pimpl::Data> dataCollection(size);
                    for (size_t i = 0; i < size; ++i) {
                        dataCollection[i] = toData(elements[i]);
                    }
                    return dataCollection;
                }

                template<typename K, typename V>
                std::vector<std::pair<K, V> > toObjectEntrySet(
                        std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > const &dataEntrySet) {
                    size_t size = dataEntrySet.size();
                    std::vector<std::pair<K, V> > entrySet(size);
                    for (size_t i = 0; i < size; i++) {
                        std::unique_ptr<K> key = toObject<K>(dataEntrySet[i].first);
                        entrySet[i].first = *key;
                        std::unique_ptr<V> value = toObject<V>(dataEntrySet[i].second);
                        entrySet[i].second = *value;
                    }
                    return entrySet;
                }

                template<typename K, typename V>
                EntryVector toDataEntries(std::map<K, V> const &m) {
                    std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > entries(
                            m.size());
                    int i = 0;
                    for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
                        entries[i++] = std::make_pair(toData(it->first), toData(it->second));
                    }
                    return entries;
                }

                template<typename T, typename CODEC>
                boost::future<T> invokeAndGetFuture(std::unique_ptr<protocol::ClientMessage> &request) {
                    invoke(request).then(boost::launch::deferred, [] (boost::future<protocol::ClientMessage> f) {
                        return CODEC::decode(f.get()).response;
                    });
                }

                template<typename T, typename CODEC>
                boost::future<T> invokeAndGetFuture(std::unique_ptr<protocol::ClientMessage> &request, int partitionId) {
                    invokeOnPartition(request, partitionId).then(boost::launch::deferred, [] (boost::future<protocol::ClientMessage> f) {
                        return (T) CODEC::decode(f.get()).response;
                    });
                }

                template<typename T, typename CODEC>
                boost::future<T> invokeAndGetFuture(std::unique_ptr<protocol::ClientMessage> &request,
                                     const serialization::pimpl::Data &key) {
                    invokeOnKeyOwner(request, key).then(boost::launch::deferred, [] (boost::future<protocol::ClientMessage> f) {
                        return CODEC::decode(f.get()).response;
                    });
                }

                std::shared_ptr<serialization::pimpl::Data> toShared(const serialization::pimpl::Data &data);
            };
        }
    }
}

