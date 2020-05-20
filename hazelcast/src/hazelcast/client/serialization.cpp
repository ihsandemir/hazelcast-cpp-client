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

#include <boost/concept_check.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/MurmurHash3.h"

namespace hazelcast {
    namespace client {
        HazelcastJsonValue::HazelcastJsonValue(const std::string &jsonString) : jsonString(jsonString) {
        }

        HazelcastJsonValue::~HazelcastJsonValue() {
        }

        const std::string &HazelcastJsonValue::toString() const {
            return jsonString;
        }

        bool HazelcastJsonValue::operator==(const HazelcastJsonValue &rhs) const {
            return jsonString == rhs.jsonString;
        }

        bool HazelcastJsonValue::operator!=(const HazelcastJsonValue &rhs) const {
            return !(rhs == *this);
        }

        std::ostream &operator<<(std::ostream &os, const HazelcastJsonValue &value) {
            os << "jsonString: " << value.jsonString;
            return os;
        }

        TypedData::TypedData() : ss(NULL) {
        }

        TypedData::TypedData(serialization::pimpl::Data d,
                             serialization::pimpl::SerializationService &serializationService) : data(d),
                                                                                                 ss(&serializationService) {
        }

        const serialization::pimpl::ObjectType TypedData::getType() const {
            return ss->getObjectType(&data);
        }

        const serialization::pimpl::Data &TypedData::getData() const {
            return data;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            auto lhsData = lhs.getData();
            auto rhsData = rhs.getData();

            return lhsData < rhsData;
        }

        namespace serialization {
            PortableWriter::PortableWriter(pimpl::DefaultPortableWriter *defaultPortableWriter)
                    : defaultPortableWriter(defaultPortableWriter), classDefinitionWriter(NULL), isDefaultWriter(true) {}

            PortableWriter::PortableWriter(pimpl::ClassDefinitionWriter *classDefinitionWriter)
                    : defaultPortableWriter(NULL), classDefinitionWriter(classDefinitionWriter),
                      isDefaultWriter(false) {}

            void PortableWriter::end() {
                if (isDefaultWriter)
                    return defaultPortableWriter->end();
                return classDefinitionWriter->end();
            }

