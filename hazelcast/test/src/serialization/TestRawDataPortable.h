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
// Created by sancar koyunlu on 5/6/13.

#ifndef __RawDataPortable_H_
#define __RawDataPortable_H_

#include "TestNamedPortable.h"
#include "TestDataSerializable.h"
#include "TestSerializationConstants.h"
#include <hazelcast/client/serialization/serialization.h>

namespace hazelcast {
    namespace client {
        namespace test {
            struct TestRawDataPortable {
                int64_t l;
                std::vector<char> c;
                TestNamedPortable p;
                int32_t k;
                std::string s;
                TestDataSerializable ds;
            };
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::TestRawDataPortable> : public portable_serializer {
                static const int32_t CLASS_ID = test::TestSerializationConstants::TEST_RAW_DATA_PORTABLE;

                static int32_t getFactoryId();

                static int32_t getClassId();

                static void writePortable(const test::TestRawDataPortable &object, serialization::PortableWriter &writer);

                static test::TestRawDataPortable readPortable(serialization::PortableReader &reader);
            };
        }
    }
}


#endif //__RawDataPortable_H_


