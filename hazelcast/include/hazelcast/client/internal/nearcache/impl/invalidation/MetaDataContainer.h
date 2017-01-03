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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATACONTAINER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATACONTAINER_H_

#include <stdint.h>
#include <boost/shared_ptr.hpp>

#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/internal/nearcache/NearCache.h>
#include <hazelcast/util/Atomic.h>
#include <hazelcast/util/UUID.h>
#include "hazelcast/util/HazelcastDll.h"

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
                         * Contains one partitions' invalidation metadata.
                         */
                        class HAZELCAST_API MetaDataContainer {
                        public:
                            MetaDataContainer();

                            boost::shared_ptr<util::UUID> getUuid();

                            void setUuid(const boost::shared_ptr<util::UUID> &id);

                            bool casUuid(const boost::shared_ptr<util::UUID> &prevUuid,
                                         const boost::shared_ptr<util::UUID> &newUuid);

                            int64_t getSequence();

                            bool casSequence(int64_t currentSequence, int64_t nextSequence);

                            void resetSequence();

                            int64_t getStaleSequence();

                            bool casStaleSequence(int64_t lastKnownStaleSequence, int64_t lastReceivedSequence);

                            void resetStaleSequence();

                            int64_t addAndGetMissedSequenceCount(int64_t missCount);

                            int64_t getMissedSequenceCount();

                        private:
                            //TODO: Change to use real atomics(AtomicLongFieldUpdater) when atomic support is added
                            // to the project
                            /**
                             * Sequence number of last received invalidation event
                             */
                            util::Atomic<int64_t> sequence;

                            /**
                             * Holds the biggest sequence number that is lost, lower sequences from this sequence are accepted as stale
                             *
                             * @see StaleReadDetector
                             */
                            util::Atomic<int64_t> staleSequence;

                            /**
                             * Number of missed sequence count
                             */
                            util::Atomic<int64_t> missedSequenceCount;

                            /**
                             * Uuid of the source partition that generates invalidation events
                             */
                            util::Atomic<boost::shared_ptr<util::UUID> > uuid;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_METADATACONTAINER_H_ */

