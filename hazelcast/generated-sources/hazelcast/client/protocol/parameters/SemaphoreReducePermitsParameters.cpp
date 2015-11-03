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

#include "hazelcast/client/protocol/parameters/SemaphoreReducePermitsParameters.h"
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                    std::auto_ptr<ClientMessage> SemaphoreReducePermitsParameters::encode(
                            const std::string &name, 
                            const int32_t &reduction) {
                        int32_t requiredDataSize = calculateDataSize(name, reduction);
                        std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                        clientMessage->setMessageType((uint16_t)SemaphoreReducePermitsParameters::TYPE);
                        clientMessage->set(name).set(reduction);
                        clientMessage->updateFrameLength();
                        return clientMessage;
                    }

                    int32_t SemaphoreReducePermitsParameters::calculateDataSize(
                            const std::string &name, 
                            const int32_t &reduction) {
                        return ClientMessage::HEADER_SIZE
                             + ClientMessage::calculateDataSize(name)
                             + ClientMessage::calculateDataSize(reduction);
                    }
            }
        }
    }
}

