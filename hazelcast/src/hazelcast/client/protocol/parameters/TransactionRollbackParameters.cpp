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
 * TransactionRollbackParameters.h
 *
 *  Created on: Apr 30, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/TransactionRollbackParameters.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<hazelcast::client::protocol::ClientMessage> TransactionRollbackParameters::encode(
                        const std::string &transactionId, long threadId) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(TYPE);
                    clientMessage->set(transactionId).set(threadId);
                    clientMessage->updateFrameLength();

                    // set the connection type to single connection
                    clientMessage->setIsBoundToSingleConnection(true);

                    return clientMessage;
                }

                int32_t TransactionRollbackParameters::calculateDataSize(
                        const std::string &transactionId, long threadId) {
                    return ClientMessage::HEADER_SIZE
                            + ClientMessage::calculateDataSize(transactionId)
                            + ClientMessage::calculateDataSize(threadId);
                }
            }
        }
    }
}

