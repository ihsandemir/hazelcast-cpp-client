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
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan demir
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
#include <assert.h>
#include <map>

#include "hazelcast/client/common/containers/LittleEndianBufferWrapper.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }

    namespace client {
        class Address;

        class Member;

        class Socket;

        class DistributedObjectInfo;

        namespace map {
            class DataEntryView;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace impl {
            class MemberAttributeChange;
        }

        namespace protocol {
            namespace codec {
                class StackTraceElement;
            }

            class HAZELCAST_API ClientTypes {
            public:
                static const std::string CPP;
            };

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

                ClientMessage(int32_t size);

                virtual ~ClientMessage();

                void wrapForDecode(byte *buffer, int32_t size, bool owner);

                static ClientMessage createForEncode(int32_t size);

                //----- Setter methods begin --------------------------------------
                // bring base class set methods into the derived class
                using common::containers::LittleEndianBufferWrapper::set;

                void setFrameLength(int32_t length);

                void setMessageType(uint16_t type);

                void setVersion(uint8_t value);

                void setFlags(uint8_t value);

                void setCorrelationId(uint32_t id);

                void setPartitionId(int32_t partitionId);

                void setDataOffset(uint16_t offset);

                void updateFrameLength();

                template<typename T>
                void setNullable(const T *value) {
                    bool isNull = (NULL == value);
                    set(isNull);
                    if (!isNull) {
                        set(*value);
                    }
                }

                void set(const std::string *data);

                void set(const serialization::pimpl::Data &data);

                void set(const serialization::pimpl::Data *data);

                void set(const Address &value);

                void set(const Address *value);

                void set(const Member &value);

                void set(const Member *value);

                void set(const map::DataEntryView &value);

                void set(const map::DataEntryView *value);

                void set(const DistributedObjectInfo &value);

                void set(const DistributedObjectInfo *value);

                template<typename K, typename V>
                void set(const std::pair<K, V> &entry) {
                    set(entry.first);
                    set(entry.second);
                }

                template<typename T>
                void setArray(const std::vector<T> &values) {
                    int32_t len = (int32_t) values.size();
                    set(len);

                    if (len > 0) {
                        for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); ++it) {
                            set(*it);
                        }
                    }
                }

                template<typename T>
                void setArray(const std::vector<T> *value) {
                    bool isNull = (NULL == value);
                    set(isNull);
                    if (!isNull) {
                        setArray<T>(*value);
                    }
                }

                template<typename K, typename V>
                void setEntryArray(const std::vector<std::pair<K, V> > &values) {
                    setArray<std::pair<K, V> >(values);
                }

                template<typename K, typename V>
                void setMap(const std::map<K, V> &values) {
                    int32_t len = (int32_t) values.size();
                    set(len);

                    if (len > 0) {
                        for (typename std::map<K, V>::const_iterator it = values.begin(); it != values.end(); ++it) {
                            set(*it);
                        }
                    }
                }

                template<typename K, typename V>
                void setNullableEntryArray(const std::vector<std::pair<K, V> > *values) {
                    setArray<std::pair<K, V> >(values);
                }
                //----- Setter methods end ---------------------

                /**
                * Tries to read enough bytes to fill the message from the provided ByteBuffer
                */
                bool fillMessageFrom(util::ByteBuffer &buffer);

                //----- Getter methods begin -------------------
                int32_t getFrameLength() const;

                uint16_t getMessageType() const;

                uint8_t getVersion();

                uint32_t getCorrelationId() const;

                int32_t getPartitionId() const;

                uint16_t getDataOffset() const;

                bool isFlagSet(uint8_t flag) const;

                //-----Getters that change the index position---------
                template<typename T>
                T get() {
                    assert(0);
                }

                template<typename KEY, typename VALUE>
                std::pair<KEY, VALUE> getEntry() {
                    return std::pair<KEY, VALUE>(get<KEY>(), get<VALUE>());
                }

                template<typename T>
                std::auto_ptr<T> getNullable() {
                    std::auto_ptr<T> result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<T>(new T(get<T>()));
                }

                template<typename T>
                std::vector<T> getArray() {
                    int32_t len = getInt32();

                    std::vector<T> result(len);
                    for (int i = 0; i < len; ++i) {
                        result[i] = get<T>();
                    }
                    return result;
                }

                template<typename T>
                std::auto_ptr<std::vector<T> > getNullableArray() {
                    std::auto_ptr<std::vector<T> > result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<std::vector<T> >(new std::vector<T>(getArray<T>()));
                }

                template<typename KEY, typename VALUE>
                std::vector<std::pair<KEY, VALUE> > getEntryArray() {
                    int32_t len = getInt32();

                    std::vector<std::pair<KEY, VALUE> > result(len);
                    for (int i = 0; i < len; ++i) {
                        result[i] = getEntry<KEY, VALUE>();
                    }
                    return result;
                }

                template<typename KEY, typename VALUE>
                std::auto_ptr<std::vector<std::pair<KEY, VALUE> > > getNullableEntryArray() {
                    std::auto_ptr<std::vector<std::pair<KEY, VALUE> > > result;
                    if (getBoolean()) {
                        return result;
                    }

                    return std::auto_ptr<std::vector<std::pair<KEY, VALUE> > >(
                            new std::vector<std::pair<KEY, VALUE> >(getEntryArray<KEY, VALUE>()));
                }
                //----- Getter methods end --------------------------

                //----- Data size calculation functions BEGIN -------
                static int32_t calculateDataSize(uint8_t param);

                static int32_t calculateDataSize(int8_t param);

                static int32_t calculateDataSize(bool param);

                static int32_t calculateDataSize(int16_t param);

                static int32_t calculateDataSize(uint16_t param);

                static int32_t calculateDataSize(int32_t param);

                static int32_t calculateDataSize(uint32_t param);

                static int32_t calculateDataSize(int64_t param);

#ifdef HZ_PLATFORM_DARWIN

                static int32_t calculateDataSize(long param);

#endif

                static int32_t calculateDataSize(uint64_t param);

                template<typename T>
                static int32_t calculateDataSizeNullable(const T *param) {
                    int32_t size = INT8_SIZE;
                    if (NULL != param) {
                        size += calculateDataSize(*param);
                    }
                    return size;
                }

                static int32_t calculateDataSize(const std::string &param);

                static int32_t calculateDataSize(const std::string *param);

                static int32_t calculateDataSize(const serialization::pimpl::Data &param);

                static int32_t calculateDataSize(const serialization::pimpl::Data *param);

                static int32_t calculateDataSize(const Address &param);

                static int32_t calculateDataSize(const Address *param);

                static int32_t calculateDataSize(const Member &param);

                static int32_t calculateDataSize(const Member *param);

                static int32_t calculateDataSize(const map::DataEntryView &param);

                static int32_t calculateDataSize(const map::DataEntryView *param);

                static int32_t calculateDataSize(const DistributedObjectInfo &param);

                static int32_t calculateDataSize(const DistributedObjectInfo *param);

                template<typename K, typename V>
                static int32_t calculateDataSize(const std::pair<K, V> &param) {
                    return calculateDataSize(param.first) + calculateDataSize(param.second);
                }

                template<typename T>
                static int32_t calculateDataSize(const std::vector<T> &param) {
                    int32_t dataSize = INT32_SIZE;
                    for (typename std::vector<T>::const_iterator it = param.begin(); param.end() != it; ++it) {
                        dataSize += calculateDataSize(*it);
                    }
                    return dataSize;
                }

                template<typename T>
                static int32_t calculateDataSize(const std::vector<T> *param) {
                    int32_t size = INT8_SIZE;
                    if (NULL != param) {
                        size += calculateDataSize<T>(*param);
                    }
                    return size;
                }

                template<typename KEY, typename VALUE>
                static int32_t calculateDataSize(const std::vector<std::pair<KEY, VALUE> > &param) {
                    int32_t size = INT32_SIZE;
                    for (typename std::vector<std::pair<KEY, VALUE> >::const_iterator it = param.begin();
                         param.end() != it; ++it) {
                        size += calculateDataSize<KEY, VALUE>(*it);
                    }
                    return size;
                }

                template<typename KEY, typename VALUE>
                static int32_t calculateDataSize(const std::vector<std::pair<KEY, VALUE> > *param) {
                    int32_t size = INT8_SIZE;

                    if (NULL != param) {
                        size += calculateDataSize<KEY, VALUE>(*param);
                    }

                    return size;
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

                void wrapForEncode(byte *buffer, int32_t size, bool owner);

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

            template<>
            uint8_t ClientMessage::get();

            template<>
            int8_t ClientMessage::get();

            template<>
            int16_t ClientMessage::get();

            template<>
            uint16_t ClientMessage::get();

            template<>
            int32_t ClientMessage::get();

            template<>
            uint32_t ClientMessage::get();

            template<>
            int64_t ClientMessage::get();

            template<>
            uint64_t ClientMessage::get();

            template<>
            Address ClientMessage::get();

            template<>
            Member ClientMessage::get();

            template<>
            map::DataEntryView ClientMessage::get();

            template<>
            DistributedObjectInfo ClientMessage::get();

            template<>
            codec::StackTraceElement ClientMessage::get();

        }


    }
}

#endif /* HAZELCAST_CLIENT_CLIENTMESSAGE_H_ */
