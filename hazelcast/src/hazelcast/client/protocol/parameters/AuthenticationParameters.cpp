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
 * AuthenticationParameters.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#include "hazelcast/client/protocol/parameters/AuthenticationParameters.h"
#include "hazelcast/client/protocol/ClientMessage.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace parameters {
                AuthenticationParameters AuthenticationParameters::decode(ClientMessage &flyweight) {
                    return AuthenticationParameters(flyweight);
                }

                /**
                * Encode parameters into byte array, i.e. ClientMessage
                *
                * @param username
                * @param password
                * @param uuid
                * @param ownerUuid
                * @param isOwnerConnection
                * @return encoded ClientMessage
                */
                std::auto_ptr<ClientMessage> AuthenticationParameters::encode(const std::string & username,
                        const std::string & password, const std::string & uuid,
                        const std::string & ownerUuid, uint8_t isOwnerConnection) {
                    int32_t requiredDataSize = calculateDataSize(username, password, uuid, ownerUuid, isOwnerConnection);
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(protocol::AUTHENTICATION_DEFAULT_REQUEST);
                    clientMessage->set(username).set(password).set(uuid).set(ownerUuid).set(isOwnerConnection);
                    clientMessage->updateFrameLength();

                    clientMessage->setRetryable(true);

                    return clientMessage;
                }

                /**
                * sample data size estimation
                *
                * @return size
                */
                int32_t AuthenticationParameters::calculateDataSize(const std::string & username,
                        const std::string & password, const std::string & uuid,
                        const std::string & ownerUuid, uint8_t isOwnerConnection) {
                    return ClientMessage::HEADER_SIZE//
                            + ClientMessage::calculateDataSize(username)//
                            + ClientMessage::calculateDataSize(password)//
                            + ClientMessage::calculateDataSize(uuid)//
                            + ClientMessage::calculateDataSize(ownerUuid)//
                            + ClientMessage::UINT8_SIZE;
                }

                AuthenticationParameters::AuthenticationParameters(ClientMessage &message) {
                    username = message.getStringUtf8();
                    password = message.getStringUtf8();
                    uuid = message.getStringUtf8();
                    ownerUuid = message.getStringUtf8();
                    isOwnerConnection = message.getUint8();
                }

                AuthenticationParameters::~AuthenticationParameters() {
                }

                AuthenticationParameters::AuthenticationParameters(const AuthenticationParameters &rhs) :
                        username(new std::string(*rhs.username)), password(new std::string(*rhs.password)),
                        uuid(new std::string(*rhs.uuid)), ownerUuid(new std::string(*rhs.ownerUuid)),
                        isOwnerConnection(rhs.isOwnerConnection) {
                }
            }
        }
    }
}
