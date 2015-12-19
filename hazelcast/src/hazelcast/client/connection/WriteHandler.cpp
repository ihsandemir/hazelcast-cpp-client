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
//
// Created by sancar koyunlu on 25/12/13.
//

#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/exception/IOException.h"
#include <hazelcast/util/Util.h>
#include <hazelcast/util/ILogger.h>

//#define BOOST_THREAD_PROVIDES_FUTURE

namespace hazelcast {
    namespace client {
        namespace connection {

            void WriteHandler::staticRun(util::ThreadArgs &args) {
                Connection *conn = (Connection *) args.arg0;

                moodycamel::BlockingConcurrentQueue<protocol::ClientMessage *> *writeQueue =
                        (moodycamel::BlockingConcurrentQueue<protocol::ClientMessage *> *) args.arg1;

                protocol::ClientMessage *lastMessages[10];
                size_t numMessages;
                size_t lastMessageNumber;

                int32_t numBytesWrittenToSocketForMessage;
                int32_t lastMessageFrameLen;

                while (conn->live) {
                    if (numMessages == 0) {
                        numMessages = writeQueue->wait_dequeue_bulk(lastMessages, 10);

                        if (numMessages > 0) {
                            lastMessageNumber = 0;
                            numBytesWrittenToSocketForMessage = 0;
                            lastMessageFrameLen = lastMessages[0]->getFrameLength();
                        }
                    }

                    while (numMessages - lastMessageNumber > 0) {
                        try {
                            numBytesWrittenToSocketForMessage += lastMessages[lastMessageNumber]->writeTo(
                                    conn->getSocket(),
                                    numBytesWrittenToSocketForMessage, lastMessageFrameLen);

                            if (numBytesWrittenToSocketForMessage >= lastMessageFrameLen) {
                                // Not deleting message since its memory management is at the future object
                                ++lastMessageNumber;

                                if (lastMessageNumber < numMessages) {
                                    numBytesWrittenToSocketForMessage = 0;
                                    lastMessageFrameLen = lastMessages[lastMessageNumber]->getFrameLength();
                                } else {
                                    numMessages = 0;
                                    break;
                                }
                            }
                        } catch (exception::IOException &e) {
                            Address const &address = conn->getRemoteEndpoint();

                            std::string message = e.what();
                            size_t len = message.length() + 150;
                            char *msg = new char[len];
                            util::snprintf(msg, len,
                                           "[IOHandler::handleSocketException] Closing socket to endpoint %s:%d, Cause:%s\n",
                                           address.getHost().c_str(), address.getPort(), message.c_str());
                            util::ILogger::getLogger().getLogger().warning(msg);

                            // TODO: This call shall resend pending requests and reregister events, hence it can be off-loaded
                            // to another thread in order not to block the critical IO thread
                            conn->close();
                            return;
                        }

                    }

                }
            }
        }
    }
}

