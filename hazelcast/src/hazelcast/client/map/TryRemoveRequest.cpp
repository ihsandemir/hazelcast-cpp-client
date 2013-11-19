//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TryRemoveRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {
            TryRemoveRequest::TryRemoveRequest(const std::string& name, serialization::Data& key, int threadId, long timeout)
            :name(name)
            , key(key)
            , threadId(threadId)
            , timeout(timeout) {

            };

            int TryRemoveRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int TryRemoveRequest::getClassId() const {
                return PortableHook::TRY_REMOVE;
            };

            void TryRemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeLong("timeout", timeout);
                writer.writeUTF("n", name);
                writer.writeInt("t", threadId);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };
        }
    }
}