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
 * TransactionCommitParameters.h
 *
 *  Created on: Apr 30, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/TransactionCommitParameters.h"

#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                std::auto_ptr<hazelcast::client::protocol::ClientMessage> TransactionCommitParameters::encode(
                        const std::string &transactionId, long threadId, bool prepareAndCommit) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId, prepareAndCommit);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(TYPE);
                    clientMessage->set(transactionId).set(threadId).set(prepareAndCommit);
                    clientMessage->updateFrameLength();

                    // set the connection type to single connection
                    clientMessage->setIsBoundToSingleConnection(true);

                    return clientMessage;
                }

                int32_t TransactionCommitParameters::calculateDataSize(
                        const std::string &transactionId, long threadId, bool prepareAndCommit) {
                    return ClientMessage::HEADER_SIZE
                            + ClientMessage::calculateDataSize(transactionId)
                            + ClientMessage::calculateDataSize(threadId)
                            + ClientMessage::calculateDataSize(prepareAndCommit);
                }
            }
        }
    }
}

