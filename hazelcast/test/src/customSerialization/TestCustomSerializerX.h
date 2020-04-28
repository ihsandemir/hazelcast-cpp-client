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
//
// Created by sancar koyunlu on 7/1/13.




#ifndef HAZELCAST_XmlSerializer
#define HAZELCAST_XmlSerializer

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h>

namespace hazelcast {
    namespace client {
        namespace test {
            template<typename T>
            class TestCustomSerializerX : public serialization::hz_serializer<T> {
            public:

                void write(serialization::ObjectDataOutput & out, const T& object) {
                    out.writeInt(666);
                    out.writeInt(object.id);
                    out.writeInt(666);
                }

                void read(serialization::ObjectDataInput & in, T& object) {
                    int i = in.readInt();
                    assert(i == 666);
                    object.id = in.readInt();
                    i = in.readInt();
                    assert(i == 666);
                }

                int getHazelcastTypeId() const {
                    return 666;
                };
            };
        }
    }
}

#endif //HAZELCAST_XmlSerializer

