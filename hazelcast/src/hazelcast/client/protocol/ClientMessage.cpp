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
 * ClientMessage.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */



#include <assert.h>
#include "hazelcast/client/protocol/ClientMessage.h"

#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            struct ClientMessage::MessageHeaderType ClientMessage::DEFAULT_HEADER = getDefaultHeader();

            ClientMessage::ClientMessage() : isOwner(false), numBytesWrittenToConnection(0), numBytesFilled(0),
                                             retryable(false), isBoundToSingleConnection(false) {}

            ClientMessage::ClientMessage(int32_t size) : numBytesWrittenToConnection(0), numBytesFilled(0), retryable(false),
                                                         isBoundToSingleConnection(false) {
            byteBuffer = new byte[size];
            memset(byteBuffer, 0, size);

            isOwner = true;

            header = reinterpret_cast<MessageHeaderType *> (byteBuffer);

            setFrameLength(size);
        }

            ClientMessage::~ClientMessage() {
                if (isOwner) {
                    delete [] byteBuffer;
                }
            }

            void ClientMessage::wrapForEncode(byte *buffer, int32_t size, bool owner) {
                isOwner = owner;
                byteBuffer = buffer;
                wrapForWrite(byteBuffer, size, HEADER_SIZE);
                header = reinterpret_cast<MessageHeaderType *> (buffer);

                *header = ClientMessage::DEFAULT_HEADER;
                setFrameLength(size);
            }

            std::auto_ptr<ClientMessage> ClientMessage::createForEncode(int32_t size) {
                std::auto_ptr<ClientMessage> msg(new ClientMessage());
                byte *buffer = new byte[size];
                memset(buffer, 0, size);
                msg->wrapForEncode(buffer, size, true);
                return msg;
            }

            std::auto_ptr<ClientMessage> ClientMessage::create(int32_t size) {
                return std::auto_ptr<ClientMessage>(new ClientMessage(size));
            }

            bool ClientMessage::fillMessageFrom(util::ByteBuffer &buffer) {
                int32_t frameLen = getFrameLength();
                int32_t numBytesSet = (int32_t) buffer.readBytes(byteBuffer, (size_t) frameLen - numBytesFilled);
                numBytesFilled += numBytesSet;

                bool isComplete = (numBytesFilled == frameLen);

                if (isComplete) {
                    wrapForRead(byteBuffer, frameLen, HEADER_SIZE);
                }

                return isComplete;
            }

            void ClientMessage::append(const ClientMessage *msg) {
                // no need to double check if correlation ids match here,
                // since we make sure that this is guaranteed at the caller that they are matching !
                int32_t dataSize = msg->getDataSize();
                int32_t existingFrameLen = getFrameLength();
                int32_t newFrameLen = existingFrameLen + dataSize;
                ensureBufferSize(newFrameLen);
                memcpy(byteBuffer + existingFrameLen, msg->byteBuffer, (size_t)dataSize);
                setFrameLength(newFrameLen);
            }

            int32_t ClientMessage::getDataSize() const {
                return this->getFrameLength() - getDataOffset();
            }

            void ClientMessage::ensureBufferSize(int32_t requiredCapacity) {
                if (isOwner) {
                    int32_t currentCapacity = getCapacity();
                    if (requiredCapacity > currentCapacity) {
                        // allocate new memory
                        int32_t newSize = findSuitableCapacity(requiredCapacity, currentCapacity);

                        // No need to keep the pointer in a smart pointer here
                        byte *newBuffer = new byte[newSize];
                        memcpy(newBuffer, byteBuffer, (size_t) currentCapacity);
                        // swap the new buffer with the old one
                        // free the old memory
                        delete [] byteBuffer;
                        byteBuffer = newBuffer;
                        wrapForWrite(byteBuffer, newSize, getIndex());
                        header = reinterpret_cast<MessageHeaderType *>(byteBuffer);
                    }
                } else {
                    // Should never be here
                    assert(0);
                }
            }

            int32_t ClientMessage::findSuitableCapacity(int32_t requiredCapacity, int32_t existingCapacity) const {
                int32_t size = existingCapacity;
                do {
                    size <<= 1;
                } while (size < requiredCapacity);

                return size;
            }

            bool ClientMessage::isRetryable() const {
                return retryable;
            }

            void ClientMessage::setRetryable(bool shouldRetry) {
                retryable = shouldRetry;
            }

            bool ClientMessage::isBindToSingleConnection() const {
                return isBoundToSingleConnection;
            }

            void ClientMessage::setIsBoundToSingleConnection(bool isSingleConnection) {
                isBoundToSingleConnection = isSingleConnection;
            }

            bool ClientMessage::writeTo(Socket & socket) {
                bool result = false;

                int32_t frameLen = getFrameLength();
                int32_t numBytesLeft = frameLen - numBytesWrittenToConnection;
                if (numBytesLeft > 0) {
                    numBytesWrittenToConnection += socket.send(byteBuffer, numBytesLeft);
                    result = (numBytesWrittenToConnection == frameLen);
                } else {
                    result = true;
                }

                return result;
            }

            const struct ClientMessage::MessageHeaderType ClientMessage::getDefaultHeader() {
                ClientMessage msg(HEADER_SIZE);
                msg.setFrameLength(HEADER_SIZE);
                msg.setVersion(VERSION);
                msg.setFlags(BEGIN_AND_END_FLAGS);
                msg.setCorrelationId(-1);
                msg.setPartitionId(-1);
                msg.setDataOffset(HEADER_SIZE);

                return *msg.header;
            }
        }
    }
}