/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API DataOutput {
                public:
                    static constexpr const size_t DEFAULT_SIZE = 4 * 1024;

                    DataOutput(bool dontWrite = false);

                    /**
                     *
                     * @return reference to the internal byte buffer.
                     */
                    inline const std::vector<byte> &toByteArray() const;

                    /**
                     *
                     * @param bytes The bytes to be appended to the current buffer
                     */
                    inline void appendBytes(const std::vector<byte> &bytes);

                    inline void writeZeroBytes(size_t numberOfBytes);

                    inline size_t position();

                    inline void position(size_t newPos);

                    /**
                    * @param value to be written
                    */
                    template <typename T>
                    typename std::enable_if<std::is_same<byte, typename std::remove_cv<T>::type>::value ||
                            std::is_same<char, typename std::remove_cv<T>::type>::value ||
                            std::is_same<bool, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int16_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int32_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<int64_t, typename std::remove_cv<T>::type>::value ||
                            std::is_same<float, typename std::remove_cv<T>::type>::value ||
                            std::is_same<double, typename std::remove_cv<T>::type>::value, void>::type
                    inline write(T) { if (isNoWrite) { return; } }

                    inline void write(const std::string *value);

                    template <typename T>
                    typename std::enable_if<std::is_same<std::string, T>::value ||
                                   std::is_same<HazelcastJsonValue, T>::value>::type
                    inline write(const T &) { if (isNoWrite) { return; }}

                    /**
                    * @param value to vector of values to be written. Only supported built-in values can be written.
                    */
                    template <typename T>
                    inline void write(const std::vector<T> &value);

                    template <typename T>
                    inline void write(const std::vector<T> *value);

                protected:
                    bool isNoWrite;
                    std::vector<byte> outputStream;

                    int getUTF8CharCount(const std::string &str);
                };

                template <>
                HAZELCAST_API void DataOutput::write(byte value);

                template <>
                HAZELCAST_API void DataOutput::write(char value);

                template <>
                HAZELCAST_API void DataOutput::write(bool value);

                template <>
                HAZELCAST_API void DataOutput::write(int16_t value);

                template <>
                HAZELCAST_API void DataOutput::write(int32_t value);

                template <>
                HAZELCAST_API void DataOutput::write(int64_t value);

                template <>
                HAZELCAST_API void DataOutput::write(float value);

                template <>
                HAZELCAST_API void DataOutput::write(double value);

                template <>
                HAZELCAST_API void DataOutput::write(const std::string &value);

                template <>
                HAZELCAST_API void DataOutput::write(const HazelcastJsonValue &value);

                void DataOutput::write(const std::string *value) {
                    if (isNoWrite) { return; }
                    if (!value) {
                        write<int32_t>(util::Bits::NULL_ARRAY);
                        return;
                    }

                    write(*value);
                }

                template <typename T>
                void DataOutput::write(const std::vector<T> &value) {
                    if (isNoWrite) { return; }
                    int32_t len = (int32_t) value.size();
                    write<int32_t>(len);
                    if (len > 0) {
                        for (auto &item : value) {
                            write(item);
                        }
                    }
                }

                template <typename T>
                void DataOutput::write(const std::vector<T> *value) {
                    if (isNoWrite) { return; }
                    if (!value) {
                        write<int32_t>(util::Bits::NULL_ARRAY);
                        return;
                    }

                    write(*value);
                }

                void DataOutput::writeZeroBytes(size_t numberOfBytes) {
                    outputStream.insert(outputStream.end(), numberOfBytes, 0);
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

