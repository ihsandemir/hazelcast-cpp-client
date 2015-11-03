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
 * AuthenticationCustomCredentialsParameters.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/AuthenticationCustomCredentialsParameters.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                /**
                * Encode parameters into byte array, i.e. ClientMessage
                *
                * @param uuid
                * @param ownerUuid
                * @param isOwnerConnection
                * @return encoded ClientMessage
                */
                std::auto_ptr<ClientMessage> AuthenticationCustomCredentialsParameters::encode(
                        std::vector<byte> &credentials,
                        const std::string & uuid,
                        const std::string & ownerUuid,
                        uint8_t isOwnerConnection) {
                    int32_t requiredDataSize = calculateDataSize(credentials, uuid, ownerUuid, isOwnerConnection);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(protocol::AUTHENTICATION_CUSTOM_REQUEST);
                    clientMessage->set(credentials).set(uuid).set(ownerUuid).set(isOwnerConnection);
                    clientMessage->updateFrameLength();

                    clientMessage->setRetryable(true);

                    return clientMessage;
                }

                /**
                * sample data size estimation
                *
                * @return size
                */
                int32_t AuthenticationCustomCredentialsParameters::calculateDataSize(
                        std::vector<byte> &credentials,
                        const std::string & uuid,
                        const std::string & ownerUuid,
                        uint8_t isOwnerConnection) {
                    return ClientMessage::HEADER_SIZE//
                            + (int32_t)credentials.size()
                            + ClientMessage::calculateDataSize(uuid)//
                            + ClientMessage::calculateDataSize(ownerUuid)//
                            + ClientMessage::UINT8_SIZE;
                }

                AuthenticationCustomCredentialsParameters::~AuthenticationCustomCredentialsParameters() {
                }
            }
        }
    }
}
