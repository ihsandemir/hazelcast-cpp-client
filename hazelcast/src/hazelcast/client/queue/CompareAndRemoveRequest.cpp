//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/queue/CompareAndRemoveRequest.h"
#include "QueuePortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            CompareAndRemoveRequest::CompareAndRemoveRequest(const std::string& name, std::vector<serialization::Data>& dataList, bool retain)
            :name(name)
            , dataList(dataList)
            , retain(retain) {

            };

            int CompareAndRemoveRequest::getFactoryId() const {
                return queue::QueuePortableHook::F_ID;
            }

            int CompareAndRemoveRequest::getClassId() const {
                return queue::QueuePortableHook::COMPARE_AND_REMOVE;
            }

            void CompareAndRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeLong("t", 0);
                writer.writeBoolean("r", retain);
                writer.writeInt("s", dataList.size());
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                for (int i = 0; i < dataList.size(); ++i) {
                    dataList[i].writeData(out);
                }
            };

            void CompareAndRemoveRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                retain = reader.readBoolean("r");
                int size = reader.readInt("s");
                dataList.resize(size);
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                for (int i = 0; i < size; ++i) {
                    dataList[i].readData(in);
                }
            };
        }
    }
}