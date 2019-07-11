/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"

#include "hazelcast/client/protocol/codec/ClientLocalBackupListenerCodec.h"
#include "hazelcast/client/protocol/EventMessageConst.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientLocalBackupListenerCodec::REQUEST_TYPE = HZ_CLIENT_LOCALBACKUPLISTENER;
                const bool ClientLocalBackupListenerCodec::RETRYABLE = false;
                const ResponseMessageConst ClientLocalBackupListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::auto_ptr<ClientMessage> ClientLocalBackupListenerCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::auto_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientLocalBackupListenerCodec::REQUEST_TYPE);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientLocalBackupListenerCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientLocalBackupListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ClientLocalBackupListenerCodec::ResponseParameters
                ClientLocalBackupListenerCodec::ResponseParameters::decode(ClientMessage &clientMessage) {
                    return ClientLocalBackupListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ClientLocalBackupListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientLocalBackupListenerCodec::AbstractEventHandler::handle(
                        std::auto_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_BACKUP: {
                            int64_t backupId = clientMessage->get<int64_t>();


                            handleBackupEventV19(backupId);
                            break;
                        }
                        default:
                            getLogger()->warning()
                                    << "[ClientLocalBackupListenerCodec::AbstractEventHandler::handle] Unknown message type ("
                                    << messageType << ") received on event handler.";
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