            ObjectDataOutput &PortableWriter::getRawDataOutput() {
                if (isDefaultWriter)
                    return defaultPortableWriter->getRawDataOutput();
                return classDefinitionWriter->getRawDataOutput();
            }

            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), index(0), done(false) {}

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName,
                                                                             std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                                "Portable class id cannot be zero!"));
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldType::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableArrayField(const std::string &fieldName,
                                                                                  std::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                                "Portable class id cannot be zero!"));
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldType::TYPE_PORTABLE_ARRAY,
                                                def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder &ClassDefinitionBuilder::addField(FieldDefinition &fieldDefinition) {
                check();
                int defIndex = fieldDefinition.getIndex();
                if (index != defIndex) {
                    char buf[100];
                    util::hz_snprintf(buf, 100, "Invalid field index. Index in definition:%d, being added at index:%d",
                                      defIndex, index);
                    BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf));
                }
                index++;
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            std::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done = true;
                std::shared_ptr<ClassDefinition> cd(new ClassDefinition(factoryId, classId, version));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = fieldDefinitions.begin(); fdIt != fieldDefinitions.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done) {
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("ClassDefinitionBuilder::check",
                                                                                     "ClassDefinition is already built for " +
                                                                                     util::IOUtil::to_string(classId)));
                }
            }

            void ClassDefinitionBuilder::addField(const std::string &fieldName, FieldType const &fieldType) {
                check();
                FieldDefinition fieldDefinition(index++, fieldName, fieldType, version);
                fieldDefinitions.push_back(fieldDefinition);
            }

            int ClassDefinitionBuilder::getFactoryId() {
                return factoryId;
            }

            int ClassDefinitionBuilder::getClassId() {
                return classId;
            }

            int ClassDefinitionBuilder::getVersion() {
                return version;
            }

            FieldDefinition::FieldDefinition()
                    : index(0), classId(0), factoryId(0), version(-1) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int version)
                    : index(index), fieldName(fieldName), type(type), classId(0), factoryId(0), version(version) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int factoryId, int classId, int version)
                    : index(index), fieldName(fieldName), type(type), classId(classId), factoryId(factoryId),
                      version(version) {}

            const FieldType &FieldDefinition::getType() const {
                return type;
            }

            std::string FieldDefinition::getName() const {
                return fieldName;
            }

            int FieldDefinition::getIndex() const {
                return index;
            }

            int FieldDefinition::getFactoryId() const {
                return factoryId;
            }

            int FieldDefinition::getClassId() const {
                return classId;
            }

            void FieldDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(index);
                dataOutput.write<std::string>(fieldName);
                dataOutput.write<byte>(type.getId());
                dataOutput.write<int32_t>(factoryId);
                dataOutput.write<int32_t>(classId);
            }

            void FieldDefinition::readData(pimpl::DataInput &dataInput) {
                index = dataInput.read<int32_t>();
                fieldName = dataInput.read<std::string>();
                type.id = dataInput.read<byte>();
                factoryId = dataInput.read<int32_t>();
                classId = dataInput.read<int32_t>();
            }

            bool FieldDefinition::operator==(const FieldDefinition &rhs) const {
                return fieldName == rhs.fieldName &&
                       type == rhs.type &&
                       classId == rhs.classId &&
                       factoryId == rhs.factoryId &&
                       version == rhs.version;
            }

            bool FieldDefinition::operator!=(const FieldDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition) {
                os << "FieldDefinition{" << "index: " << definition.index << " fieldName: " << definition.fieldName
                   << " type: " << definition.type << " classId: " << definition.classId << " factoryId: "
                   << definition.factoryId << " version: " << definition.version;
                return os;
            }

            ObjectDataInput::ObjectDataInput(std::vector<byte> buffer, int offset, pimpl::DataInput &dataInput,
                    pimpl::PortableSerializer &portableSer, pimpl::DataSerializer &dataSer,
                    const std::shared_ptr<serialization::global_serializer> &globalSerializer)
                    : pimpl::DataInput(std::move(buffer), offset), portableSerializer(portableSer), dataSerializer(dataSer),
                      globalSerializer_(globalSerializer) {}

            pimpl::Data ObjectDataInput::readData() {
                return pimpl::Data(dataInput.read<byte>Array());
            }

            int ObjectDataInput::position() {
                return dataInput.position();
            }

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            }

            FieldType::FieldType() : id(0) {
            }

            FieldType::FieldType(int type) : id((byte) type) {
            }

            FieldType::FieldType(FieldType const &rhs) : id(rhs.id) {
            }

            byte FieldType::getId() const {
                return id;
            }

            FieldType &FieldType::operator=(FieldType const &rhs) {
                this->id = rhs.id;
                return (*this);
            }

            bool FieldType::operator==(FieldType const &rhs) const {
                if (id != rhs.id) return false;
                return true;
            }

            bool FieldType::operator!=(FieldType const &rhs) const {
                if (id == rhs.id) return false;
                return true;
            }

            std::ostream &operator<<(std::ostream &os, const FieldType &type) {
                os << "FieldType{id: " << type.id << "}";
                return os;
            }

            ObjectDataOutput::ObjectDataOutput(bool dontWrite, pimpl::PortableSerializer *portableSer,
                                               pimpl::DataSerializer *dataSer,
                                               std::shared_ptr<serialization::global_serializer> globalSerializer)
                    : DataOutput(dontWrite), portableSerializer(portableSer), dataSerializer(dataSer),
                      globalSerializer_(globalSerializer) {}

            PortableReader::PortableReader(pimpl::PortableSerializer &portableSer, ObjectDataInput &input,
                                           std::shared_ptr<ClassDefinition> cd, bool isDefaultReader)
                    : isDefaultReader(isDefaultReader) {
                if (isDefaultReader) {
                    defaultPortableReader.reset(new pimpl::DefaultPortableReader(portableSer, input, cd));
                } else {
                    morphingPortableReader.reset(new pimpl::MorphingPortableReader(portableSer, input, cd));
                }
            }

            PortableReader::PortableReader(const PortableReader &reader)
                    : isDefaultReader(reader.isDefaultReader),
                      defaultPortableReader(reader.defaultPortableReader.release()),
                      morphingPortableReader(reader.morphingPortableReader.release()) {}

            PortableReader &PortableReader::operator=(const PortableReader &reader) {
                this->isDefaultReader = reader.isDefaultReader;
                this->defaultPortableReader.reset(reader.defaultPortableReader.release());
                this->morphingPortableReader.reset(reader.morphingPortableReader.release());
                return *this;
            }

            int32_t PortableReader::readInt(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readInt(fieldName);
                return morphingPortableReader->readInt(fieldName);
            }

            int64_t PortableReader::readLong(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLong(fieldName);
                return morphingPortableReader->readLong(fieldName);
            }

            bool PortableReader::readBoolean(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBoolean(fieldName);
                return morphingPortableReader->readBoolean(fieldName);
            }

            byte PortableReader::readByte(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByte(fieldName);
                return morphingPortableReader->readByte(fieldName);
            }

            char PortableReader::readChar(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readChar(fieldName);
                return morphingPortableReader->readChar(fieldName);
            }

            double PortableReader::readDouble(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDouble(fieldName);
                return morphingPortableReader->readDouble(fieldName);
            }

            float PortableReader::readFloat(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloat(fieldName);
                return morphingPortableReader->readFloat(fieldName);
            }

            int16_t PortableReader::readShort(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShort(fieldName);
                return morphingPortableReader->readShort(fieldName);
            }

            std::unique_ptr<std::string> PortableReader::readUTF(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readUTF(fieldName);
                return morphingPortableReader->readUTF(fieldName);
            }

            std::unique_ptr<std::vector<byte> > PortableReader::readByteArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readByteArray(fieldName);
                return morphingPortableReader->readByteArray(fieldName);
            }


            std::unique_ptr<std::vector<bool> > PortableReader::readBooleanArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readBooleanArray(fieldName);
                return morphingPortableReader->readBooleanArray(fieldName);
            }

            std::unique_ptr<std::vector<char> > PortableReader::readCharArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readCharArray(fieldName);
                return morphingPortableReader->readCharArray(fieldName);
            }

            std::unique_ptr<std::vector<int32_t> > PortableReader::readIntArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readIntArray(fieldName);
                return morphingPortableReader->readIntArray(fieldName);
            }

            std::unique_ptr<std::vector<int64_t> > PortableReader::readLongArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readLongArray(fieldName);
                return morphingPortableReader->readLongArray(fieldName);
            }

            std::unique_ptr<std::vector<double> > PortableReader::readDoubleArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readDoubleArray(fieldName);
                return morphingPortableReader->readDoubleArray(fieldName);
            }

            std::unique_ptr<std::vector<float> > PortableReader::readFloatArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readFloatArray(fieldName);
                return morphingPortableReader->readFloatArray(fieldName);
            }

            std::unique_ptr<std::vector<int16_t> > PortableReader::readShortArray(const char *fieldName) {
                if (isDefaultReader)
                    return defaultPortableReader->readShortArray(fieldName);
                return morphingPortableReader->readShortArray(fieldName);
            }

            ObjectDataInput &PortableReader::getRawDataInput() {
                if (isDefaultReader)
                    return defaultPortableReader->getRawDataInput();
                return morphingPortableReader->getRawDataInput();
            }

            void PortableReader::end() {
                if (isDefaultReader)
                    return defaultPortableReader->end();
                return morphingPortableReader->end();

            }

            ClassDefinition::ClassDefinition()
                    : factoryId(0), classId(0), version(-1), binary(new std::vector<byte>) {
            }

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
                    : factoryId(factoryId), classId(classId), version(version), binary(new std::vector<byte>) {
            }

            void ClassDefinition::addFieldDef(FieldDefinition &fd) {
                fieldDefinitionsMap[fd.getName()] = fd;
            }

            const FieldDefinition &ClassDefinition::getField(const std::string &name) const {
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return fieldDefinitionsMap.find(name)->second;
                }
                BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("ClassDefinition::getField",
                                                                          (boost::format("Field (%1%) does not exist") %
                                                                           name).str()));
            }

            bool ClassDefinition::hasField(const std::string &fieldName) const {
                return fieldDefinitionsMap.find(fieldName) != fieldDefinitionsMap.end();
            }

            FieldType ClassDefinition::getFieldType(const std::string &fieldName) const {
                FieldDefinition const &fd = getField(fieldName);
                return fd.getType();
            }

            int ClassDefinition::getFieldCount() const {
                return (int) fieldDefinitionsMap.size();
            }


            int ClassDefinition::getFactoryId() const {
                return factoryId;
            }

            int ClassDefinition::getClassId() const {
                return classId;
            }

            int ClassDefinition::getVersion() const {
                return version;
            }

            void ClassDefinition::setVersionIfNotSet(int newVersion) {
                if (getVersion() < 0) {
                    this->version = newVersion;
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput &dataOutput) {
                dataOutput.write<int32_t>(factoryId);
                dataOutput.write<int32_t>(classId);
                dataOutput.write<int32_t>(version);
                dataOutput.writeShort(fieldDefinitionsMap.size());
                for (std::map<std::string, FieldDefinition>::iterator it = fieldDefinitionsMap.begin();
                     it != fieldDefinitionsMap.end(); ++it) {
                    it->second.writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(pimpl::DataInput &dataInput) {
                factoryId = dataInput.read<int32_t>();
                classId = dataInput.read<int32_t>();
                version = dataInput.read<int32_t>();
                int size = dataInput.read<int16_t>();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
            }

            bool ClassDefinition::operator==(const ClassDefinition &rhs) const {
                return factoryId == rhs.factoryId &&
                       classId == rhs.classId &&
                       version == rhs.version &&
                       fieldDefinitionsMap == rhs.fieldDefinitionsMap;
            }

            bool ClassDefinition::operator!=(const ClassDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition) {
                os << "ClassDefinition{" << "factoryId: " << definition.factoryId << " classId: " << definition.classId
                   << " version: "
                   << definition.version << " fieldDefinitions: {";

                for (std::map<std::string, FieldDefinition>::const_iterator it = definition.fieldDefinitionsMap.begin();
                     it != definition.fieldDefinitionsMap.end(); ++it) {
                    os << it->second;
                }
                os << "} }";
                return os;
            }

            namespace pimpl {
                ClassDefinitionWriter::ClassDefinitionWriter(PortableContext &portableContext,
                                                             ClassDefinitionBuilder &builder)
                        : builder(builder), context(portableContext), emptyDataOutput(true) {}

                std::shared_ptr<ClassDefinition> ClassDefinitionWriter::registerAndGet() {
                    std::shared_ptr<ClassDefinition> cd = builder.build();
                    return context.registerClassDefinition(cd);
                }

                ObjectDataOutput &ClassDefinitionWriter::getRawDataOutput() {
                    return emptyDataOutput;
                }

                void ClassDefinitionWriter::end() {}

                DataOutput::DataOutput(bool dontWrite) :isNoWrite(dontWrite) {
                    if (isNoWrite) {
                        outputStream.reserve(0);
                    } else {
                        outputStream.reserve(DEFAULT_SIZE);
                    }
                }

                std::vector<byte> DataOutput::toByteArray() {
                    return outputStream;
                }

                template<>
                void DataOutput::write(byte i) {
                    if (isNoWrite) { return; }
                    outputStream.push_back(i);
                }

                template<>
                void DataOutput::write(char i) {
                    if (isNoWrite) { return; }
                    write<byte>(i);
                }

                template<>
                void DataOutput::write(int16_t value) {
                    if (isNoWrite) { return; }
                    int16_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian2(&value, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::SHORT_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(int32_t v) {
                    if (isNoWrite) { return; }
                    int32_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian4(&v, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::INT_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(int64_t l) {
                    if (isNoWrite) { return; }
                    int64_t result;
                    byte *target = (byte *) &result;
                    util::Bits::nativeToBigEndian8(&l, target);
                    outputStream.insert(outputStream.end(), target, target + util::Bits::LONG_SIZE_IN_BYTES);
                }

                template<>
                void DataOutput::write(float x) {
                    if (isNoWrite) { return; }
                    union {
                        float f;
                        int32_t i;
                    } u;
                    u.f = x;
                    write<int32_t>(u.i);
                }

                template<>
                void DataOutput::write(double v) {
                    if (isNoWrite) { return; }
                    union {
                        double d;
                        int64_t l;
                    } u;
                    u.d = v;
                    write<int64_t>(u.l);
                }

                template<>
                void DataOutput::write(bool value) {
                    if (isNoWrite) { return; }
                    write<byte>(value);
                }

                template<>
                void DataOutput::write(const std::string &str) {
                    if (isNoWrite) { return; }
                    int32_t len = util::UTFUtil::isValidUTF8(str);
                    if (len < 0) {
                        throw (exception::ExceptionBuilder<exception::UTFDataFormatException>(
                                "DataOutput::writeUTF")
                                << "String \"" << (*str) << "\" is not UTF-8 formatted !!!").build();
                    }

                    write<int32_t>(len);
                    if (len > 0) {
                        outputStream.insert(outputStream.end(), str.begin(), str.end());
                    }
                }

                template<>
                void DataOutput::write(const HazelcastJsonValue &value) {
                    if (isNoWrite) { return; }
                    write<std::string>(value.toString());
                }

                size_t DataOutput::position() {
                    return outputStream.size();
                }

                void DataOutput::position(size_t newPos) {
                    if (isNoWrite) { return; }
                    if (outputStream.size() < newPos) {
                        outputStream.resize(newPos, 0);
                    }
                }

                int DataOutput::getUTF8CharCount(const std::string &str) {
                    int size = 0;
                    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
                        // Any additional byte for an UTF character has a bit mask of 10xxxxxx
                        size += (*it & 0xC0) != 0x80;
                    }

                    return size;
                }

                ObjectType::ObjectType() : typeId(SerializationConstants::CONSTANT_TYPE_NULL), factoryId(-1), classId(-1) {}

                std::ostream &operator<<(std::ostream &os, const ObjectType &type) {
                    os << "typeId: " << static_cast<int32_t>(type.typeId) << " factoryId: " << type.factoryId << " classId: "
                       << type.classId;
                    return os;
                }

                DataSerializer::DataSerializer(const SerializationConfig &serializationConfig)
                        : serializationConfig(serializationConfig) {}

                DataSerializer::~DataSerializer() {}

                int32_t DataSerializer::readInt(ObjectDataInput &in) const {
                    return in.read<int32_t>();
                }

                PortableContext::PortableContext(const SerializationConfig &serializationConf) :
                        serializationConfig(serializationConf) {}

                int PortableContext::getClassVersion(int factoryId, int classId) {
                    return getClassDefinitionContext(factoryId).getClassVersion(classId);
                }

                void PortableContext::setClassVersion(int factoryId, int classId, int version) {
                    getClassDefinitionContext(factoryId).setClassVersion(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::lookupClassDefinition(int factoryId, int classId, int version) {
                    return getClassDefinitionContext(factoryId).lookup(classId, version);
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::readClassDefinition(ObjectDataInput &in, int factoryId, int classId, int version) {
                    bool shouldRegister = true;
                    ClassDefinitionBuilder builder(factoryId, classId, version);

                    // final position after portable is read
                    in.readInt();

                    // field count
                    int fieldCount = in.readInt();
                    int offset = in.position();
                    for (int i = 0; i < fieldCount; i++) {
                        in.position(offset + i * util::Bits::INT_SIZE_IN_BYTES);
                        int pos = in.readInt();
                        in.position(pos);

                        short len = in.readShort();
                        std::vector<byte> chars(len);
                        in.readFully(chars);
                        chars.push_back('\0');

                        FieldType type(in.readByte());
                        std::string name((char *) &(chars[0]));
                        int fieldFactoryId = 0;
                        int fieldClassId = 0;
                        int fieldVersion = version;
                        if (type == FieldType::TYPE_PORTABLE) {
                            // is null
                            if (in.readBoolean()) {
                                shouldRegister = false;
                            }
                            fieldFactoryId = in.readInt();
                            fieldClassId = in.readInt();

                            // TODO: what if there's a null inner Portable field
                            if (shouldRegister) {
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            }
                        } else if (type == FieldType::TYPE_PORTABLE_ARRAY) {
                            int k = in.readInt();
                            if (k > 0) {
                                fieldFactoryId = in.readInt();
                                fieldClassId = in.readInt();

                                int p = in.readInt();
                                in.position(p);

                                // TODO: what if there's a null inner Portable field
                                fieldVersion = in.readInt();
                                readClassDefinition(in, fieldFactoryId, fieldClassId, fieldVersion);
                            } else {
                                shouldRegister = false;
                            }

                        }
                        FieldDefinition fieldDef(i, name, type, fieldFactoryId, fieldClassId, fieldVersion);
                        builder.addField(fieldDef);
                    }
                    std::shared_ptr<ClassDefinition> classDefinition = builder.build();
                    if (shouldRegister) {
                        classDefinition = registerClassDefinition(classDefinition);
                    }
                    return classDefinition;
                }

                std::shared_ptr<ClassDefinition>
                PortableContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    return getClassDefinitionContext(cd->getFactoryId()).registerClassDefinition(cd);
                }

                int PortableContext::getVersion() {
                    return serializationConfig.getPortableVersion();
                }

                ClassDefinitionContext &PortableContext::getClassDefinitionContext(int factoryId) {
                    std::shared_ptr<ClassDefinitionContext> value = classDefContextMap.get(factoryId);
                    if (value == NULL) {
                        value = std::shared_ptr<ClassDefinitionContext>(new ClassDefinitionContext(factoryId, this));
                        std::shared_ptr<ClassDefinitionContext> current = classDefContextMap.putIfAbsent(factoryId,
                                                                                                         value);
                        if (current != NULL) {
                            value = current;
                        }
                    }
                    return *value;
                }

                const SerializationConfig &PortableContext::getSerializationConfig() const {
                    return serializationConfig;
                }

                SerializationService::SerializationService(const SerializationConfig &serializationConfig)
                        : portableContext(serializationConfig), serializationConfig(serializationConfig),
                          portableSerializer(portableContext), dataSerializer(serializationConfig) {}

                DefaultPortableWriter::DefaultPortableWriter(PortableSerializer &portableSer,
                                                             std::shared_ptr<ClassDefinition> cd,
                                                             ObjectDataOutput &output)
                        : raw(false), portableSerializer(portableSer), objectDataOutput(output), 
                        begin(objectDataOutput.position()), cd(cd) {
                    // room for final offset
                    objectDataOutput.writeZeroBytes(4);

                    objectDataOutput.write<int32_t>(cd->getFieldCount());

                    offset = objectDataOutput.position();
                    // one additional for raw data
                    int fieldIndexesLength = (cd->getFieldCount() + 1) * util::Bits::INT_SIZE_IN_BYTES;
                    objectDataOutput.writeZeroBytes(fieldIndexesLength);
                }

                FieldDefinition const &DefaultPortableWriter::setPosition(const std::string &fieldName, FieldType fieldType) {
                    if (raw) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                                         "Cannot write Portable fields after getRawDataOutput() is called!"));
                    }

                    try {
                        FieldDefinition const &fd = cd->getField(fieldName);

                        if (writtenFields.count(fieldName) != 0) {
                            BOOST_THROW_EXCEPTION(
                                    exception::HazelcastSerializationException("PortableWriter::setPosition",
                                                                               "Field '" + std::string(fieldName) +
                                                                               "' has already been written!"));
                        }

                        writtenFields.insert(fieldName);
                        size_t pos = objectDataOutput.position();
                        int32_t index = fd.getIndex();
                        objectDataOutput.position(offset + index * util::Bits::INT_SIZE_IN_BYTES);
                        objectDataOutput.write<int32_t>(static_cast<int32_t>(pos));
                        objectDataOutput.position(pos);
                        objectDataOutput.write(fieldName);
                        objectDataOutput.write(static_cast<int32_t>(fieldType));

                        return fd;

                    } catch (exception::IllegalArgumentException &iae) {
                        std::stringstream error;
                        error << "HazelcastSerializationException( Invalid field name: '" << fieldName;
                        error << "' for ClassDefinition {class id: " << util::IOUtil::to_string(cd->getClassId());
                        error << ", factoryId:" + util::IOUtil::to_string(cd->getFactoryId());
                        error << ", version: " << util::IOUtil::to_string(cd->getVersion()) << "}. Error:";
                        error << iae.what();

                        BOOST_THROW_EXCEPTION(
                                exception::HazelcastSerializationException("PortableWriter::setPosition", error.str()));
                    }

                }

                ObjectDataOutput &DefaultPortableWriter::getRawDataOutput() {
                    if (!raw) {
                        size_t pos = objectDataOutput.position();
                        int32_t index = cd->getFieldCount(); // last index
                        objectDataOutput.position(offset + index * util::Bits::INT_SIZE_IN_BYTES);
                        objectDataOutput.write(static_cast<int32_t>(pos));
                        objectDataOutput.position(pos);
                    }
                    raw = true;
                    return objectDataOutput;
                }

                void DefaultPortableWriter::end() {
                    size_t pos = objectDataOutput.position();
                    objectDataOutput.position(static_cast<int32_t>(begin));
                    objectDataOutput.write(static_cast<int32_t>(pos)); // write final offset
                }

                void
                DefaultPortableWriter::checkPortableAttributes(const FieldDefinition &fd, const Portable &portable) {
                    if (fd.getFactoryId() != portable.getFactoryId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << " Expected factory-id: " << fd.getFactoryId() << ", Actual factory-id: "
                                     << portable.getFactoryId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }
                    if (fd.getClassId() != portable.getClassId()) {
                        std::stringstream errorMessage;
                        errorMessage << "Wrong Portable type! Templated portable types are not supported! "
                                     << "Expected class-id: " << fd.getClassId() << ", Actual class-id: "
                                     << portable.getClassId();
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableWriter::::checkPortableAttributes", errorMessage.str()));
                    }
                }

                bool SerializationService::isNullData(const Data &data) {
                    return data.dataSize() == 0 &&
                           data.getType() == static_cast<int32_t>(SerializationConstants::CONSTANT_TYPE_NULL);
                }

                template<>
                Data SerializationService::toData(const char *object) {
                    if (!object) {
                        return toData<std::string>(nullptr);
                    }
                    std::string str(object);
                    return toData<std::string>(&str);
                }

                const byte SerializationService::getVersion() const {
                    return 1;
                }

                ObjectType SerializationService::getObjectType(const Data *data) {
                    ObjectType type;

                    if (NULL == data) {
                        type.typeId = SerializationConstants::CONSTANT_TYPE_NULL;
                        return type;
                    }

                    type.typeId = data->getType();

                    if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId ||
                        SerializationConstants::CONSTANT_TYPE_PORTABLE == type.typeId) {
                        // Constant 4 is Data::TYPE_OFFSET. Windows DLL export does not
                        // let usage of static member.
                        const std::vector<byte> &bytes = data->toByteArray();
                        auto start = bytes.begin() + 4;
                        DataInput dataInput(std::vector<byte>(start, start + 3 * util::Bits::INT_SIZE_IN_BYTES +
                                                                     util::Bits::BOOLEAN_SIZE_IN_BYTES));

                        auto objectTypeId = SerializationConstants(dataInput.read<int32_t>());
                        assert(type.typeId == objectTypeId);
                        boost::ignore_unused_variable_warning(objectTypeId);

                        if (SerializationConstants::CONSTANT_TYPE_DATA == type.typeId) {
                            bool identified = dataInput.read<bool>();
                            if (!identified) {
                                BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                              "SerializationService::getObjectType",
                                                                      " DataSerializable is not identified"));
                            }
                        }

                        type.factoryId = dataInput.read<int32_t>();
                        type.classId = dataInput.read<int32_t>();
                    }

                    return type;
                }

                void SerializationService::dispose() {
                }

                template<>
                Data SerializationService::toData(const TypedData *object) {
                    if (!object) {
                        return Data();
                    }

                    auto data = object->getData();
                    return Data(data);
                }

                //first 4 byte is partition hash code and next last 4 byte is type id
                unsigned int Data::PARTITION_HASH_OFFSET = 0;

                unsigned int Data::TYPE_OFFSET = Data::PARTITION_HASH_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OFFSET = Data::TYPE_OFFSET + util::Bits::INT_SIZE_IN_BYTES;

                unsigned int Data::DATA_OVERHEAD = Data::DATA_OFFSET;

                Data::Data() : cachedHashValue(-1) {}
                
                Data::Data(std::vector<byte> &&buffer) : data(buffer), cachedHashValue(-1) {
                    size_t size = data.size();
                    if (size > 0 && size < Data::DATA_OVERHEAD) {
                        throw (exception::ExceptionBuilder<exception::IllegalArgumentException>("Data::setBuffer")
                                << "Provided buffer should be either empty or should contain more than "
                                << Data::DATA_OVERHEAD << " bytes! Provided buffer size:" << size).build();
                    }

                    cachedHashValue = calculateHash();
                }

                size_t Data::dataSize() const {
                    return (size_t) std::max<int>((int) totalSize() - (int) Data::DATA_OVERHEAD, 0);
                }

                size_t Data::totalSize() const {
                    return data.size();
                }

                int Data::getPartitionHash() const {
                    return cachedHashValue;
                }

                bool Data::hasPartitionHash() const {
                    return data.size() >= Data::DATA_OVERHEAD &&
                           *reinterpret_cast<const int32_t *>(&data[PARTITION_HASH_OFFSET]) != 0;
                }

                const std::vector<byte> &Data::toByteArray() const {
                    return data;
                }

                SerializationConstants Data::getType() const {
                    if (totalSize() == 0) {
                        return SerializationConstants::CONSTANT_TYPE_NULL;
                    }
                    return SerializationConstants(util::Bits::readIntB(data, Data::TYPE_OFFSET));
                }

                int Data::hash() const {
                    return cachedHashValue;
                }

                int Data::calculateHash() const {
                    size_t size = dataSize();
                    if (size == 0) {
                        return 0;
                    }

                    if (hasPartitionHash()) {
                        return util::Bits::readIntB(data, Data::PARTITION_HASH_OFFSET);
                    }

                    return util::MurmurHash3_x86_32((void *) &((data)[Data::DATA_OFFSET]), (int) size);
                }

                bool Data::operator<(const Data &rhs) const {
                    return cachedHashValue < rhs.cachedHashValue;
                }

                DataInput::DataInput(std::vector<byte> &&buf) : buffer(buf), pos(0) {}

                DataInput::DataInput(std::vector<byte> &&buf, int offset) : buffer(buf), pos(offset) {}

                int DataInput::position() {
                    return pos;
                }

                void DataInput::position(int position) {
                    if (position > pos) {
                        checkAvailable((size_t) (position - pos));
                    }
                    pos = position;
                }

                ClassDefinitionContext::ClassDefinitionContext(int factoryId, PortableContext *portableContext)
                        : factoryId(factoryId), portableContext(portableContext) {
                }

                int ClassDefinitionContext::getClassVersion(int classId) {
                    std::shared_ptr<int> version = currentClassVersions.get(classId);
                    return version != NULL ? *version : -1;
                }

                void ClassDefinitionContext::setClassVersion(int classId, int version) {
                    std::shared_ptr<int> current = currentClassVersions.putIfAbsent(classId, std::shared_ptr<int>(
                            new int(version)));
                    if (current != NULL && *current != version) {
                        std::stringstream error;
                        error << "Class-id: " << classId << " is already registered!";
                        BOOST_THROW_EXCEPTION(
                                exception::IllegalArgumentException("ClassDefinitionContext::setClassVersion",
                                                                    error.str()));
                    }
                }

                std::shared_ptr<ClassDefinition> ClassDefinitionContext::lookup(int classId, int version) {
                    long long key = combineToLong(classId, version);
                    return versionedDefinitions.get(key);

                }

                std::shared_ptr<ClassDefinition>
                ClassDefinitionContext::registerClassDefinition(std::shared_ptr<ClassDefinition> cd) {
                    if (cd.get() == NULL) {
                        return std::shared_ptr<ClassDefinition>();
                    }
                    if (cd->getFactoryId() != factoryId) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition") << "Invalid factory-id! "
                                                                                   << factoryId << " -> "
                                                                                   << cd).build();
                    }

                    cd->setVersionIfNotSet(portableContext->getVersion());

                    long long versionedClassId = combineToLong(cd->getClassId(), cd->getVersion());
                    std::shared_ptr<ClassDefinition> currentCd = versionedDefinitions.putIfAbsent(versionedClassId, cd);
                    if (currentCd.get() == NULL) {
                        return cd;
                    }

                    if (currentCd.get() != cd.get() && *currentCd != *cd) {
                        throw (exception::ExceptionBuilder<exception::HazelcastSerializationException>(
                                "ClassDefinitionContext::registerClassDefinition")
                                << "Incompatible class-definitions with same class-id: " << *cd << " VS "
                                << *currentCd).build();
                    }

                    return currentCd;
                }

                int64_t ClassDefinitionContext::combineToLong(int x, int y) const {
                    return ((int64_t) x) << 32 | (((int64_t) y) & 0xFFFFFFFL);
                }

                DefaultPortableReader::DefaultPortableReader(PortableSerializer &portableSer,
                                                             ObjectDataInput &input,
                                                             std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableSer, input, cd) {}

                PortableReaderBase::PortableReaderBase(PortableSerializer &portableSer, ObjectDataInput &input,
                                                       std::shared_ptr<ClassDefinition> cd)
                        : cd(cd), dataInput(input), portableSerializer(portableSer), raw(false) {
                    int fieldCount;
                    try {
                        // final position after portable is read
                        finalPosition = input.read<int32_t>();
                        // field count
                        fieldCount = input.read<int32_t>();
                    } catch (exception::IException &e) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "[PortableReaderBase::PortableReaderBase]", e.what()));
                    }
                    if (fieldCount != cd->getFieldCount()) {
                        char msg[50];
                        util::hz_snprintf(msg, 50, "Field count[%d] in stream does not match %d", fieldCount,
                                          cd->getFieldCount());
                        BOOST_THROW_EXCEPTION(
                                exception::IllegalStateException("[PortableReaderBase::PortableReaderBase]",
                                                                 msg));
                    }
                    this->offset = input.position();
                }
                
                void PortableReaderBase::setPosition(const std::string &fieldName, FieldType const &fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int PortableReaderBase::readPosition(const std::string &fieldName, FieldType const &fieldType) {
                    if (raw) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Cannot read Portable fields after getRawDataInput() is called!"));
                    }
                    if (!cd->hasField(fieldName)) {
                        // TODO: if no field def found, java client reads nested position:
                        // readNestedPosition(fieldName, type);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Don't have a field named " +
                                                                                         std::string(fieldName)));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("PortableReader::getPosition ",
                                                                                         "Field type did not matched for " +
                                                                                         std::string(fieldName)));
                    }

                    dataInput.position(offset + cd->getField(fieldName).getIndex() * util::Bits::INT_SIZE_IN_BYTES);
                    int32_t pos = dataInput.read<int32_t>();

                    dataInput.position(pos);
                    int16_t len = dataInput.read<int16_t>();

                    // name + len + type
                    return pos + util::Bits::SHORT_SIZE_IN_BYTES + len + 1;
                }

                hazelcast::client::serialization::ObjectDataInput &PortableReaderBase::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * util::Bits::INT_SIZE_IN_BYTES);
                        int32_t pos = dataInput.read<int32_t>();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return dataInput;
                }

                void PortableReaderBase::end() {
                    dataInput.position(finalPosition);
                }

                void
                PortableReaderBase::checkFactoryAndClass(FieldDefinition fd, int32_t factoryId, int32_t classId) const {
                    if (factoryId != fd.getFactoryId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid factoryId! Expected: %d, Current: %d", fd.getFactoryId(),
                                          factoryId);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableReader::checkFactoryAndClass ",
                                                              std::string(msg)));
                    }
                    if (classId != fd.getClassId()) {
                        char msg[100];
                        util::hz_snprintf(msg, 100, "Invalid classId! Expected: %d, Current: %d", fd.getClassId(),
                                          classId);
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException(
                                                      "DefaultPortableReader::checkFactoryAndClass ",
                                                              std::string(msg)));
                    }
                }

                int32_t ByteSerializer::getHazelcastTypeId() const {

                }

                void ByteSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByte(*(static_cast<const byte *>(object)));
                }

                void *ByteSerializer::read(ObjectDataInput &in) {
                    return new byte(in.readByte());
                }

                template<>
                SerializationConstants hz_serializer<byte>::getTypeId() {
                    return SerializationConstants::CONSTANT_TYPE_BYTE;
                }

                template<>
                void hz_serializer<byte>::write(byte object, ObjectDataOutput &out) {
                    out.writeByte(object);
                }

                template<>
                byte hz_serializer<byte>::read(ObjectDataInput &in) {
                    return in.read
                }

                template<>
                struct HAZELCAST_API hz_serializer<bool> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId();
                    
                    template<>
                    void hz_serializer<bool>::write(bool object, ObjectDataOutput &out);

                    template<>
                    bool hz_serializer<bool>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<char> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                    }

                    template<>
                    void hz_serializer<char>::write(char object, ObjectDataOutput &out);

                    template<>
                    char hz_serializer<char>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<int16_t> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                    }
                    template<>
                    void hz_serializer<int16_t>::write(int16_t object, ObjectDataOutput &out);

                    int16_t hz_serializer<t>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<long> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    template<>
                    void hz_serializer<long>::write(long object, ObjectDataOutput &out);

                    template<>
                    long hz_serializer<long>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<int32_t> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    template<>
                    void hz_serializer<int32_t>::write(int32_t object, ObjectDataOutput &out);

                    int32_t hz_serializer<t>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<int64_t> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    template<>
                    void hz_serializer<int64_t>::write(int64_t object, ObjectDataOutput &out);

                    int64_t hz_serializer<t>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<float> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    template<>
                    void hz_serializer<float>::write(float object, ObjectDataOutput &out);

                    template<>
                    float hz_serializer<float>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<double> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    template<>
                    void hz_serializer<double>::write(double object, ObjectDataOutput &out);

                    template<>
                    double hz_serializer<double>::read(ObjectDataInput &in);
                };

                template<>
                struct HAZELCAST_API hz_serializer<std::string> : public builtin_serializer {
                public:
                    template<>
                    SerializationConstants hz_serializer<byte>::getTypeId() {
                         
                    }
                    
                    void write(const std::string &object, ObjectDataOutput &out);

                    std::string hz_serializer<string>::read(ObjectDataInput &in);
                };
                
                
                int32_t IntegerSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER;
                }

                void IntegerSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeInt(*(static_cast<const int32_t *>(object)));
                }

                void *IntegerSerializer::read(ObjectDataInput &in) {
                    return new int32_t(in.readInt());
                }


                int32_t BooleanSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
                }

                void BooleanSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBoolean(*(static_cast<const bool *>(object)));
                }

                void *BooleanSerializer::read(ObjectDataInput &in) {
                    return new bool(in.readBoolean());
                }

                int32_t CharSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR;
                }

                void CharSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeChar(*(static_cast<const char *>(object)));
                }

                void *CharSerializer::read(ObjectDataInput &in) {
                    return new char(in.readChar());
                }

                int32_t ShortSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT;
                }

                void ShortSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShort(*(static_cast<const int16_t *>(object)));
                }

                void *ShortSerializer::read(ObjectDataInput &in) {
                    return new int16_t(in.readShort());
                }

                int32_t LongSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG;
                }

                void LongSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLong(*(static_cast<const int64_t *>(object)));
                }

                void *LongSerializer::read(ObjectDataInput &in) {
                    return new int64_t(in.readLong());;
                }

                int32_t FloatSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT;
                }

                void FloatSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloat(*(static_cast<const float *>(object)));
                }

                void *FloatSerializer::read(ObjectDataInput &in) {
                    return new float(in.readFloat());
                }


                int32_t DoubleSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE;
                }

                void DoubleSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDouble(*(static_cast<const double *>(object)));
                }

                void *DoubleSerializer::read(ObjectDataInput &in) {
                    return new double(in.readDouble());
                }

                int32_t StringSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING;
                }

                void StringSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(static_cast<const std::string *>(object));
                }

                void *StringSerializer::read(ObjectDataInput &in) {
                    return in.readUTF().release();
                }

                int32_t NullSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_NULL;
                }

                void *NullSerializer::read(ObjectDataInput &in) {
                    return NULL;
                }

                void NullSerializer::write(ObjectDataOutput &out, const void *object) {
                }

                int32_t TheByteArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
                }

                void TheByteArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeByteArray(static_cast<const std::vector<byte> *>(object));
                }

                void *TheByteArraySerializer::read(ObjectDataInput &in) {
                    return in.readByteArray().release();
                }

                int32_t BooleanArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
                }

                void BooleanArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeBooleanArray(static_cast<const std::vector<bool> *>(object));
                }

                void *BooleanArraySerializer::read(ObjectDataInput &in) {
                    return in.readBooleanArray().release();
                }

                int32_t CharArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
                }

                void CharArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeCharArray(static_cast<const std::vector<char> *>(object));
                }

                void *CharArraySerializer::read(ObjectDataInput &in) {
                    return in.readCharArray().release();
                }

                int32_t ShortArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
                }

                void ShortArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeShortArray(static_cast<const std::vector<int16_t> *>(object));
                }

                void *ShortArraySerializer::read(ObjectDataInput &in) {
                    return in.readShortArray().release();
                }

                int32_t IntegerArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
                }

                void IntegerArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeIntArray(static_cast<const std::vector<int32_t> *>(object));
                }

                void *IntegerArraySerializer::read(ObjectDataInput &in) {
                    return in.readIntArray().release();
                }

                int32_t LongArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
                }

                void LongArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeLongArray(static_cast<const std::vector<int64_t> *>(object));
                }

                void *LongArraySerializer::read(ObjectDataInput &in) {
                    return in.readLongArray().release();
                }

                int32_t FloatArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
                }

                void FloatArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeFloatArray(static_cast<const std::vector<float> *>(object));
                }

                void *FloatArraySerializer::read(ObjectDataInput &in) {
                    return in.readFloatArray().release();
                }

                int32_t DoubleArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
                }

                void DoubleArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeDoubleArray(static_cast<const std::vector<double> *>(object));
                }

                void *DoubleArraySerializer::read(ObjectDataInput &in) {
                    return in.readDoubleArray().release();
                }

                int32_t StringArraySerializer::getHazelcastTypeId() const {
                    return SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
                }

                void StringArraySerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTFArray(static_cast<const std::vector<std::string *> *>(object));
                }

                void *StringArraySerializer::read(ObjectDataInput &in) {
                    return in.readUTFPointerArray().release();
                }

                int32_t HazelcastJsonValueSerializer::getHazelcastTypeId() const {
                    return SerializationConstants::JAVASCRIPT_JSON_SERIALIZATION_TYPE;
                }

                void HazelcastJsonValueSerializer::write(ObjectDataOutput &out, const void *object) {
                    out.writeUTF(&(static_cast<const HazelcastJsonValue *>(object)->toString()));
                }

                void *HazelcastJsonValueSerializer::read(ObjectDataInput &in) {
                    return new HazelcastJsonValue(*in.readUTF());
                }

                MorphingPortableReader::MorphingPortableReader(PortableSerializer &portableSer, ObjectDataInput &input,
                                                               std::shared_ptr<ClassDefinition> cd)
                        : PortableReaderBase(portableSer, input, cd) {}

                int32_t MorphingPortableReader::readInt(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldType::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                         "IncompatibleClassChangeError"));
                    }
                }

                int64_t MorphingPortableReader::readLong(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldType::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                         "IncompatibleClassChangeError"));
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }

                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldType::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                         "IncompatibleClassChangeError"));
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0.0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldType::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_INT) {
                        return (float) PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_BYTE) {
                        return (float) PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_CHAR) {
                        return (float) PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_SHORT) {
                        return (float) PortableReaderBase::readShort(fieldName);
                    } else {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                         "IncompatibleClassChangeError"));
                    }
                }

                int16_t MorphingPortableReader::readShort(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return 0;
                    }
                    const FieldType &currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldType::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldType::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("MorphingPortableReader::*",
                                                                                         "IncompatibleClassChangeError"));
                    }
                }

                std::unique_ptr<std::string> MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::string>(new std::string(""));
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::unique_ptr<std::vector<byte> > MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<byte> >(new std::vector<byte>(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::unique_ptr<std::vector<char> > MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<char> >(new std::vector<char>(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::unique_ptr<std::vector<int32_t> > MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int32_t> >(new std::vector<int32_t>(1, 0));
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::unique_ptr<std::vector<int64_t> > MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int64_t> >(new std::vector<int64_t>(1, 0));
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::unique_ptr<std::vector<double> > MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<double> >(new std::vector<double>(1, 0));
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::unique_ptr<std::vector<float> > MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<float> >(new std::vector<float>(1, 0));
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::unique_ptr<std::vector<int16_t> > MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)) {
                        return std::unique_ptr<std::vector<int16_t> >(new std::vector<int16_t>(1, 0));
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }

                int PortableVersionHelper::getVersion(const Portable *portable, int defaultVersion) {
                    int version = defaultVersion;
                    if (const VersionedPortable *versionedPortable = dynamic_cast<const VersionedPortable *>(portable)) {
                        version = versionedPortable->getClassVersion();
                        if (version < 0) {
                            BOOST_THROW_EXCEPTION(
                                    exception::IllegalArgumentException("PortableVersionHelper:getVersion",
                                                                        "Version cannot be negative!"));
                        }
                    }
                    return version;
                }

                PortableSerializer::PortableSerializer(PortableContext &portableContext)
                        : context(portableContext) {
                }

                PortableReader
                PortableSerializer::createReader(ObjectDataInput &input, int factoryId, int classId, int version,
                                                 int portableVersion) const {

                    int effectiveVersion = version;
                    if (version < 0) {
                        effectiveVersion = context.getVersion();
                    }

                    std::shared_ptr<ClassDefinition> cd = context.lookupClassDefinition(factoryId, classId,
                                                                                        effectiveVersion);
                    if (cd == NULL) {
                        int begin = input.position();
                        cd = context.readClassDefinition(input, factoryId, classId, effectiveVersion);
                        input.position(begin);
                    }

                    if (portableVersion == effectiveVersion) {
                        PortableReader reader(*this, input, cd, true);
                        return reader;
                    } else {
                        PortableReader reader(*this, input, cd, false);
                        return reader;
                    }
                }

                int32_t PortableSerializer::readInt(ObjectDataInput &in) const {
                    return in.readInt();
                }

            }
        }
    }
}

bool std::less<std::shared_ptr<hazelcast::client::serialization::pimpl::Data>>::operator()(
        const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &lhs,
        const std::shared_ptr<hazelcast::client::serialization::pimpl::Data> &rhs) const noexcept {
    const hazelcast::client::serialization::pimpl::Data *leftPtr = lhs.get();
    const hazelcast::client::serialization::pimpl::Data *rightPtr = rhs.get();
    if (leftPtr == rightPtr) {
        return false;
    }

    if (leftPtr == NULL) {
        return true;
    }

    if (rightPtr == NULL) {
        return false;
    }

    return lhs->hash() < rhs->hash();
}


