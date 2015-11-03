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

/*
 * DestroyProxyParameters.h
 *
 *  Created on: Apr 25, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/DestroyProxyParameters.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<hazelcast::client::protocol::ClientMessage> DestroyProxyParameters::encode(
                        const std::string &name, const std::string &serviceName) {
                    int32_t requiredDataSize = calculateDataSize(name, serviceName);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(TYPE);
                    clientMessage->set(name).set(serviceName);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t DestroyProxyParameters::calculateDataSize(const std::string &name,
                                                                  const std::string &serviceName) {
                    return ClientMessage::HEADER_SIZE//
                           + ClientMessage::calculateDataSize(name)//
                           + ClientMessage::calculateDataSize(serviceName);
                }
            }
        }
    }
}

