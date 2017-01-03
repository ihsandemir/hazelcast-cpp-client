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
#include "hazelcast/client/internal/nearcache/impl/invalidation/MetaDataContainer.h"

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace invalidation {
                        MetaDataContainer::MetaDataContainer() : sequence(0), staleSequence(0), missedSequenceCount(0) {
                        }

                        boost::shared_ptr<util::UUID> MetaDataContainer::getUuid() {
                            return uuid;
                        }

                        void MetaDataContainer::setUuid(const boost::shared_ptr<util::UUID> &id) {
                            uuid = id;
                        }

                        bool MetaDataContainer::casUuid(const boost::shared_ptr<util::UUID> &prevUuid,
                                                        const boost::shared_ptr<util::UUID> &newUuid) {
                            return uuid.compareAndSet(prevUuid, newUuid);
                        }

                        int64_t MetaDataContainer::getSequence() {
                            return sequence;
                        }

                        bool MetaDataContainer::casSequence(int64_t currentSequence, int64_t nextSequence) {
                            return sequence.compareAndSet(currentSequence, nextSequence);
                        }

                        void MetaDataContainer::resetSequence() {
                            sequence = 0;
                        }

                        int64_t MetaDataContainer::getStaleSequence() {
                            return staleSequence;
                        }

                        bool MetaDataContainer::casStaleSequence(int64_t lastKnownStaleSequence,
                                                                 int64_t lastReceivedSequence) {
                            return staleSequence.compareAndSet(lastKnownStaleSequence, lastReceivedSequence);
                        }

                        void MetaDataContainer::resetStaleSequence() {
                            staleSequence = 0;
                        }

                        int64_t MetaDataContainer::addAndGetMissedSequenceCount(int64_t missCount) {
                            return missedSequenceCount.addAndGet(missCount);
                        }

                        int64_t MetaDataContainer::getMissedSequenceCount() {
                            return missedSequenceCount;
                        }
                    }
                }
            }
        }
    }
}
