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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATAFETCHER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATAFETCHER_H_

#include <vector>

#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/connection/CallFuture.h>
#include <hazelcast/util/SynchronizedMap.h>
#include "hazelcast/util/HazelcastDll.h"
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
                        /**
                         * Runs on near-cache side, an instance of this task is responsible for fetching of all near-caches' remote metadata like last
                         * sequence numbers and partition-uuids. To see usage of this metadata visit: {@link MetaDataGenerator}.
                         * <p>
                         * This class is abstract to provide different implementations on client and member sides.
                         */
                        class HAZELCAST_API MetaDataFetcher {
                        public:
                        void fetchMetadata(util::SynchronizedMap<std::string, RepairingHandler> &handlers) {
                                if (handlers.isEmpty()) {
                                    return;
                                }

                            std::vector<std::string> mapNames = getNames(handlers);
                            std::vector<connection::CallFuture> futures = scanMembers(mapNames);
                                for (std::vector<connection::CallFuture>::const_iterator it = futures.begin();it != futures.end();++it) {
                                    process(*it, handlers);
                                }
                            }

                        protected:
                            util::ILogger &logger;

                            virtual std::vector<connection::CallFuture> scanMembers(const std::vector<std::string> &names) = 0;

                            virtual void process(connection::CallFuture &future, util::SynchronizedMap<std::string, RepairingHandler> &handlers) = 0;

                            /**
                             * Gets or assigns partition-uuids before start of {@link RepairingTask} and returns a list of partition-id,
                             * partition-uuid pairs.
                             * <p>
                             * This method is likely to be called only one time during the life-cycle of a client.
                             *
                             * @return list of partition-id, partition-uuid pairs for initialization
                             * @throws Exception possible exceptions raised by remote calls
                             */
                        virtual std::vector<boost::shared_ptr<util::UUID> > assignAndGetUuids() = 0;

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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATAFETCHER_H_ */

