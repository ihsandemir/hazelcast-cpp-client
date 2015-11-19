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
#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/codec/MemberCodec.h"
#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/protocol/codec/DistributedObjectInfoCodec.h"
#include "hazelcast/client/DistributedObjectInfo.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            const std::string ClientTypes::CPP = "CPP";

            struct ClientMessage::MessageHeaderType ClientMessage::DEFAULT_HEADER = getDefaultHeader();

            ClientMessage::ClientMessage() : isOwner(false), numBytesWrittenToConnection(0), numBytesFilled(0),
                                             retryable(false), isBoundToSingleConnection(false) {}

            ClientMessage::ClientMessage(int32_t size) : numBytesWrittenToConnection(0), numBytesFilled(0),
                                                         retryable(false),
                                                         isBoundToSingleConnection(false) {
                byteBuffer = new byte[size];
                memset(byteBuffer, 0, size);

                isOwner = true;

                wrapForWrite(byteBuffer, size, DATA_OFFSET_FIELD_OFFSET);

                header = reinterpret_cast<MessageHeaderType *> (byteBuffer);

                setFrameLength(size);
            }

            ClientMessage::~ClientMessage() {
                if (isOwner) {
                    delete [] byteBuffer;
                }
            }

            void ClientMessage::wrapForDecode(byte *buffer, int32_t size, bool owner) {
                isOwner = owner;
                wrapForRead(buffer, size, HEADER_SIZE);
                header = reinterpret_cast<MessageHeaderType *> (buffer);
            }

            void ClientMessage::wrapForEncode(byte *buffer, int32_t size, bool owner) {
                isOwner = owner;
                byteBuffer = buffer;
                wrapForWrite(byteBuffer, size, HEADER_SIZE);
                header = reinterpret_cast<MessageHeaderType *> (buffer);

                *header = ClientMessage::DEFAULT_HEADER;
                setFrameLength(size);
            }

            ClientMessage ClientMessage::createForEncode(int32_t size) {
                ClientMessage msg;
                byte *buffer = new byte[size];
                memset(buffer, 0, size);
                msg.wrapForEncode(buffer, size, true);
                return msg;
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::setFrameLength(int32_t length) {
                util::Bits::nativeToLittleEndian4(&length, &header->frameLength);
            }

            void ClientMessage::setMessageType(uint16_t type) {
                util::Bits::nativeToLittleEndian2(&type, &header->type);
            }

            void ClientMessage::setVersion(uint8_t value) {
                header->version = value;
            }

            void ClientMessage::setFlags(uint8_t value) {
                header->flags = value;
            }

            void ClientMessage::setCorrelationId(uint32_t id) {
                util::Bits::nativeToLittleEndian4(&id, &header->correlationId);
            }

            void ClientMessage::setPartitionId(int32_t partitionId) {
                util::Bits::nativeToLittleEndian4(&partitionId, &header->partitionId);
            }

            void ClientMessage::setDataOffset(uint16_t offset) {
                util::Bits::nativeToLittleEndian2(&offset, &header->dataOffset);
            }

            void ClientMessage::updateFrameLength() {
                setFrameLength(getIndex());
            }

            void ClientMessage::set(const std::string *value) {
                setNullable<std::string> (value);
            }

            void ClientMessage::set(const serialization::pimpl::Data &value) {
                setArray<byte>(value.toByteArray());
            }

            void ClientMessage::set(const serialization::pimpl::Data *value) {
                setNullable<serialization::pimpl::Data>(value);
            }

            void ClientMessage::set(const Address &value) {
                codec::AddressCodec::encode(value, *this);
            }

            void ClientMessage::set(const Address *value) {
                setNullable<Address>(value);
            }

            void ClientMessage::set(const Member &value) {
                codec::MemberCodec::encode(value, *this);
            }

            void ClientMessage::set(const Member *value) {
                setNullable<Member>(value);
            }

            void ClientMessage::set(const map::DataEntryView &value) {
                codec::DataEntryViewCodec::encode(value, *this);
            }

            void ClientMessage::set(const map::DataEntryView *value) {
                setNullable<map::DataEntryView>(value);
            }

            void ClientMessage::set(const DistributedObjectInfo &value) {
                codec::DistributedObjectInfoCodec::encode(value, *this);
            }

            void ClientMessage::set(const DistributedObjectInfo *value) {
                setNullable<DistributedObjectInfo>(value);
            }
            //----- Setter methods end ---------------------

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

            //----- Getter methods begin -------------------
            int32_t ClientMessage::getFrameLength() const {
                int32_t result;

                util::Bits::littleEndianToNative4(
                        &header->frameLength, &result);

                return result;
            }

            uint16_t ClientMessage::getMessageType() const {
                uint16_t type;

                util::Bits::littleEndianToNative2(&header->type, &type);

                return type;
            }

            uint8_t ClientMessage::getVersion() {
                return header->version;
            }

            uint32_t ClientMessage::getCorrelationId() const {
                uint32_t value;
                util::Bits::littleEndianToNative4(&header->correlationId, &value);
                return value;
            }

            int32_t ClientMessage::getPartitionId() const {
                int32_t value;
                util::Bits::littleEndianToNative4(&header->partitionId, &value);
                return value;
            }

            uint16_t ClientMessage::getDataOffset() const {
                uint16_t value;
                util::Bits::littleEndianToNative2(&header->dataOffset, &value);
                return value;
            }

            bool ClientMessage::isFlagSet(uint8_t flag) const {
                return flag == (header->flags & flag);
            }

            template <>
            uint8_t ClientMessage::get() {
                return getUint8();
            }

            template <>
            int8_t ClientMessage::get() {
                return getInt8();
            }

            template <>
            int16_t ClientMessage::get() {
                return getInt16();
            }

            template <>
            uint16_t ClientMessage::get() {
                return getUint16();
            }

            template <>
            uint32_t ClientMessage::get() {
                return getUint32();
            }

            template <>
            int32_t ClientMessage::get() {
                return getInt32();
            }

            template <>
            int64_t ClientMessage::get() {
                return getInt64();
            }

            template <>
            uint64_t ClientMessage::get() {
                return getUint64();
            }

            template <>
            Address ClientMessage::get() {
                return codec::AddressCodec::decode(*this);
            }

            template <>
            Member ClientMessage::get() {
                return codec::MemberCodec::decode(*this);
            }

            template <>
            map::DataEntryView ClientMessage::get() {
                return codec::DataEntryViewCodec::decode(*this);
            }

            template <>
            DistributedObjectInfo ClientMessage::get() {
                return codec::DistributedObjectInfoCodec::decode(*this);
            }

            template <>
            codec::StackTraceElement ClientMessage::get() {
                return codec::StackTraceElementCodec::decode(*this);
            }
            //----- Getter methods end --------------------------

            //----- Data size calculation functions BEGIN -------
            int32_t ClientMessage::calculateDataSize(uint8_t param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int8_t param) {
                return INT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(bool param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int16_t param) {
                return INT16_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(uint16_t param) {
                return UINT16_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int32_t param) {
                return INT32_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(uint32_t param) {
                return UINT32_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int64_t param) {
                return INT64_SIZE;
            }

            #ifdef HZ_PLATFORM_DARWIN
            int32_t ClientMessage::calculateDataSize(long param) {
                return calculateDataSize((int64_t)param);
            }
            #endif

            int32_t ClientMessage::calculateDataSize(uint64_t param) {
                return UINT64_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(const std::string &param) {
                return INT32_SIZE +  // bytes for the length field
                       param.length();
            }

            int32_t ClientMessage::calculateDataSize(const std::string *param) {
                return calculateDataSizeNullable<std::string>(param);
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data &param) {
                return INT32_SIZE +  // bytes for the length field
                       (int32_t)param.totalSize();
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data *param) {
                return calculateDataSizeNullable<serialization::pimpl::Data>(param);
            }

            int32_t ClientMessage::calculateDataSize(const Address &param) {
                return codec::AddressCodec::calculateDataSize(param);
            }

            int32_t ClientMessage::calculateDataSize(const Address *param) {
                return calculateDataSizeNullable<Address>(param);
            }

            int32_t ClientMessage::calculateDataSize(const Member &param) {
                return codec::MemberCodec::calculateDataSize(param);
            }

            int32_t ClientMessage::calculateDataSize(const Member *param) {
                return calculateDataSizeNullable<Member>(param);
            }

            int32_t ClientMessage::calculateDataSize(const map::DataEntryView &param) {
                return codec::DataEntryViewCodec::calculateDataSize(param);
            }

            int32_t ClientMessage::calculateDataSize(const map::DataEntryView *param) {
                return calculateDataSizeNullable<map::DataEntryView>(param);
            }

            int32_t ClientMessage::calculateDataSize(const DistributedObjectInfo &param) {
                return codec::DistributedObjectInfoCodec::calculateDataSize(param);
            }

            int32_t ClientMessage::calculateDataSize(const DistributedObjectInfo *param) {
                return calculateDataSizeNullable<DistributedObjectInfo>(param);
            }
            //----- Data size calculation functions END ---------

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