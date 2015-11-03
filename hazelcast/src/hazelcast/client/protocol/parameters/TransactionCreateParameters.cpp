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
 * TransactionCreateParameters.h
 *
 *  Created on: Apr 30, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/TransactionCreateParameters.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<hazelcast::client::protocol::ClientMessage> TransactionCreateParameters::encode(
                        bool xid, long timeout, int32_t durability, int32_t transactionType, long threadId) {
                    int32_t requiredDataSize = calculateDataSize(xid, timeout, durability, transactionType, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(TYPE);
                    // TODO: change this to use XID which is not null in the future
                    clientMessage->set(xid).set(timeout).set(durability).set(transactionType).set(threadId);
                    clientMessage->updateFrameLength();

                    // set the connection type to single connection
                    clientMessage->setIsBoundToSingleConnection(true);

                    return clientMessage;
                }

                int32_t TransactionCreateParameters::calculateDataSize(
                        bool xid, long timeout, int32_t durability, int32_t transactionType, long threadId) {
                    return ClientMessage::HEADER_SIZE
                            + ClientMessage::calculateDataSize(xid)
                            + ClientMessage::calculateDataSize(timeout)
                            + ClientMessage::calculateDataSize(durability)
                            + ClientMessage::calculateDataSize(transactionType)
                            + ClientMessage::calculateDataSize(threadId);
                }
            }
        }
    }
}

