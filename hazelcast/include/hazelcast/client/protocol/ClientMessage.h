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
 * ClientMessage.h
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

/**
* <p>
* Client Message is the carrier framed data as defined below.
* </p>
* <p>
* Any request parameter, response or event data will be carried in
* the payload.
* </p>
* <p/>
* <pre>
* 0                   1                   2                   3
* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
* |R|                      Frame Length                           |
* +-------------+---------------+---------------------------------+
* |  Version    |B|E|  Flags  |L|               Type              |
* +-------------+---------------+---------------------------------+
* |                       CorrelationId                           |
* +---------------------------------------------------------------+
* |R|                      PartitionId                            |
* +-----------------------------+---------------------------------+
* |        Data Offset          |                                 |
* +-----------------------------+                                 |
* |                      Message Payload Data                    ...
* |                                                              ...
*
* </pre>
*/

#ifndef HAZELCAST_CLIENT_CLIENTMESSAGE_H_
#define HAZELCAST_CLIENT_CLIENTMESSAGE_H_

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <assert.h>

#include "hazelcast/client/common/containers/LittleEndianBufferWrapper.h"

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/ClientMessageType.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/impl/MemberAttributeChange.h"
#include "hazelcast/client/common/containers/ManagedPointerVector.h"
#include "hazelcast/client/common/containers/ManagedArray.h"
#include "hazelcast/client/protocol/ProtocolTypeDefs.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage : public common::containers::LittleEndianBufferWrapper {

            public:
                /**
                * Current protocol version
                */
                static const uint8_t VERSION = 0;

                /**
                * Begin Flag
                */
                static const uint8_t BEGIN_FLAG = 0x80;
                /**
                * End Flag
                */
                static const uint8_t END_FLAG = 0x40;
                /**
                * Begin and End Flags
                */
                static const uint8_t BEGIN_AND_END_FLAGS = (BEGIN_FLAG | END_FLAG);

                /**
                * Listener Event Flag
                */
                static const uint8_t LISTENER_EVENT_FLAG = 0x01;

                static const int32_t FRAME_LENGTH_FIELD_OFFSET = 0;
                static const int32_t VERSION_FIELD_OFFSET = FRAME_LENGTH_FIELD_OFFSET + INT32_SIZE;
                static const int32_t FLAGS_FIELD_OFFSET = VERSION_FIELD_OFFSET + UINT8_SIZE;
                static const int32_t TYPE_FIELD_OFFSET = FLAGS_FIELD_OFFSET + UINT8_SIZE;
                static const int32_t CORRELATION_ID_FIELD_OFFSET = TYPE_FIELD_OFFSET + UINT16_SIZE;
                static const int32_t PARTITION_ID_FIELD_OFFSET = CORRELATION_ID_FIELD_OFFSET + INT32_SIZE;
                static const int32_t DATA_OFFSET_FIELD_OFFSET = PARTITION_ID_FIELD_OFFSET + INT32_SIZE;

                /**
                * ClientMessage Fixed Header size in bytes
                */
                static const uint16_t HEADER_SIZE = DATA_OFFSET_FIELD_OFFSET + UINT16_SIZE;

                ClientMessage();

                virtual ~ClientMessage();

                inline void wrapForDecode(byte *buffer, int32_t size, bool owner) {
                    isOwner = owner;
                    wrapForRead(buffer, size, HEADER_SIZE);
                    header = reinterpret_cast<MessageHeaderType *> (buffer);
                }

                static std::auto_ptr<ClientMessage> createForEncode(int32_t size);

                static std::auto_ptr<ClientMessage> create(int32_t size);

                //----- Setter methods begin --------------------------------------
                inline void setFrameLength(int32_t length) {
                    util::Bits::nativeToLittleEndian4(&length, &header->frameLength);
                }

                inline void setMessageType(uint16_t type) {
                    util::Bits::nativeToLittleEndian2(&type, &header->type);
                }

                inline void setVersion(uint8_t value) {
                    header->version = value;
                }

                inline void setFlags(uint8_t value) {
                    header->flags = value;
                }

                inline void setCorrelationId(uint32_t id) {
                    util::Bits::nativeToLittleEndian4(&id, &header->correlationId);
                }

                inline void setPartitionId(int32_t partitionId) {
                    util::Bits::nativeToLittleEndian4(&partitionId, &header->partitionId);
                }

                inline void setDataOffset(uint16_t offset) {
                    util::Bits::nativeToLittleEndian2(&offset, &header->dataOffset);
                }

                inline void updateFrameLength() {
                    setFrameLength(getIndex());
                }

                inline ClientMessage &set(const std::string & value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(uint8_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(bool value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(char value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }


                inline ClientMessage &set(uint16_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(int16_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(uint32_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(int32_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(uint64_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                inline ClientMessage &set(int64_t value) {
                    common::containers::LittleEndianBufferWrapper::set(value);
                    return *this;
                }

                #ifdef HZ_PLATFORM_DARWIN
                inline ClientMessage &set(long value) {
                    common::containers::LittleEndianBufferWrapper::set((int64_t)value);
                    return *this;
                }
                #endif

                inline ClientMessage &set(const serialization::pimpl::Data &data) {
                    set<std::vector<byte> >(data.toByteArray());
                    return *this;
                }

                inline ClientMessage &set(const Address &address) {
                    set(address.getHost()).set(address.getPort());
                    return *this;
                }

                template <typename T>
                ClientMessage &set(const std::vector<T> &values) {
                    int32_t len = (int32_t)values.size();
                    set(len);

                    if (len > 0) {
                        for (std::vector<T>::const_iterator it = values.begin();it != values.end(); ++it) {
                            set(*it);
                        }
                    }
                    return *this;
                }

                template <typename K, typename V>
                ClientMessage &set(const std::map<K, V> &values) {
                    int32_t len = (int32_t)values.size();
                    set(len);

                    if (len > 0) {
                        for (std::map<K, V>::const_iterator it = values.begin();it != values.end(); ++it) {
                            set((*it).first);
                            set((*it).second);
                        }
                    }
                    return *this;
                }
                //----- Setter methods end ---------------------

                /**
                * Tries to read enough bytes to fill the message from the provided ByteBuffer
                */
                bool fillMessageFrom(util::ByteBuffer &buffer);

                //----- Getter methods begin -------------------
                inline int32_t getFrameLength() const {
                    int32_t result;

                    util::Bits::littleEndianToNative4(
                            &header->frameLength, &result);

                    return result;
                }

                inline uint16_t getMessageType() const {
                    uint16_t type;

                    util::Bits::littleEndianToNative2(&header->type, &type);

                    return type;
                }

                inline uint8_t getVersion() {
                    return header->version;
                }

                inline uint32_t getCorrelationId() const {
                    uint32_t value;
                    util::Bits::littleEndianToNative4(&header->correlationId, &value);
                    return value;
                }

                inline int32_t getPartitionId() const {
                    int32_t value;
                    util::Bits::littleEndianToNative4(&header->partitionId, &value);
                    return value;
                }

                inline uint16_t getDataOffset() const {
                    uint16_t value;
                    util::Bits::littleEndianToNative2(&header->dataOffset, &value);
                    return value;
                }

                inline bool isFlagSet(uint8_t flag) const {
                    return flag == (header->flags & flag);
                }

                //-----Getters that change the index position---------
                template <typename T>
                T get() {
                    #error "Data type is not supported by the protocol."
                    return T();
                }

                template <typename T>
                std::auto_ptr<T> getNullable() {
                    std::auto_ptr<T> result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<T>(new T(get<T>()));
                }

                template <typename T>
                std::vector<T> getArray() {
                    int32_t len = getInt32();

                    std::vector<T> result(len);
                    for (int i = 0; i < len; ++i) {
                        result[i] = get<T>();
                    }
                    return result;
                }

                template <typename T>
                std::auto_ptr<std::vector<T> > getNullableArray() {
                    std::auto_ptr<T> result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<std::vector<T> >(new std::vector<T>(getArray<T>()));
                }

                template <typename KEY, typename VALUE>
                std::map<KEY, VALUE > getMap() {
                    int32_t len = getInt32();

                    std::map<KEY, VALUE> result;
                    for (int i = 0; i < len; ++i) {
                        KEY k = get<KEY>();
                        VALUE v = get<VALUE>();
                        result[k] = v;
                    }
                    return result;
                }

                template <typename KEY, typename VALUE>
                std::auto_ptr<std::map<KEY, VALUE > > getNullableMap() {
                    std::auto_ptr<std::map<KEY, VALUE > > result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<std::map<KEY, VALUE > >(new std::map<KEY, VALUE >(getMap<KEY, VALUE>()));
                }
                //----- Getter methods end --------------------------

                //----- Data size calculation functions BEGIN -------
                static inline int32_t calculateDataSize(uint8_t param) {
                    return UINT8_SIZE;
                }

                static inline int32_t calculateDataSize(int8_t param) {
                    return INT8_SIZE;
                }

                static inline int32_t calculateDataSize(bool param) {
                    return UINT8_SIZE;
                }

                static inline int32_t calculateDataSize(int16_t param) {
                    return INT16_SIZE;
                }

                static inline int32_t calculateDataSize(uint16_t param) {
                    return UINT16_SIZE;
                }

                static inline int32_t calculateDataSize(int32_t param) {
                    return INT32_SIZE;
                }

                static inline int32_t calculateDataSize(uint32_t param) {
                    return UINT32_SIZE;
                }

                static inline int32_t calculateDataSize(int64_t param) {
                    return INT64_SIZE;
                }

                #ifdef HZ_PLATFORM_DARWIN
                static inline int32_t calculateDataSize(long param) {
                    return calculateDataSize((int64_t)param);
                }
                #endif

                static inline int32_t calculateDataSize(uint64_t param) {
                    return UINT64_SIZE;
                }

                static inline int32_t calculateDataSize(const std::string &param) {
                    return INT32_SIZE +  // bytes for the length field
                            UTF8_MAX_CHAR_SIZE * param.length();
                }

                static inline int32_t calculateDataSize(const Address &param) {
                    // TODO: double check if port should be written 4 bytes or 2 bytes
                    return calculateDataSize(param.getHost()) + calculateDataSize((int32_t)param.getPort());
                }

                static inline int32_t calculateDataSize(const serialization::pimpl::Data &param) {
                    return INT32_SIZE +  // bytes for the length field
                        (int32_t)param.totalSize();
                }

                static inline int32_t calculateDataSize(const protocol::DataArray &values) {
                    int32_t totalSize = 0;

                    int32_t len = (int32_t)values.size();
                    if (len > 0) {
                        for (protocol::DataArray::VECTOR_TYPE::const_iterator it = values.begin();
                             it != values.end(); ++it) {
                            totalSize += INT32_SIZE; // bytes for the length field
                            totalSize += calculateDataSize(**it);
                        }
                    }

                    return totalSize;
                }

                static inline int32_t calculateDataSize(const std::set<serialization::pimpl::Data> &values) {
                    int32_t totalSize = INT32_SIZE;  // bytes for the length field

                    int32_t len = (int32_t)values.size();
                    if (len > 0) {
                        for (std::set<serialization::pimpl::Data>::const_iterator it = values.begin();
                             it != values.end(); ++it) {
                            totalSize += calculateDataSize(*it);
                        }
                    }

                    return totalSize;
                }

                static inline int32_t calculateDataSize(const std::vector<serialization::pimpl::Data> &values) {
                    int32_t totalSize = INT32_SIZE;  // bytes for the length field

                    int32_t len = (int32_t)values.size();
                    if (len > 0) {
                        for (std::vector<serialization::pimpl::Data>::const_iterator it = values.begin();
                             it != values.end(); ++it) {
                            totalSize += calculateDataSize(*it);
                        }
                    }

                    return totalSize;
                }
                //----- Data size calculation functions END ---------

                //Builder function
                void append(const ClientMessage *msg);

                int32_t getDataSize() const;

                bool isRetryable() const;

                void setRetryable(bool shouldRetry);

                bool isBindToSingleConnection() const;

                void setIsBoundToSingleConnection(bool isSingleConnection);

                /**
                 * Returns true if all bytes of the frame are written to Socket, false otherwise.
                 **/
                bool writeTo(Socket &socket);

            private:
                struct MessageHeaderType {
                    int32_t frameLength;
                    uint8_t version;
                    uint8_t flags;
                    uint16_t type;
                    uint32_t correlationId;
                    int32_t partitionId;
                    uint16_t dataOffset;
                };

                ClientMessage(int32_t size);

                inline void wrapForEncode(byte *buffer, int32_t size, bool owner);

                void ensureBufferSize(int32_t newCapacity);

                int32_t findSuitableCapacity(int32_t requiredCapacity, int32_t existingCapacity) const;

                static const struct MessageHeaderType getDefaultHeader();

                bool isOwner;
                byte *byteBuffer;
                struct MessageHeaderType *header;
                int32_t numBytesWrittenToConnection;
                int32_t numBytesFilled;

                bool retryable;
                bool isBoundToSingleConnection;

                static struct MessageHeaderType DEFAULT_HEADER;
            };
        }

        inline std::auto_ptr<ManagedInt32Array> getInt32Array() {
            int32_t len = getInt32();
            int32_t numBytes = len * INT32_SIZE;

            const byte *bytes = getBytes(numBytes);
            int32_t *buffer = new int32_t[len];
            memcpy(buffer, bytes, (size_t)numBytes);

            util::Bits::littleEndianToNativeArray4(len, buffer);

            std::auto_ptr<ManagedInt32Array> result(
                    new ManagedInt32Array(len, buffer));

            return result;
        }

        inline std::auto_ptr<std::vector<byte> > getByteArray() {
            int32_t len = getInt32();

            if (len > 0) {
                const byte *bytes = getBytes(len);
                return std::auto_ptr<std::vector<byte> >(new std::vector<byte>(bytes, bytes + len));
            } else {
                return std::auto_ptr<std::vector<byte> >();
            }
        }

        inline std::auto_ptr<Address> getAddress() {
            // read 1 byte to see if null
            bool isNull = getBoolean();
            if (isNull) {
                return std::auto_ptr<Address>(new Address());
            }
            std::auto_ptr<std::string> host(getStringUtf8());
            int32_t port = getInt32();
            return std::auto_ptr<Address>(new Address(host, port));
        }

        inline std::auto_ptr<AddressArray> getAddressList() {
            int32_t len = getInt32();

            std::auto_ptr<AddressArray> result(
                    new AddressArray(len));

            for (int i = 0; i < len; ++i) {
                std::auto_ptr<Address> ptr = getAddress();
                result->push_back(ptr.release());
            }
            return result;
        }

        template <>
        serialization::pimpl::Data ClientMessage::get() {
            return serialization::pimpl::Data(getByteArray());
        }

        template <>
        Member ClientMessage::get() {
            std::auto_ptr<Address> address = getAddress();
            std::auto_ptr<std::string> uuid = getStringUtf8();
            bool isLite = getBoolean();
            int32_t numAttributes = getInt32();

            std::auto_ptr<std::map<std::string, std::string > > attributes(
                    new std::map<std::string, std::string >());

            for (int i = 0; i < numAttributes; ++i) {
                std::auto_ptr<std::string> key = getStringUtf8();
                std::auto_ptr<std::string> value = getStringUtf8();
                (*attributes)[*key] = *value;
            }

            return std::auto_ptr<Member>(new Member(address, uuid, attributes, isLite));
        }


        inline std::auto_ptr<MemberArray> getMemberList() {
            int32_t len = getInt32();

            std::auto_ptr<MemberArray> result(new MemberArray(len));

            for (int i = 0; i < len; ++i) {
                std::auto_ptr<Member> ptr = getMember();
                result->push_back(ptr.release());
            }

            return result;
        }

        inline std::auto_ptr<impl::MemberAttributeChange> getMemberAttributeChange() {
            std::auto_ptr<std::string> uuid = getStringUtf8();
            std::auto_ptr<std::string> key = getStringUtf8();
            MemberAttributeEvent::MemberAttributeOperationType operationType = (MemberAttributeEvent::MemberAttributeOperationType)getInt32();

            std::auto_ptr<std::string> value = std::auto_ptr<std::string>(new std::string());
            if (operationType == MemberAttributeEvent::PUT) {
                value = getStringUtf8();
            }

            return std::auto_ptr<impl::MemberAttributeChange>(
                    new impl::MemberAttributeChange(uuid, operationType, key, value));

        }


        // Generate the types
        template class common::containers::ManagedPointerVector<Member>;
        template class common::containers::ManagedPointerVector<Address>;
        template class common::containers::ManagedPointerVector<serialization::pimpl::Data>;
        template class common::containers::ManagedArray<int32_t>;
    }
}

#endif /* HAZELCAST_CLIENT_CLIENTMESSAGE_H_ */
