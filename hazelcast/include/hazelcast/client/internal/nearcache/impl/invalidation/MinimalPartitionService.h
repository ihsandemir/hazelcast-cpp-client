/*
 * Copyright (c) 2008-2016, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_MINIMALPARTITIONSERVICE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_MINIMALPARTITIONSERVICE_H_

#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace invalidation {
                        /**
                         * Abstraction over client partition services to provide commonly useful methods.
                         */
                        class HAZELCAST_API MinimalPartitionService {
                        public:
                            /**
                             * Returns the partition id for a Data key.
                             *
                             * @param key the Data key.
                             * @return the partition id.
                             * @throws NullPointerException if key is null.
                             */
                            virtual int getPartitionId(serialization::pimpl::Data &key) = 0;

                            /**
                             * Returns the partition id for a given object.
                             *
                             * @param key the object key.
                             * @return the partition id.
                             */
                            virtual int getPartitionId(const std::string &key) = 0;

                            /**
                             * Returns the number of partitions.
                             *
                             * @return the number of partitions.
                             */
                            virtual int getPartitionCount() const = 0;
                        };
                    }
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_MINIMALPARTITIONSERVICE_H_ */

