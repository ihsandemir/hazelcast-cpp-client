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
// Created by sancar koyunlu on 6/24/13.





#ifndef HAZELCAST_ENTRY_EVENT_HANDLER
#define HAZELCAST_ENTRY_EVENT_HANDLER

#include "hazelcast/client/impl/PortableEntryEvent.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/MapEvent.h"
#include "hazelcast/client/EntryListener.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/codec/EntryEventCodec.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {

            template<typename K, typename V>
            class EntryEventHandler : public BaseEventHandler {
            public:
                EntryEventHandler(const std::string& instanceName, spi::ClusterService& clusterService, serialization::pimpl::SerializationService& serializationService, EntryListener<K, V>& listener, bool includeValue)
                : instanceName(instanceName)
                , clusterService(clusterService)
                , serializationService(serializationService)
                , listener(listener)
                , includeValue(includeValue) {
                }

                void handle(std::auto_ptr<protocol::ClientMessage> message)
                {
                    std::auto_ptr<protocol::parameters::EntryEventParameters> result = protocol::parameters::EntryEventParameters::decode(*message);

                    if (result->eventType == EntryEventType::EVICT_ALL || result->eventType == EntryEventType::CLEAR_ALL) {
                        fireMapWideEvent(*result);
                        return;
                    }

                    fireEntryEvent(*result);
                }

            private:
                void fireMapWideEvent(const protocol::parameters::EntryEventParameters &event) {
                    std::auto_ptr<Member> member = clusterService.getMember(*event.uuid);

                    MapEvent mapEvent(*member, (EntryEventType::Type)event.eventType, instanceName, event.numberOfAffectedEntries);

                    if (event.eventType == EntryEventType::CLEAR_ALL) {
                        listener.mapCleared(mapEvent);
                    } else if (event.eventType == EntryEventType::EVICT_ALL) {
                        listener.mapEvicted(mapEvent);
                    }
                }

                void fireEntryEvent(const protocol::parameters::EntryEventParameters &event) {
                    EntryEventType type((EntryEventType::Type)event.eventType);
                    boost::shared_ptr<V> value;
                    boost::shared_ptr<V> oldValue;
                    boost::shared_ptr<V> mergingValue;
                    if (includeValue) {
                        value = serializationService.toObject<V>(*event.value);
                        oldValue = serializationService.toObject<V>(*event.oldValue);
                        oldValue = serializationService.toObject<V>(*event.mergingValue);
                    }
                    boost::shared_ptr<K> key = serializationService.toObject<K>(*event.key);
                    std::auto_ptr<Member> member = clusterService.getMember(*event.uuid);
                    EntryEvent<K, V> entryEvent(instanceName, *member, type, key, value, oldValue, mergingValue);
                    if (type == EntryEventType::ADDED) {
                        listener.entryAdded(entryEvent);
                    } else if (type == EntryEventType::REMOVED) {
                        listener.entryRemoved(entryEvent);
                    } else if (type == EntryEventType::UPDATED) {
                        listener.entryUpdated(entryEvent);
                    } else if (type == EntryEventType::EVICTED) {
                        listener.entryEvicted(entryEvent);
                    } else if (type == EntryEventType::EXPIRED) {
                        listener.entryExpired(entryEvent);
                    } else if (type == EntryEventType::MERGED) {
                        listener.entryMerged(entryEvent);
                    }

                }

            private:
                const std::string& instanceName;
                spi::ClusterService& clusterService;
                serialization::pimpl::SerializationService& serializationService;
                EntryListener<K, V>& listener;
                bool includeValue;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__EntryEventHandler_H_

