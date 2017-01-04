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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGTASK_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGTASK_H_

#include <hazelcast/client/ClientProperties.h>
#include "hazelcast/util/HazelcastDll.h"
#include "MinimalPartitionService.h"
#include "RepairingHandler.h"

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
                        class HAZELCAST_API RepairingTask {
                        public:
                            RepairingTask(MetaDataFetcher &metaDataFetcher, MinimalPartitionService &partitionService,
                                          ClientProperties &properties,
                                          const std::string &localUuid, util::ILogger &logger);
                        private:
                            static const int64_t GET_UUID_TASK_SCHEDULE_MILLIS = 500;
                            static const int64_t HALF_MINUTE_MILLIS = 30 * 1000;
                            static const int64_t MIN_RECONCILIATION_INTERVAL_SECONDS = 30;

                            const int maxToleratedMissCount;
                            const long reconciliationIntervalNanos;
                            const int partitionCount;
                            const std::string &localUuid;
                            util::ILogger &logger;
                            util::AtomicReferenceArray<util::UUID> partitionUuids;
                            MinimalPartitionService &partitionService;
                            MetaDataFetcher &metaDataFetcher;
                            util::SynchronizedMap<std::string, RepairingHandler *> handlers;
                            util::AtomicBoolean running = new AtomicBoolean(false);

                            util::Atomic<int64_t> lastAntiEntropyRunNanos;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGTASK_H_ */

