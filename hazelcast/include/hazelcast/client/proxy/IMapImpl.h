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
#ifndef HAZELCAST_IMAP_IMPL
#define HAZELCAST_IMAP_IMPL

#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/map/DataEntryView.h"

#include "hazelcast/client/protocol/parameters/MapExecuteOnKeyParameters.h"
#include "hazelcast/client/protocol/parameters/MapExecuteOnAllKeysParameters.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }
        namespace proxy {
            class HAZELCAST_API IMapImpl : public ProxyImpl {
            protected:
                IMapImpl(const std::string& instanceName, spi::ClientContext *context);

                bool containsKey(const serialization::pimpl::Data& key);

                bool containsValue(const serialization::pimpl::Data& value);

                serialization::pimpl::Data get(const serialization::pimpl::Data& key);

                serialization::pimpl::Data put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                serialization::pimpl::Data remove(const serialization::pimpl::Data& key);

                bool remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void deleteEntry(const serialization::pimpl::Data& key);

                void flush();

                bool tryRemove(const serialization::pimpl::Data& key, long timeoutInMillis);

                bool tryPut(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long timeoutInMillis);

                serialization::pimpl::Data put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                void putTransient(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                serialization::pimpl::Data putIfAbsent(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttlInMillis);

                bool replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& oldValue, const serialization::pimpl::Data& newValue);

                serialization::pimpl::Data replace(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                void set(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, long ttl);

                void lock(const serialization::pimpl::Data& key);

                void lock(const serialization::pimpl::Data& key, long leaseTime);

                bool isLocked(const serialization::pimpl::Data& key);

                bool tryLock(const serialization::pimpl::Data& key, long timeInMillis);

                void unlock(const serialization::pimpl::Data& key);

                void forceUnlock(const serialization::pimpl::Data& key);

                std::auto_ptr<std::string> addInterceptor(serialization::Portable &interceptor);

                std::auto_ptr<std::string> addInterceptor(serialization::IdentifiedDataSerializable &interceptor);

                void removeInterceptor(const std::string& id);

                std::auto_ptr<std::string> addEntryListener(impl::BaseEventHandler *entryEventHandler, bool includeValue);

                bool removeEntryListener(const std::string& registrationId);

                std::auto_ptr<std::string> addEntryListener(impl::BaseEventHandler *entryEventHandler, const serialization::pimpl::Data& key, bool includeValue);

                std::auto_ptr<map::DataEntryView> getEntryView(const serialization::pimpl::Data& key);

                bool evict(const serialization::pimpl::Data& key);

                void evictAll();

                void getAll(const std::vector<serialization::pimpl::Data> &keys,
                            std::auto_ptr<protocol::DataArray> &resultKeys,
                            std::auto_ptr<protocol::DataArray> &resultValue);

                std::auto_ptr<protocol::DataArray> keySet(const std::string& sql);

                void entrySet(const std::string &sql,
                                        std::auto_ptr<protocol::DataArray> &resultKeys,
                                        std::auto_ptr<protocol::DataArray> &resultValue);

                std::auto_ptr<protocol::DataArray> values(const std::string& sql);

                void addIndex(const std::string& attribute, bool ordered);

                template<typename K, typename ResultType, typename EntryProcessor>
                ResultType executeOnKey(const K& key, EntryProcessor& entryProcessor) {
                    serialization::pimpl::Data keyData = toData<K>(key);

                    int partitionId = getPartitionId(keyData);

                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::MapExecuteOnKeyParameters::encode(
                                    getName(), toData<EntryProcessor>(entryProcessor), toData<K>(key));

                    return invokeAndGetResult<ResultType>(request, partitionId);
                }

                template<typename ResultType, typename EntryProcessor>
                void executeOnEntries(EntryProcessor& entryProcessor,
                                            std::auto_ptr<protocol::DataArray> &resultKeys,
                                            std::auto_ptr<protocol::DataArray> &resultValues) {

                    std::auto_ptr<protocol::ClientMessage> request =
                            protocol::parameters::MapExecuteOnAllKeysParameters::encode(
                                    getName(), toData<EntryProcessor>(entryProcessor));

                    std::auto_ptr<protocol::ClientMessage> response = invoke(request);

                    resultKeys = getDataList(response.get());
                    resultValues = getDataList(response.get());
                }

                int size();

                bool isEmpty();

                void putAll(const protocol::DataArray &keys,
                                      const protocol::DataArray &values);

                void clear();

                static const std::string VALUE_ITERATION_TYPE;
                static const std::string KEY_ITERATION_TYPE;
                static const std::string ENTRY_ITERATION_TYPE;
                static const std::string NO_PREDICATE;
            };
        }

    }
}

#endif /* HAZELCAST_IMAP_IMPL */

