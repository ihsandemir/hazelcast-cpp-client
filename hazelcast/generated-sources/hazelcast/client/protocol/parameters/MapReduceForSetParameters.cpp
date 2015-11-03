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

#include "hazelcast/client/protocol/parameters/MapReduceForSetParameters.h"
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                    std::auto_ptr<ClientMessage> MapReduceForSetParameters::encode(
                            const std::string &name, 
                            const std::string &jobId, 
                            const serialization::pimpl::Data &predicate, 
                            const serialization::pimpl::Data &mapper, 
                            const serialization::pimpl::Data &combinerFactory, 
                            const serialization::pimpl::Data &reducerFactory, 
                            const std::string &setName, 
                            const int32_t &chunkSize, 
                            const std::vector<serialization::pimpl::Data> &keys, 
                            const std::string &topologyChangedStrategy) {
                        int32_t requiredDataSize = calculateDataSize(name, jobId, predicate, mapper, combinerFactory, reducerFactory, setName, chunkSize, keys, topologyChangedStrategy);
                        std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                        clientMessage->setMessageType((uint16_t)MapReduceForSetParameters::TYPE);
                        clientMessage->set(name).set(jobId).set(predicate).set(mapper).set(combinerFactory).set(reducerFactory).set(setName).set(chunkSize).set(keys).set(topologyChangedStrategy);
                        clientMessage->updateFrameLength();
                        return clientMessage;
                    }

                    int32_t MapReduceForSetParameters::calculateDataSize(
                            const std::string &name, 
                            const std::string &jobId, 
                            const serialization::pimpl::Data &predicate, 
                            const serialization::pimpl::Data &mapper, 
                            const serialization::pimpl::Data &combinerFactory, 
                            const serialization::pimpl::Data &reducerFactory, 
                            const std::string &setName, 
                            const int32_t &chunkSize, 
                            const std::vector<serialization::pimpl::Data> &keys, 
                            const std::string &topologyChangedStrategy) {
                        return ClientMessage::HEADER_SIZE
                             + ClientMessage::calculateDataSize(name)
                             + ClientMessage::calculateDataSize(jobId)
                             + ClientMessage::calculateDataSize(predicate)
                             + ClientMessage::calculateDataSize(mapper)
                             + ClientMessage::calculateDataSize(combinerFactory)
                             + ClientMessage::calculateDataSize(reducerFactory)
                             + ClientMessage::calculateDataSize(setName)
                             + ClientMessage::calculateDataSize(chunkSize)
                             + ClientMessage::calculateDataSize(keys)
                             + ClientMessage::calculateDataSize(topologyChangedStrategy);
                    }
            }
        }
    }
}

