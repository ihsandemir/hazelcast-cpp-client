/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//  ObjectDataInput.h
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_DATA_INPUT
#define HAZELCAST_DATA_INPUT

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/Serializer.h"
#include "hazelcast/client/serialization/pimpl/SerializerHolder.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/TypeIDS.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <stdint.h>
#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;

            class IdentifiedDataSerializable;

            namespace pimpl {
                class PortableContext;

                class DataInput;

                class Data;
            }

            /**
            * Provides deserialization methods for primitives types, arrays of primitive types
            * Portable, IdentifiedDataSerializable and custom serializable types
            */
            class HAZELCAST_API ObjectDataInput {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(pimpl::DataInput&, pimpl::PortableContext&);

                /**
                * fills all content to given byteArray
                * @param byteArray to fill the data in
                */
                void readFully(std::vector<byte>& byteArray);

                /**
                *
                * @param i number of bytes to skip
                */
                int skipBytes(int i);

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                bool readBoolean();

                /**
                * @return the byte read
                * @throws IOException if it reaches end of file before finish reading
                */
                byte readByte();

                /**
                * @return the int16_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int16_t readShort();

                /**
                * @return the char read
                * @throws IOException if it reaches end of file before finish reading
                */
                char readChar();

                /**
                * @return the int32_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int32_t readInt();

                /**
                * @return the int64_t read
                * @throws IOException if it reaches end of file before finish reading
                */
                int64_t readLong();

                /**
                * @return the boolean read
                * @throws IOException if it reaches end of file before finish reading
                */
                float readFloat();

                /**
                * @return the double read
                * @throws IOException if it reaches end of file before finish reading
                */
                double readDouble();

                /**
                * @return the utf string read as an ascii string
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::string> readUTF();

                /**
                * @return the byte array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<byte> > readByteArray();

                /**
                * @return the boolean array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<bool> > readBooleanArray();

                /**
                * @return the char array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<char> > readCharArray();

                /**
                * @return the int32_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<int32_t> > readIntArray();

                /**
                * @return the int64_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<int64_t> > readLongArray();

                /**
                * @return the double array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<double> > readDoubleArray();

                /**
                * @return the float array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<float> > readFloatArray();

                /**
                * @return the int16_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<int16_t> > readShortArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::auto_ptr<std::vector<std::string> > readUTFArray();

                /**
                * Object can be Portable, IdentifiedDataSerializable or custom serializable
                * for custom serialization @see Serializer
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                std::auto_ptr<T> readObject() {
                    int32_t typeId = readInt();
                    switch (typeId) {
                        case pimpl::SerializationConstants::CONSTANT_TYPE_NULL:
                            return std::auto_ptr<T>();
                        case pimpl::SerializationConstants::CONSTANT_TYPE_DATA:
                        {
                            std::auto_ptr<IdentifiedDataSerializable> object = serializerHolder.getDataSerializer().read(
                                    *this);
                            if (NULL == object.get()) {
                                // Keep original behaviour for backward compatbility
                                #pragma clang diagnostic push
                                #pragma clang diagnostic ignored "-Wreinterpret-base-class"
                                object = std::auto_ptr<IdentifiedDataSerializable>(reinterpret_cast<IdentifiedDataSerializable *>(new T));
                                #pragma clang diagnostic pop
                                portableContext.getConstants().checkClassType(getHazelcastTypeId(object.get()), typeId);
                                readDataSerializable(object.get());
                            }
                            return std::auto_ptr<T>((T *)object.release());
                        }
                        case pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE:
                        {
                            std::auto_ptr<Portable> object = serializerHolder.getPortableSerializer().read(dataInput);
                            if (NULL == object.get()) {
                                // Keep original behaviour for backward compatbility
                                #pragma clang diagnostic push
                                #pragma clang diagnostic ignored "-Wreinterpret-base-class"
                                object = std::auto_ptr<Portable>(reinterpret_cast<Portable *>(new T));
                                #pragma clang diagnostic pop
                                portableContext.getConstants().checkClassType(getHazelcastTypeId(object.get()), typeId);
                                readPortable(object.get());
                            }
                            return std::auto_ptr<T>((T *)object.release());;
                        }
                        default:
                        {
                            return readInternal<T>(typeId);
                        }
                    }
                }

                /**
                * @return the data read
                * @throws IOException if it reaches end of file before finish reading
                */
                pimpl::Data readData();

                /**
                * @return current position index
                */
                int position();

                /**
                * Move cursor to given index
                * @param newPos new position index to be set
                */
                void position(int newPos);

                pimpl::DataInput &getDataInput() const;

            private:

                template <typename T>
                std::auto_ptr<T> readInternal(int typeId) {
                    boost::shared_ptr<SerializerBase> serializer = serializerHolder.serializerFor(typeId);
                    if (NULL == serializer.get()) {
                        const std::string message = "No serializer found for serializerId :"+
                                                     util::IOUtil::to_string(typeId) + ", typename :" +
                                                     typeid(T).name();
                        throw exception::HazelcastSerializationException("ObjectDataInput::readInternal", message);
                    }

                    std::auto_ptr<T> object(reinterpret_cast<T *>(serializer->create()));
                    if (NULL == object.get()) {
                        object = std::auto_ptr<T>(new T);
                        portableContext.getConstants().checkClassType(getHazelcastTypeId(object.get()), typeId);
                    }
                    
                    Serializer<T> *s = static_cast<Serializer<T> * >(serializer.get());
                    ObjectDataInput objectDataInput(dataInput, portableContext);
                    s->read(objectDataInput, *object);

                    return object;
                }

                void readPortable(Portable *object);

                void readDataSerializable(IdentifiedDataSerializable * object);

                pimpl::DataInput& dataInput;
                pimpl::PortableContext& portableContext;
                pimpl::SerializerHolder& serializerHolder;

                ObjectDataInput(const ObjectDataInput&);

                void operator=(const ObjectDataInput&);

            };

/*
            template <>
            HAZELCAST_API std::auto_ptr<T> ObjectDataInput::readInternal(int typeId, byte *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, bool *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, char *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, int16_t *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, int32_t *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, int64_t *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, float *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, double *object);

            template <>
            HAZELCAST_API void ObjectDataInput::readInternal(int typeId, std::string *object);
*/
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_DATA_INPUT */

