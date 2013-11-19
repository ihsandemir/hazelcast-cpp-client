//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "MultiMapIsLockedRequest.h"
#include "MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace multimap {
            MultiMapIsLockedRequest::MultiMapIsLockedRequest(const std::string& name, const serialization::Data& key)
            :KeyBasedRequest(name, key) {

            };

            int MultiMapIsLockedRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            };

            int MultiMapIsLockedRequest::getClassId() const {
                return MultiMapPortableHook::IS_LOCKED;
            };


            void MultiMapIsLockedRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeInt("tid", -1);
                KeyBasedRequest::writePortable(writer);
            };
        };

    }
}