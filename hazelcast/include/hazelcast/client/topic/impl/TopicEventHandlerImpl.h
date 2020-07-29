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

#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                template<typename Listener>
                class TopicEventHandlerImpl : public protocol::codec::topic_addmessagelistener_handler {
                public:
                    TopicEventHandlerImpl(const std::string &instanceName, spi::ClientClusterService &clusterService,
                                      serialization::pimpl::SerializationService &serializationService,
                                          Listener &&messageListener)
                            :instanceName(instanceName), clusterService(clusterService),
                            serializationService(serializationService), listener(messageListener) {}

                    void handle_topic(const Data &item, const int64_t &publishTime, const boost::optional<boost::uuids::uuid> &uuid) override {
                        listener(Message(instanceName, TypedData(std::move(item), serializationService), publishTime,
                                        clusterService.getMember(*uuid)));
                    }
                private:
                    std::string instanceName;
                    spi::ClientClusterService &clusterService;
                    serialization::pimpl::SerializationService &serializationService;
                    Listener &listener;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



