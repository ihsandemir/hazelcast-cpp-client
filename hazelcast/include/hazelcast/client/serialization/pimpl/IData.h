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
 * IData.h
 *
 * Data interface
 *
 *  Created on: Apr 18, 2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_IDATA
#define HAZELCAST_IDATA

#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/util/HazelcastDll.h"
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {

                class HAZELCAST_API IData {
                public:
                    // type and partition_hash are always written with BIG_ENDIAN byte-order
                    static unsigned int TYPE_OFFSET;
                    // will use a byte to store partition_hash bit
                    static unsigned int PARTITION_HASH_BIT_OFFSET;
                    static unsigned int DATA_OFFSET;

                    IData();

                    IData(std::auto_ptr<std::vector<byte> > buffer);

                    IData(const IData&);

                    IData& operator=(const IData&);

                    size_t dataSize() const;

                    size_t totalSize() const;

                    int getPartitionHash() const;

                    bool hasPartitionHash() const;

                    std::vector<byte> &toByteArray() const;

                    int getType() const;

                private:
                    mutable std::auto_ptr<std::vector<byte> > data;

                    int hashCode() const;

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_IDATA */

