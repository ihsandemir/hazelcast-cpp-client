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

#include <set>
#include <type_traits>

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Portable.h"

#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/client/PartitionAware.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/Disposable.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class TypedData;
        namespace serialization {
            class ObjectDataInput;

            class ObjectDataOutput;

            namespace pimpl {
                // forward declarations
                class PortableContext;

                class ClassDefinitionContext;

                class ClassDefinitionWriter;

                class DefaultPortableWriter;

                class DefaultPortableReader;

                class MorphingPortableReader;

                class PortableSerializer;

                class DataSerializer;

                class SerializationService;
            }

            template<typename T>
            struct hz_serializer {
            };

            /**
             * Classes derived from this class should implement the following static methods:
             *      static int32_t getClassId() const noexcept;
             *      static int32_t getFactoryId() const noexcept;
             *      static int32_t writeData(ObjectDataOutput &out);
             *      static int32_t readData(ObjectDataInput &in) const;
             */
            struct identified_data_serializer {
            };

            /**
             * Classes derived from this class should implement the following static methods:
             *      static int32_t getClassId() const noexcept;
             *      static int32_t getFactoryId() const noexcept;
             *      static int32_t writePortable(PortableWriter &out);
             *      static int32_t readPortable(PortableReader &in) const;
             */
            struct portable_serializer {
            };

            struct versioned_portable_serializer : public portable_serializer {
            };

            template<>
            struct hz_serializer<std::string> {
                static int32_t getTypeId();
                static void write(const std::string &object, ObjectDataOutput &out);
                static std::string read(ObjectDataInput &in);
            };

            class HAZELCAST_API FieldType {
            public:
                FieldType();

                FieldType(int type);

                FieldType(FieldType const &rhs);

                const byte getId() const;

                FieldType &operator=(FieldType const &rhs);

                bool operator==(FieldType const &rhs) const;

                bool operator!=(FieldType const &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const FieldType &type);

                byte id;
            };

            namespace FieldTypes {
                static const FieldType TYPE_PORTABLE(0);
                static const FieldType TYPE_BYTE(1);
                static const FieldType TYPE_BOOLEAN(2);
                static const FieldType TYPE_CHAR(3);
                static const FieldType TYPE_SHORT(4);
                static const FieldType TYPE_INT(5);
                static const FieldType TYPE_LONG(6);
                static const FieldType TYPE_FLOAT(7);
                static const FieldType TYPE_DOUBLE(8);
                static const FieldType TYPE_UTF(9);
                static const FieldType TYPE_PORTABLE_ARRAY(10);
                static const FieldType TYPE_BYTE_ARRAY(11);
                static const FieldType TYPE_BOOLEAN_ARRAY(12);
                static const FieldType TYPE_CHAR_ARRAY(13);
                static const FieldType TYPE_SHORT_ARRAY(14);
                static const FieldType TYPE_INT_ARRAY(15);
                static const FieldType TYPE_LONG_ARRAY(16);
                static const FieldType TYPE_FLOAT_ARRAY(17);
                static const FieldType TYPE_DOUBLE_ARRAY(18);
                static const FieldType TYPE_UTF_ARRAY(19);
            }

            /**
            * ClassDefinition defines a class schema for Portable classes. It allows to query field names, types, class id etc.
            * It can be created manually using {@link ClassDefinitionBuilder}
            * or on demand during serialization phase.
            *
            * @see Portable
            * @see ClassDefinitionBuilder
            */
            /**
            * FieldDefinition defines name, type, index of a field
            */
            class HAZELCAST_API FieldDefinition {
            public:

                /**
                * Constructor
                */
                FieldDefinition();

                /**
                * Constructor
                */
                FieldDefinition(int, const std::string &, FieldType const &type, int version);

                /**
                * Constructor
                */
                FieldDefinition(int index, const std::string &fieldName, FieldType const &type, int factoryId,
                                int classId, int version);

                /**
                * @return field type
                */
                const FieldType &getType() const;

                /**
                * @return field name
                */
                std::string getName() const;

                /**
                * @return field index
                */
                int getIndex() const;

                /**
                * @return factory id of this field's class
                */
                int getFactoryId() const;

                /**
                * @return class id of this field's class
                */
                int getClassId() const;


                /**
                * @param dataOutput writes fieldDefinition to given dataOutput
                */
                void writeData(pimpl::DataOutput &dataOutput);

                /**
                * @param dataInput reads fieldDefinition from given dataOutput
                */
                void readData(pimpl::DataInput &dataInput);

                bool operator==(const FieldDefinition &rhs) const;

                bool operator!=(const FieldDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition);

            private:
                int index;
                std::string fieldName;
                FieldType type;
                int classId;
                int factoryId;
                int version;
            };

            class HAZELCAST_API ClassDefinition {
            public:

                /**
                * Constructor
                */
                ClassDefinition();

                /**
                * Constructor
                * @param factoryId factory id of class
                * @param classId id of class
                * @param version version of portable class
                */
                ClassDefinition(int factoryId, int classId, int version);

                /**
                * Internal API
                * @param fieldDefinition to be added
                */
                void addFieldDef(FieldDefinition &fieldDefinition);

                /**
                * @param fieldName field name
                * @return true if this class definition contains a field named by given name
                */
                bool hasField(char const *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return field definition by given name
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition &getField(const char *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return type of given field
                * @throws IllegalArgumentException
                */
                FieldType getFieldType(const char *fieldName) const;

                /**
                * @return total field count
                */
                int getFieldCount() const;

                /**
                * @return factory id
                */
                int getFactoryId() const;

                /**
                * @return class id
                */
                int getClassId() const;

                /**
                * @return version
                */
                int getVersion() const;

                /**
                * Internal API
                * @param version portable version
                */
                void setVersionIfNotSet(int version);

                /**
                * Internal API
                * @param dataOutput
                */
                void writeData(pimpl::DataOutput &dataOutput);

                /**
                * Internal API
                * @param dataInput
                */
                void readData(pimpl::DataInput &dataInput);

                bool operator==(const ClassDefinition &rhs) const;

                bool operator!=(const ClassDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition);

            private:
                int factoryId;
                int classId;
                int version;

                ClassDefinition(const ClassDefinition &);

                ClassDefinition &operator=(const ClassDefinition &rhs);

                std::map<std::string, FieldDefinition> fieldDefinitionsMap;

                std::unique_ptr<std::vector<byte> > binary;

            };

            /**
            * ClassDefinitionBuilder is used to build and register ClassDefinitions manually.
            *
            * @see ClassDefinition
            * @see Portable
            * @see SerializationConfig#addClassDefinition(ClassDefinition)
            */
            class HAZELCAST_API ClassDefinitionBuilder {
            public:
                ClassDefinitionBuilder(int factoryId, int classId, int version);

                ClassDefinitionBuilder &addIntField(const std::string &fieldName);

                ClassDefinitionBuilder &addLongField(const std::string &fieldName);

                ClassDefinitionBuilder &addUTFField(const std::string &fieldName);

                ClassDefinitionBuilder &addBooleanField(const std::string &fieldName);

                ClassDefinitionBuilder &addByteField(const std::string &fieldName);

                ClassDefinitionBuilder &addCharField(const std::string &fieldName);

                ClassDefinitionBuilder &addDoubleField(const std::string &fieldName);

                ClassDefinitionBuilder &addFloatField(const std::string &fieldName);

                ClassDefinitionBuilder &addShortField(const std::string &fieldName);

                ClassDefinitionBuilder &addByteArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addBooleanArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addCharArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addIntArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addLongArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addDoubleArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addFloatArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &addShortArrayField(const std::string &fieldName);

                ClassDefinitionBuilder &
                addPortableField(const std::string &fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &
                addPortableArrayField(const std::string &fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder &addField(FieldDefinition &fieldDefinition);

                std::shared_ptr<ClassDefinition> build();

                int getFactoryId();

                int getClassId();

                int getVersion();

            private:
                int factoryId;
                int classId;
                int version;
                int index;
                bool done;
                std::vector<FieldDefinition> fieldDefinitions;

                void addField(const std::string &fieldName, FieldType const &fieldType);

                void check();
            };

            class PortableVersionHelper {
            public:
                template<typename T>
                static inline typename std::enable_if<std::is_base_of<versioned_portable_serializer, hz_serializer<T>>::value, int>::type
                getVersion(int) {
                    return hz_serializer<T>::getClassVersion();
                }

                template<typename T>
                static inline int getVersion(int defaultVersion) {
                    return defaultVersion;
                }
            };

            /**
            * Provides a mean of reading portable fields from a binary in form of java primitives
            * arrays of java primitives , nested portable fields and array of portable fields.
            */
            class HAZELCAST_API PortableReader {
            public:
                PortableReader(pimpl::PortableSerializer &portableSer, ObjectDataInput &dataInput,
                               std::shared_ptr<ClassDefinition> cd, bool isDefaultReader);

                PortableReader(const PortableReader &reader);

                PortableReader &operator=(const PortableReader &reader);

                /**
                * @param fieldName name of the field
                * @return the int32_t value read
                * @throws IOException
                */
                int32_t readInt(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int64_t value read
                * @throws IOException
                */
                int64_t readLong(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the boolean value read
                * @throws IOException
                */
                bool readBoolean(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte value read
                * @throws IOException
                */
                byte readByte(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char value read
                * @throws IOException
                */
                char readChar(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double value read
                * @throws IOException
                */
                double readDouble(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float value read
                * @throws IOException
                */
                float readFloat(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int32_t value read
                * @throws IOException
                */
                int16_t readShort(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the utf string value read
                * @throws IOException
                */
                std::unique_ptr<std::string> readUTF(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the byte array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the bool array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<bool> > readBooleanArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the char array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<char> > readCharArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int32_t array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int64_t array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the double array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the float array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                /**
                * @param fieldName name of the field
                * @return the int16_t array value read
                * @throws IOException
                */
                std::unique_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @return the portable value read
                * @throws IOException
                */
                template<typename T>
                std::shared_ptr<T> readPortable(const char *fieldName);

                /**
                * @tparam type of the portable class in array
                * @param fieldName name of the field
                * @return the portable array value read
                * @throws IOException
                */
                template<typename T>
                std::vector<boost::optional<T>> readPortableArray(const char *fieldName);

                /**
                * @see PortableWriter#getRawDataOutput
                *
                * Note that portable fields can not read after getRawDataInput() is called. In case this happens,
                * IOException will be thrown.
                *
                * @return rawDataInput
                * @throws IOException
                */
                ObjectDataInput &getRawDataInput();

                /**
                * Internal Api. Should not be called by end user.
                */
                void end();

            private:
                bool isDefaultReader;
                mutable std::unique_ptr<pimpl::DefaultPortableReader> defaultPortableReader;
                mutable std::unique_ptr<pimpl::MorphingPortableReader> morphingPortableReader;

            };

            /**
* Provides a mean of writing portable fields to a binary in form of java primitives
* arrays of java primitives , nested portable fields and array of portable fields.
*/
            class HAZELCAST_API PortableWriter {
            public:
                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter);

                /**
                * Internal api constructor
                */
                PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter);

                /**
                * @param fieldName name of the field
                * @param value     int32_t value to be written
                * @throws IOException
                */
                void writeInt(const char *fieldName, int32_t value);

                /**
                * @param fieldName name of the field
                * @param value int64_t value to be written
                * @throws IOException
                */
                void writeLong(const char *fieldName, int64_t value);

                /**
                * @param fieldName name of the field
                * @param value boolean value to be written
                * @throws IOException
                */
                void writeBoolean(const char *fieldName, bool value);

                /**
                * @param fieldName name of the field
                * @param value byte value to be written
                * @throws IOException
                */
                void writeByte(const char *fieldName, byte value);

                /**
                * @param fieldName name of the field
                * @param value  char value to be written
                * @throws IOException
                */
                void writeChar(const char *fieldName, int32_t value);

                /**
                * @param fieldName name of the field
                * @param value double value to be written
                * @throws IOException
                */
                void writeDouble(const char *fieldName, double value);

                /**
                * @param fieldName name of the field
                * @param value float value to be written
                * @throws IOException
                */
                void writeFloat(const char *fieldName, float value);

                /**
                * @param fieldName name of the field
                * @param value int16_t value to be written
                * @throws IOException
                */
                void writeShort(const char *fieldName, int16_t value);

                /**
                * @param fieldName name of the field
                * @param value utf string value to be written
                * @throws IOException
                */
                void writeUTF(const char *fieldName, const std::string *value);

                /**
                * @param fieldName name of the field
                * @param values byte array to be written
                * @throws IOException
                */
                void writeByteArray(const char *fieldName, const std::vector<byte> *values);

                /**
                * @param fieldName name of the field
                * @param values bool array to be written
                * @throws IOException
                */
                void writeBooleanArray(const char *fieldName, const std::vector<bool> *values);

                /**
                * @param fieldName name of the field
                * @param values     char array to be written
                * @throws IOException
                */
                void writeCharArray(const char *fieldName, const std::vector<char> *values);

                /**
                * @param fieldName name of the field
                * @param values int16_t array to be written
                * @throws IOException
                */
                void writeShortArray(const char *fieldName, const std::vector<int16_t> *values);

                /**
                * @param fieldName name of the field
                * @param values int32_t array to be written
                * @throws IOException
                */
                void writeIntArray(const char *fieldName, const std::vector<int32_t> *values);

                /**
                * @param fieldName name of the field
                * @param values     int64_t array to be written
                * @throws IOException
                */
                void writeLongArray(const char *fieldName, const std::vector<int64_t> *values);

                /**
                * @param fieldName name of the field
                * @param values    float array to be written
                * @throws IOException
                */
                void writeFloatArray(const char *fieldName, const std::vector<float> *values);

                /**
                * @param fieldName name of the field
                * @param values    double array to be written
                * @throws IOException
                */
                void writeDoubleArray(const char *fieldName, const std::vector<double> *values);

                /**
                * Internal api , should not be called by end user.
                */
                void end();

                /**
                * To write a null portable value.
                *
                * @tparam type of the portable field
                * @param fieldName name of the field
                *
                * @throws IOException
                */
                template<typename T>
                void writeNullPortable(const char *fieldName);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param portable  Portable to be written
                * @throws IOException
                */
                template<typename T>
                void writePortable(const char *fieldName, const T *portable);

                /**
                * @tparam type of the portable class
                * @param fieldName name of the field
                * @param values portable array to be written
                * @throws IOException
                */
                template<typename T>
                void writePortableArray(const char *fieldName, const std::vector<T> *values);

                /**
                * After writing portable fields, one can write remaining fields in old fashioned way consecutively at the end
                * of stream. User should not that after getting rawDataOutput trying to write portable fields will result
                * in IOException
                *
                * @return ObjectDataOutput
                * @throws IOException
                */
                ObjectDataOutput &getRawDataOutput();

            private:
                pimpl::DefaultPortableWriter *defaultPortableWriter;
                pimpl::ClassDefinitionWriter *classDefinitionWriter;
                bool isDefaultWriter;
            };

            /**
            * Provides deserialization methods for primitives types, arrays of primitive types
            * Portable, IdentifiedDataSerializable and custom serializable types
            */
            class HAZELCAST_API ObjectDataInput {
            public:
                /**
                * Internal API. Constructor
                */
                ObjectDataInput(pimpl::DataInput &dataInput, pimpl::PortableSerializer &portableSer,
                                pimpl::DataSerializer &dataSer);

                /**
                * fills all content to given byteArray
                * @param byteArray to fill the data in
                */
                void readFully(std::vector<byte> &byteArray);

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
                std::unique_ptr<std::string> readUTF();

                /**
                * @return the byte array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<byte> > readByteArray();

                /**
                * @return the boolean array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<bool> > readBooleanArray();

                /**
                * @return the char array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<char> > readCharArray();

                /**
                * @return the int32_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int32_t> > readIntArray();

                /**
                * @return the int64_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int64_t> > readLongArray();

                /**
                * @return the double array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<double> > readDoubleArray();

                /**
                * @return the float array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<float> > readFloatArray();

                /**
                * @return the int16_t array read
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<int16_t> > readShortArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<std::string> > readUTFArray();

                /**
                * @return the array of strings
                * @throws IOException if it reaches end of file before finish reading
                */
                std::unique_ptr<std::vector<std::string *> > readUTFPointerArray();

                /**
                * Object can be Portable, IdentifiedDataSerializable or custom serializable
                * for custom serialization @see Serializer
                * @return the object read
                * @throws IOException if it reaches end of file before finish reading
                */
                template<typename T>
                inline boost::optional<T> readObject();

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
                inline readObject(int32_t typeId);

                template<typename T>
                inline boost::optional<T> readObject(int32_t typeId);

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

            private:
                pimpl::DataInput &dataInput;
                pimpl::PortableSerializer &portableSerializer;
                pimpl::DataSerializer &dataSerializer;

                ObjectDataInput(const ObjectDataInput &);

                void operator=(const ObjectDataInput &);
            };

            /**
            * Provides serialization methods for primitive types,a arrays of primitive types, Portable,
            * IdentifiedDataSerializable and custom serializables.
            * For custom serialization @see Serializer
            */
            class HAZELCAST_API ObjectDataOutput {
            public:
                /**
                * Internal API Constructor
                */
                ObjectDataOutput(pimpl::DataOutput &dataOutput, pimpl::PortableSerializer &portableSer,
                                 pimpl::DataSerializer &dataSer);

                /**
                * Internal API Constructor
                */
                ObjectDataOutput();

                /**
                * @return copy of internal byte array
                */
                std::unique_ptr<std::vector<byte> > toByteArray();

                /**
                * Writes all the bytes in array to stream
                * @param bytes to be written
                */
                void write(const std::vector<byte> &bytes);

                /**
                * @param value the bool value to be written
                */
                void writeBoolean(bool value);

                /**
                * @param value the byte value to be written
                */
                void writeByte(int32_t value);

                /**
                 * @param bytes The data bytes to be written
                 * @param len Number of bytes to write
                 */
                void writeBytes(const byte *bytes, size_t len);

                /**
                * @param value the int16_t value to be written
                */
                void writeShort(int32_t value);

                /**
                * @param value the char value to be written
                */
                void writeChar(int32_t value);

                /**
                * @param value the int32_t value to be written
                */
                void writeInt(int32_t value);

                /**
                * @param value the int64_t  value to be written
                */
                void writeLong(int64_t value);

                /**
                * @param value the float value to be written
                */
                void writeFloat(float value);

                /**
                * @param value the double value to be written
                */
                void writeDouble(double value);

                /**
                * @param value the UTF string value to be written
                */
                void writeUTF(const std::string *value);

                /**
                * @param value the bytes to be written
                */
                void writeByteArray(const std::vector<byte> *value);

                /**
                * @param value the characters to be written
                */
                void writeCharArray(const std::vector<char> *value);

                /**
                * @param value the boolean values to be written
                */
                void writeBooleanArray(const std::vector<bool> *value);

                /**
                * @param value the int16_t array value to be written
                */
                void writeShortArray(const std::vector<int16_t> *value);

                /**
                * @param value the int32_t array value to be written
                */
                void writeIntArray(const std::vector<int32_t> *value);

                /**
                * @param value the int16_t array value to be written
                */
                void writeLongArray(const std::vector<int64_t> *value);

                /**
                * @param value the float array value to be written
                */
                void writeFloatArray(const std::vector<float> *value);

                /**
                * @param value the double array value to be written
                */
                void writeDoubleArray(const std::vector<double> *value);

                /**
                 * @param strings the array of strings to be serialized
                 */
                void writeUTFArray(const std::vector<std::string *> *strings);

                /**
                * @param value the data value to be written
                */
                void writeData(const pimpl::Data *value);

                template<typename T>
                void writeObject(const T *object);

                template<typename T>
                typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
                inline writeObject(const T &object);

                template<typename T>
                inline void writeObject(const T &object);

                pimpl::DataOutput *getDataOutput() const;

            private:
                pimpl::DataOutput *dataOutput;
                pimpl::PortableSerializer &portableSerializer;
                pimpl::DataSerializer &dataSerializer;
                bool isEmpty;

                size_t position();

                void position(size_t newPos);

                ObjectDataOutput(const ObjectDataOutput &);

                void operator=(const ObjectDataOutput &);
            };

            namespace pimpl {
                class HAZELCAST_API PortableContext {
                public:
                    PortableContext(const SerializationConfig &serializationConf);

                    int getClassVersion(int factoryId, int classId);

                    void setClassVersion(int factoryId, int classId, int version);

                    std::shared_ptr<ClassDefinition> lookupClassDefinition(int factoryId, int classId, int version);

                    std::shared_ptr<ClassDefinition> registerClassDefinition(std::shared_ptr<ClassDefinition>);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookupOrRegisterClassDefinition();

                    int getVersion();

                    std::shared_ptr<ClassDefinition> readClassDefinition(ObjectDataInput &input, int id, int classId,
                                                                         int version);

                    const SerializationConfig &getSerializationConfig() const;

                private:
                    PortableContext(const PortableContext &);

                    ClassDefinitionContext &getClassDefinitionContext(int factoryId);

                    void operator=(const PortableContext &);

                    util::SynchronizedMap<int, ClassDefinitionContext> classDefContextMap;
                    const SerializationConfig &serializationConfig;
                };

                class ClassDefinitionContext {
                public:

                    ClassDefinitionContext(int portableContext, PortableContext *pContext);

                    int getClassVersion(int classId);

                    void setClassVersion(int classId, int version);

                    std::shared_ptr<ClassDefinition> lookup(int, int);

                    std::shared_ptr<ClassDefinition> registerClassDefinition(std::shared_ptr<ClassDefinition>);

                private:
                    int64_t combineToLong(int x, int y) const;

                    const int factoryId;
                    util::SynchronizedMap<long long, ClassDefinition> versionedDefinitions;
                    util::SynchronizedMap<int, int> currentClassVersions;
                    PortableContext *portableContext;
                };

                class HAZELCAST_API ClassDefinitionWriter {
                public:
                    ClassDefinitionWriter(PortableContext &portableContext, ClassDefinitionBuilder &builder);

                    void writeInt(const char *fieldName, int32_t value);

                    void writeLong(const char *fieldName, int64_t value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int32_t value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, int16_t value);

                    void writeUTF(const char *fieldName, const std::string *str);

                    void writeByteArray(const char *fieldName, const std::vector<byte> *values);

                    void writeBooleanArray(const char *fieldName, const std::vector<bool> *values);

                    void writeCharArray(const char *fieldName, const std::vector<char> *data);

                    void writeShortArray(const char *fieldName, const std::vector<int16_t> *data);

                    void writeIntArray(const char *fieldName, const std::vector<int32_t> *data);

                    void writeLongArray(const char *fieldName, const std::vector<int64_t> *data);

                    void writeFloatArray(const char *fieldName, const std::vector<float> *data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double> *data);

                    template<typename T>
                    void writeNullPortable(const char *fieldName) {

                        T portable;
                        int32_t factoryId = portable.getFactoryId();
                        int32_t classId = portable.getClassId();
                        std::shared_ptr<ClassDefinition> nestedClassDef = context.lookupClassDefinition(factoryId,
                                                                                                        classId,
                                                                                                        context.getVersion());
                        if (nestedClassDef == NULL) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefWriter::writeNullPortable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }
                        builder.addPortableField(fieldName, nestedClassDef);
                    }

                    template<typename T>
                    void writePortable(const char *fieldName, const T *portable) {
                        if (NULL == portable) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("ClassDefinitionWriter::writePortable",
                                                                               "Cannot write null portable without explicitly registering class definition!"));
                        }

                        std::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef(*portable);
                        builder.addPortableField(fieldName, nestedClassDef);
                    };

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> *portables) {
                        if (NULL == portables || portables->size() == 0) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ClassDefinitionWriter::writePortableArray",
                                                                  "Cannot write null portable array without explicitly registering class definition!"));
                        }
                        std::shared_ptr<ClassDefinition> nestedClassDef = createNestedClassDef((*portables)[0]);
                        builder.addPortableArrayField(fieldName, nestedClassDef);
                    };

                    std::shared_ptr<ClassDefinition> registerAndGet();

                    ObjectDataOutput &getRawDataOutput();

                    void end();

                private:
                    std::shared_ptr<ClassDefinition> createNestedClassDef(const Portable &portable);

                    ObjectDataOutput emptyDataOutput;
                    ClassDefinitionBuilder &builder;
                    PortableContext &context;

                };

                class HAZELCAST_API PortableReaderBase {
                public:
                    PortableReaderBase(PortableSerializer &portableSer,
                                       ObjectDataInput &input,
                                       std::shared_ptr<ClassDefinition> cd);

                    virtual ~PortableReaderBase();

                    virtual int32_t readInt(const char *fieldName);

                    virtual int64_t readLong(const char *fieldName);

                    virtual bool readBoolean(const char *fieldName);

                    virtual byte readByte(const char *fieldName);

                    virtual char readChar(const char *fieldName);

                    virtual double readDouble(const char *fieldName);

                    virtual float readFloat(const char *fieldName);

                    virtual int16_t readShort(const char *fieldName);

                    virtual std::unique_ptr<std::string> readUTF(const char *fieldName);

                    virtual std::unique_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<bool> > readBooleanArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<char> > readCharArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                    virtual std::unique_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                    ObjectDataInput &getRawDataInput();

                    void end();

                protected:
                    void setPosition(char const *, FieldType const &fieldType);

                    void checkFactoryAndClass(FieldDefinition fd, int factoryId, int classId) const;

                    int readPosition(const char *, FieldType const &fieldType);

                    template<typename T>
                    boost::optional<T> getPortableInstance(char const *fieldName);

                    std::shared_ptr<ClassDefinition> cd;
                    ObjectDataInput &dataInput;
                    PortableSerializer &portableSerializer;
                private:
                    int finalPosition;
                    int offset;
                    bool raw;

                };

                class HAZELCAST_API DefaultPortableReader : public PortableReaderBase {
                public:
                    DefaultPortableReader(PortableSerializer &portableSer,
                                          ObjectDataInput &input, std::shared_ptr<ClassDefinition> cd);

                    template<typename T>
                    boost::optional<T> readPortable(const char *fieldName);

                    template<typename T>
                    std::vector<boost::optional<T>> readPortableArray(const char *fieldName);
                };

                class HAZELCAST_API MorphingPortableReader : public PortableReaderBase {
                public:
                    MorphingPortableReader(PortableSerializer &portableSer, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd);

                    int32_t readInt(const char *fieldName);

                    int64_t readLong(const char *fieldName);

                    bool readBoolean(const char *fieldName);

                    byte readByte(const char *fieldName);

                    char readChar(const char *fieldName);

                    double readDouble(const char *fieldName);

                    float readFloat(const char *fieldName);

                    int16_t readShort(const char *fieldName);

                    std::unique_ptr<std::string> readUTF(const char *fieldName);

                    std::unique_ptr<std::vector<byte> > readByteArray(const char *fieldName);

                    std::unique_ptr<std::vector<char> > readCharArray(const char *fieldName);

                    std::unique_ptr<std::vector<int32_t> > readIntArray(const char *fieldName);

                    std::unique_ptr<std::vector<int64_t> > readLongArray(const char *fieldName);

                    std::unique_ptr<std::vector<double> > readDoubleArray(const char *fieldName);

                    std::unique_ptr<std::vector<float> > readFloatArray(const char *fieldName);

                    std::unique_ptr<std::vector<int16_t> > readShortArray(const char *fieldName);

                    template<typename T>
                    std::shared_ptr<T> readPortable(const char *fieldName);

                    template<typename T>
                    std::vector<boost::optional<T>> readPortableArray(const char *fieldName);
                };

                class HAZELCAST_API PortableSerializer {
                public:
                    PortableSerializer(PortableContext &portableContext);

                    template<typename T>
                    boost::optional<T> readObject(ObjectDataInput &in);

                    template<typename T>
                    boost::optional<T> read(ObjectDataInput &in, int32_t factoryId, int32_t classId);

                    template<typename T>
                    void write(const T &object, ObjectDataOutput &out);

                    template<typename T>
                    void write(const T &object, std::shared_ptr<ClassDefinition> &cd, ObjectDataOutput &out);

                    template<typename T>
                    std::shared_ptr<ClassDefinition> lookupOrRegisterClassDefinition();
                private:
                    PortableContext &context;

                    template<typename T>
                    int findPortableVersion(int factoryId, int classId) const;

                    PortableReader createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                int portableVersion) const;

                    int32_t readInt(ObjectDataInput &in) const;
                };

                class HAZELCAST_API DataSerializer {
                public:
                    DataSerializer(const SerializationConfig &serializationConfig);

                    ~DataSerializer();

                    virtual int32_t getHazelcastTypeId() const;

                    template<typename T>
                    static boost::optional<T> readObject(ObjectDataInput &in) {
                        bool identified = in.readBoolean();
                        if (!identified) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                    "ObjectDataInput::readObject<identified_data_serializer>",
                                    "Received data is not identified data serialized."));
                        }

                        int32_t expectedFactoryId = hz_serializer<T>::getFactoryId();
                        int32_t expectedClassId = hz_serializer<T>::getClassId();
                        int32_t factoryId = in.readInt();
                        int32_t classId = in.readInt();
                        if (expectedFactoryId != factoryId || expectedClassId != classId) {
                            BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                          "ObjectDataInput::readObject<identified_data_serializer>",
                                                                  (boost::format(
                                                                          "Factory id %1% and class id %2% of data do not match expected "
                                                                          "factory id %3% and class id %4%!") %
                                                                   factoryId % classId % expectedFactoryId %
                                                                   expectedClassId).str())
                            );
                        }

                        return boost::make_optional(hz_serializer<T>::readData(in));
                    }

                    template<typename T>
                    static void write(const T &object, ObjectDataOutput &out);

                private:
                    const SerializationConfig &serializationConfig;

                    int32_t readInt(ObjectDataInput &in) const;
                };

                class HAZELCAST_API DefaultPortableWriter {
                public:
                    DefaultPortableWriter(PortableSerializer &portableSer, std::shared_ptr<ClassDefinition> cd,
                                          ObjectDataOutput &output);

                    void writeInt(const char *fieldName, int32_t value);

                    void writeLong(const char *fieldName, int64_t value);

                    void writeBoolean(const char *fieldName, bool value);

                    void writeByte(const char *fieldName, byte value);

                    void writeChar(const char *fieldName, int32_t value);

                    void writeDouble(const char *fieldName, double value);

                    void writeFloat(const char *fieldName, float value);

                    void writeShort(const char *fieldName, int32_t value);

                    void writeUTF(const char *fieldName, const std::string *str);

                    void writeByteArray(const char *fieldName, const std::vector<byte> *x);

                    void writeBooleanArray(const char *fieldName, const std::vector<bool> *x);

                    void writeCharArray(const char *fieldName, const std::vector<char> *data);

                    void writeShortArray(const char *fieldName, const std::vector<int16_t> *data);

                    void writeIntArray(const char *fieldName, const std::vector<int32_t> *data);

                    void writeLongArray(const char *fieldName, const std::vector<int64_t> *data);

                    void writeFloatArray(const char *fieldName, const std::vector<float> *data);

                    void writeDoubleArray(const char *fieldName, const std::vector<double> *data);

                    void end();

                    template<typename T>
                    void writeNullPortable(const char *fieldName);

                    template<typename T>
                    void writePortable(const char *fieldName, const T *portable);

                    template<typename T>
                    void writePortableArray(const char *fieldName, const std::vector<T> *values);

                    ObjectDataOutput &getRawDataOutput();
                private:
                    FieldDefinition const &setPosition(const char *fieldName, FieldType fieldType);

                    void write(const Portable &p);

                    void checkPortableAttributes(const FieldDefinition &fd, const Portable &portable);

                    bool raw;
                    PortableSerializer &portableSerializer;
                    DataOutput &dataOutput;
                    ObjectDataOutput &objectDataOutput;
                    size_t begin;
                    size_t offset;
                    std::set<std::string> writtenFields;
                    std::shared_ptr<ClassDefinition> cd;
                };

                /**
                 * This class represents the type of a Hazelcast serializable object. The fields can take the following
                 * values:
                 * 1. Primitive types: factoryId=-1, classId=-1, typeId is the type id for that primitive as listed in
                 * @link SerializationConstants
                 * 2. Array of primitives: factoryId=-1, classId=-1, typeId is the type id for that array as listed in
                 * @link SerializationConstants
                 * 3. IdentifiedDataSerializable: factory, class and type ids are non-negative values as registered by
                 * the DataSerializableFactory.
                 * 4. Portable: factory, class and type ids are non-negative values as registered by the PortableFactory.
                 * 5. Custom serialized objects: factoryId=-1, classId=-1, typeId is the non-negative type id as
                 * registered for the custom object.
                 *
                 */
                struct HAZELCAST_API ObjectType {
                    ObjectType();

                    int32_t typeId;
                    int32_t factoryId;
                    int32_t classId;

                };

                std::ostream HAZELCAST_API &operator<<(std::ostream &os, const ObjectType &type);

                class HAZELCAST_API SerializationService : public util::Disposable {
                public:
                    SerializationService(const SerializationConfig &serializationConfig);

                    template<typename T>
                    inline Data toData(const T *object) {
                        DataOutput output;

                        ObjectDataOutput dataOutput(output, portableSerializer, dataSerializer);

                        writeHash<T>(object, output);

                        dataOutput.writeObject<T>(object);

                        Data data(output.toByteArray());
                        return data;
                    }

                    template<typename T>
                    inline std::shared_ptr<Data> toSharedData(const T *object) {
                        if (NULL == object) {
                            return std::shared_ptr<Data>();
                        }
                        return std::shared_ptr<Data>(new Data(toData<T>(object)));
                    }

                    template<typename T>
                    inline boost::optional<T> toObject(const Data *data) {
                        if (NULL == data) {
                            return boost::optional<T>();
                        }
                        return toObject<T>(*data);
                    }

                    template<typename T>
                    inline boost::optional<T> toObject(const Data &data) {
                        if (isNullData(data)) {
                            return boost::optional<T>();
                        }

                        int32_t typeId = data.getType();

                        // Constant 8 is Data::DATA_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        DataInput dataInput(data.toByteArray(), 8);

                        ObjectDataInput objectDataInput(dataInput, portableSerializer, dataSerializer);
                        return objectDataInput.readObject<T>(typeId);
                    }

                    template<typename T>
                    inline const std::shared_ptr<T> toSharedObject(const std::shared_ptr<Data> &data) {
                        return std::make_shared<T>(new T(std::move(toObject<T>(data.get()).value())));
                    }

                    template<typename T>
                    inline const std::shared_ptr<T> toSharedObject(const std::unique_ptr<Data> &data) {
                        return std::make_shared<T>(new T(std::move(toObject<T>(data.get()).value())));
                    }

                    const byte getVersion() const;

                    ObjectType getObjectType(const Data *data);

                    /**
                     * @link Disposable interface implementation
                     */
                    void dispose();

                private:

                    SerializationService(const SerializationService &);

                    SerializationService &operator=(const SerializationService &);

                    PortableContext portableContext;
                    const SerializationConfig &serializationConfig;
                    serialization::pimpl::PortableSerializer portableSerializer;
                    serialization::pimpl::DataSerializer dataSerializer;

                    bool isNullData(const Data &data);

                    template<typename T>
                    void writeHash(const PartitionAwareMarker *obj, DataOutput &out) {
                        typedef typename T::KEY_TYPE PK_TYPE;
                        const PartitionAware<PK_TYPE> *partitionAwareObj = static_cast<const PartitionAware<PK_TYPE> *>(obj);
                        const PK_TYPE *pk = partitionAwareObj->getPartitionKey();
                        if (pk != NULL) {
                            Data partitionKey = toData<PK_TYPE>(pk);
                            out.writeInt(partitionKey.getPartitionHash());
                        }
                    }

                    template<typename T>
                    void writeHash(const void *obj, DataOutput &out) {
                        out.writeInt(0);
                    }
                };
            }

            template<typename T>
            std::shared_ptr<T> PortableReader::readPortable(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readPortable<T>(fieldName);
                return morphingPortableReader->readPortable<T>(fieldName);
            };

            /**
            * @tparam type of the portable class in array
            * @param fieldName name of the field
            * @return the portable array value read
            * @throws IOException
            */
            template<typename T>
            std::vector<boost::optional<T>> PortableReader::readPortableArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readPortableArray<T>(fieldName);
                return morphingPortableReader->readPortableArray<T>(fieldName);
            };

            template<typename T>
            void PortableWriter::writeNullPortable(const char *fieldName) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writeNullPortable<T>(fieldName);
                return classDefinitionWriter->writeNullPortable<T>(fieldName);
            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param portable  Portable to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::writePortable(const char *fieldName, const T *portable) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writePortable(fieldName, portable);
                return classDefinitionWriter->writePortable(fieldName, portable);

            }

            /**
            * @tparam type of the portable class
            * @param fieldName name of the field
            * @param values portable array to be written
            * @throws IOException
            */
            template<typename T>
            void PortableWriter::writePortableArray(const char *fieldName, const std::vector<T> *values) {
                if (isDefaultWriter)
                    return defaultPortableWriter->writePortableArray(fieldName, values);
                return classDefinitionWriter->writePortableArray(fieldName, values);
            }

            template<typename T>
            void ObjectDataOutput::writeObject(const T *object) {
                if (!object) {
                    writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_NULL);
                    return;
                }

                writeObject<T>(*object);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_DATA);
                pimpl::DataSerializer::write<T>(object, *this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, void>::type
            inline ObjectDataOutput::writeObject(const T &object) {
                writeInt(pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE);
                portableSerializer.write<T>(object, *this);
            }

            template<typename T>
            inline void ObjectDataOutput::writeObject(const T &object) {
                writeInt(hz_serializer<T>::getTypeId());
                hz_serializer<T>::write(object, *this);
            }

            template<typename T>
            inline boost::optional<T> ObjectDataInput::readObject() {
                int32_t typeId = readInt();
                return readObject<T>(typeId);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<identified_data_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != pimpl::SerializationConstants::CONSTANT_TYPE_DATA) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                            "ObjectDataInput::readObject<identified_data_serializer>",
                            (boost::format("The associated serializer Serializer<T> is identified_data_serializer "
                                           "but received data type id is %1%") % typeId).str()));
                }

                return dataSerializer.readObject<T>(*this);
            }

            template<typename T>
            typename std::enable_if<std::is_base_of<portable_serializer, hz_serializer<T>>::value, boost::optional<T>>::type
            inline ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                            "ObjectDataInput::readObject<portable_serializer>",
                            (boost::format("The associated serializer Serializer<T> is portable_serializer "
                                           "but received data type id is %1%") % typeId).str()));
                }

                return portableSerializer.readObject<T>(*this);
            }

            template<typename T>
            inline boost::optional<T> ObjectDataInput::readObject(int32_t typeId) {
                if (typeId != hz_serializer<T>::getTypeId()) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ObjectDataInput::readObject<>",
                                                                                     (boost::format(
                                                                                             "The associated serializer Serializer<T> type id %1% does not match "
                                                                                             "received data type id is %2%") %
                                                                                      hz_serializer<T>::getTypeId() %
                                                                                      typeId).str()));
                }

                return boost::optional<T>(hz_serializer<T>::read(*this));
            }

            namespace pimpl {
                template<>
                Data SerializationService::toData(const TypedData *object);

                template<typename T>
                boost::optional<T> DefaultPortableReader::readPortable(const char *fieldName) {
                    return getPortableInstance<T>(fieldName);
                }

                template<typename T>
                std::vector<boost::optional<T>> DefaultPortableReader::readPortableArray(const char *fieldName) {
                    PortableReaderBase::setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    dataInput.readInt();
                    std::vector<boost::optional<T>> portables;

                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    int32_t len = dataInput.readInt();
                    int32_t factoryId = dataInput.readInt();
                    int32_t classId = dataInput.readInt();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (len > 0) {
                        int offset = dataInput.position();
                        for (int i = 0; i < len; i++) {
                            dataInput.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = dataInput.readInt();
                            dataInput.position(start);

                            portables.push_back(portableSerializer.read<T>(dataInput, factoryId, classId));
                        }
                    }
                    return portables;
                }

                template<typename T>
                std::shared_ptr<T> MorphingPortableReader::readPortable(const char *fieldName) {
                    return getPortableInstance<T>(fieldName);
                }

                template<typename T>
                std::vector<boost::optional<T>> MorphingPortableReader::readPortableArray(const char *fieldName) {
                    PortableReaderBase::setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    dataInput.readInt();
                    std::vector<boost::optional<T>> portables;

                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    int32_t len = dataInput.readInt();
                    int32_t factoryId = dataInput.readInt();
                    int32_t classId = dataInput.readInt();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (len > 0) {
                        int offset = dataInput.position();
                        for (int i = 0; i < len; i++) {
                            dataInput.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                            int32_t start = dataInput.readInt();
                            dataInput.position(start);

                            portables.push_back(portableSerializer.read<T>(dataInput, factoryId, classId));
                        }
                    }

                    return portables;
                }

                template<typename T>
                boost::optional<T> PortableSerializer::readObject(ObjectDataInput &in) {
                    int32_t factoryId = readInt(in);
                    int32_t classId = readInt(in);

                    return read<T>(in, factoryId, classId);
                }

                template<typename T>
                boost::optional<T> PortableSerializer::read(ObjectDataInput &in, int32_t factoryId, int32_t classId) {
                    int version = in.readInt();

                    int portableVersion = findPortableVersion<T>(factoryId, classId);

                    PortableReader reader = createReader(in, factoryId, classId, version, portableVersion);
                    boost::optional<T> result = hz_serializer<T>::readPortable(reader);
                    reader.end();
                    return result;
                }

                template<typename T>
                void PortableSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.writeInt(hz_serializer<T>::getFactoryId());
                    out.writeInt(hz_serializer<T>::getClassId());

                    std::shared_ptr<ClassDefinition> cd = context.lookupOrRegisterClassDefinition<T>();
                    write<T>(object, cd, out);
                }

                template<typename T>
                void PortableSerializer::write(const T &object, std::shared_ptr<ClassDefinition> &cd, ObjectDataOutput &out) {
                    out.writeInt(cd->getVersion());

                    DefaultPortableWriter dpw(*this, cd, out);
                    PortableWriter portableWriter(&dpw);
                    hz_serializer<T>::writePortable(portableWriter);
                    portableWriter.end();
                }

                template<typename T>
                std::shared_ptr<ClassDefinition> PortableSerializer::lookupOrRegisterClassDefinition() {
                    return context.lookupOrRegisterClassDefinition<T>();
                }

                template<typename T>
                int PortableSerializer::findPortableVersion(int factoryId, int classId) const {
                    int currentVersion = context.getClassVersion(factoryId, classId);
                    if (currentVersion < 0) {
                        currentVersion = PortableVersionHelper::getVersion<T>(context.getVersion());
                        if (currentVersion > 0) {
                            context.setClassVersion(factoryId, classId, currentVersion);
                        }
                    }
                    return currentVersion;
                }

                template<typename T>
                void DataSerializer::write(const T &object, ObjectDataOutput &out) {
                    out.writeBoolean(true);
                    out.writeInt(hz_serializer<T>::getFactoryId());
                    out.writeInt(hz_serializer<T>::getClassId());
                    hz_serializer<T>::writeData(object, out);
                }

                template<typename T>
                std::shared_ptr<ClassDefinition>
                PortableContext::lookupOrRegisterClassDefinition() {
                    int portableVersion = PortableVersionHelper::getVersion<T>(
                            serializationConfig.getPortableVersion());
                    std::shared_ptr<ClassDefinition> cd = lookupClassDefinition(hz_serializer<T>::getFactoryId(),
                                                                                hz_serializer<T>::getClassId(),
                                                                                portableVersion);
                    if (cd.get() == NULL) {
                        ClassDefinitionBuilder classDefinitionBuilder(hz_serializer<T>::getFactoryId(),
                                                                      hz_serializer<T>::getClassId(), portableVersion);
                        ClassDefinitionWriter cdw(*this, classDefinitionBuilder);
                        PortableWriter portableWriter(&cdw);
                        hz_serializer<T>::writePortable(portableWriter);
                        cd = cdw.registerAndGet();
                    }
                    return cd;
                }

                template<typename T>
                boost::optional<T> PortableReaderBase::getPortableInstance(char const *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE);

                    bool isNull = dataInput.readBoolean();
                    int32_t factoryId = dataInput.readInt();
                    int32_t classId = dataInput.readInt();

                    checkFactoryAndClass(cd->getField(fieldName), factoryId, classId);

                    if (isNull) {
                        return boost::optional<T>();
                    } else {
                        portableSerializer.read<T>(dataInput, factoryId, classId);
                    }
                }

                template<typename T>
                void DefaultPortableWriter::writeNullPortable(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_PORTABLE);
                    dataOutput.writeBoolean(true);

                    dataOutput.writeInt(hz_serializer<T>::getFactoryId());
                    dataOutput.writeInt(hz_serializer<T>::getClassId());
                }

                template<typename T>
                void DefaultPortableWriter::writePortable(const char *fieldName, const T *portable) {
                    bool isNull = (NULL == portable);
                    dataOutput.writeBoolean(isNull);
                    if (isNull) {
                        dataOutput.writeInt(hz_serializer<T>::getFactoryId());
                        dataOutput.writeInt(hz_serializer<T>::getClassId());
                        return;
                    }

                    FieldDefinition const &fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE);
                    if (fd.getFactoryId() != hz_serializer<T>::getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << hz_serializer<T>::getFactoryId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }
                    if (fd.getClassId() != hz_serializer<T>::getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << hz_serializer<T>::getClassId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }

                    portableSerializer.write(*portable, dataOutput);
                }

                template<typename T>
                void DefaultPortableWriter::writePortableArray(const char *fieldName, const std::vector<T> *values) {
                    FieldDefinition const &fd = setPosition(fieldName, FieldTypes::TYPE_PORTABLE_ARRAY);

                    if (fd.getFactoryId() != hz_serializer<T>::getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << hz_serializer<T>::getFactoryId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }
                    if (fd.getClassId() != hz_serializer<T>::getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << hz_serializer<T>::getClassId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }

                    int32_t len = (NULL == values ? util::Bits::NULL_ARRAY : (int32_t) values->size());
                    dataOutput.writeInt(len);

                    dataOutput.writeInt(fd.getFactoryId());
                    dataOutput.writeInt(fd.getClassId());
                    std::shared_ptr<ClassDefinition> classDefinition = portableSerializer.lookupOrRegisterClassDefinition<T>();
                    if (len > 0) {
                        size_t currentOffset = dataOutput.position();
                        dataOutput.position(currentOffset + len * util::Bits::INT_SIZE_IN_BYTES);
                        for (int32_t i = 0; i < len; i++) {
                            size_t position = dataOutput.position();
                            dataOutput.writeInt((int32_t) (currentOffset + i * util::Bits::INT_SIZE_IN_BYTES),
                                                (int32_t) position);
                            portableSerializer.write((*values)[i], classDefinition, dataOutput);
                        }
                    }
                }
            }

            int32_t hz_serializer<std::string>::getTypeId() {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
            }
            void hz_serializer<std::string>::write(const std::string &object, ObjectDataOutput &out) {
                out.writeUTF(&object);
            }
            std::string hz_serializer<std::string>::read(ObjectDataInput &in) {
                return *in.readUTF();
            }

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

