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

#include "hazelcast/client/protocol/parameters/TransactionalMapSetParameters.h"
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                    std::auto_ptr<ClientMessage> TransactionalMapSetParameters::encode(
                            const std::string &name, 
                            const std::string &txnId, 
                            const int64_t &threadId, 
                            const serialization::pimpl::Data &key, 
                            const serialization::pimpl::Data &value) {
                        int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                        std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                        clientMessage->setMessageType((uint16_t)TransactionalMapSetParameters::TYPE);
                        clientMessage->set(name).set(txnId).set(threadId).set(key).set(value);
                        clientMessage->updateFrameLength();
                        return clientMessage;
                    }

                    int32_t TransactionalMapSetParameters::calculateDataSize(
                            const std::string &name, 
                            const std::string &txnId, 
                            const int64_t &threadId, 
                            const serialization::pimpl::Data &key, 
                            const serialization::pimpl::Data &value) {
                        return ClientMessage::HEADER_SIZE
                             + ClientMessage::calculateDataSize(name)
                             + ClientMessage::calculateDataSize(txnId)
                             + ClientMessage::calculateDataSize(threadId)
                             + ClientMessage::calculateDataSize(key)
                             + ClientMessage::calculateDataSize(value);
                    }
            }
        }
    }
}

