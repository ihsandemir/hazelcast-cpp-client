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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGHANDLER_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGHANDLER_H_

#include <string>
#include <vector>
#include <assert.h>

#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/internal/nearcache/NearCache.h>
#include <hazelcast/util/AtomicReferenceArray.h>
#include <hazelcast/util/Util.h>
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/internal/nearcache/impl/invalidation/MinimalPartitionService.h"
#include "hazelcast/client/internal/nearcache/impl/invalidation/MetaDataContainer.h"

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
                         * Handler used on near-cache side. Observes local and remote invalidations and registers relevant
                         * data to {@link MetaDataContainer}s
                         *
                         * Used to repair near-cache in the event of invalidation-event-miss
                         * or partition uuid changes. Here repairing is done by making relevant near-cache data unreachable. To make
                         * stale data unreachable {@link StaleReadDetectorImpl} is used.
                         *
                         * An instance of this class is created per near-cache and can concurrently be used by many threads.
                         *
                         * @see StaleReadDetectorImpl
                         */
                        template<typename K, typename V>
                        class RepairingHandler {
                        public:
                            RepairingHandler(const std::string &cacheName, NearCache<K, V> &cache,
                                             MinimalPartitionService &minimalPartitionService,
                                             const std::string &uuid, util::ILogger &log)
                                    : name(cacheName), localUuid(uuid), nearCache(cache), logger(log),
                                      partitionService(minimalPartitionService),
                                      partitionCount(minimalPartitionService.getPartitionCount()),
                                      metaDataContainers((size_t) minimalPartitionService.getPartitionCount()) {
                            }

                            void initUnknownUuids(util::AtomicReferenceArray<util::UUID> &partitionUuids) {
                                for (int partition = 0; partition < partitionCount; partition++) {
                                    metaDataContainers[partition].casUuid(boost::shared_ptr<util::UUID>(),
                                                                          partitionUuids.get(partition));
                                }
                            }

                            MetaDataContainer &getMetaDataContainer(int partition) const {
                                return metaDataContainers[partition];
                            }

                            /**
                             * Handles a single invalidation
                             */
                            void handle(std::auto_ptr<serialization::pimpl::Data> &key, const std::string &sourceUuid,
                                        const boost::shared_ptr<util::UUID> &partitionUuid, int64_t sequence) {
                                // Apply invalidation if it is not originated by local member/client. Because local near-caches are invalidated
                                // immediately. No need to invalidate them twice.
                                if (localUuid != sourceUuid) {
                                    // sourceUuid is allowed to be null.
                                    if (key.get() == NULL) {
                                        nearCache.clear();
                                    } else {
                                        nearCache.remove(boost::shared_ptr<K>(key));
                                    }
                                }

                                int partitionId = getPartitionIdOrDefault(key.get());
                                checkOrRepairUuid(partitionId, partitionUuid);
                                checkOrRepairSequence(partitionId, sequence, false);
                            }

                            /**
                             * Handles batch invalidations
                             */
                            void handle(std::vector<serialization::pimpl::Data> &keys,
                                        std::vector<std::string> &sourceUuids,
                                        std::vector<boost::shared_ptr<util::UUID> > &partitionUuids,
                                        std::vector<int64_t> &sequences) {
                                std::vector<hazelcast::client::serialization::pimpl::Data>::const_iterator keyIterator = keys.begin();
                                std::vector<int64_t>::const_iterator sequenceIterator = sequences.begin();
                                std::vector<boost::shared_ptr<util::UUID> >::const_iterator partitionUuidIterator = partitionUuids.begin();
                                std::vector<std::string>::const_iterator sourceUuidsIterator = sourceUuids.begin();

                                do {
                                    if (!(keyIterator != keys.end() && sequenceIterator != sequences.end()
                                          && partitionUuidIterator != partitionUuids.end() &&
                                          sourceUuidsIterator != sourceUuids.end())) {
                                        break;
                                    }

                                    std::auto_ptr<serialization::pimpl::Data> key;
                                    serialization::pimpl::Data keyData = *keyIterator++;
                                    if (keyData.dataSize() > 0) {
                                        key = std::auto_ptr<serialization::pimpl::Data>(
                                                new serialization::pimpl::Data(keyData));
                                    }
                                    handle(key, *sourceUuidsIterator++, *partitionUuidIterator++,
                                           *sequenceIterator++);
                                } while (true);
                            }

                            const std::string &getName() const {
                                return name;
                            }

                            // TODO really need to pass partition-id?
                            void updateLastKnownStaleSequence(MetaDataContainer &metaData, int partition) {
                                int64_t lastReceivedSequence;
                                int64_t lastKnownStaleSequence;

                                do {
                                    lastReceivedSequence = metaData.getSequence();
                                    lastKnownStaleSequence = metaData.getStaleSequence();

                                    if (lastKnownStaleSequence >= lastReceivedSequence) {
                                        break;
                                    }

                                } while (!metaData.casStaleSequence(lastKnownStaleSequence, lastReceivedSequence));


                                if (logger.isFinestEnabled()) {
                                    char msg[500];
                                    util::snprintf(msg, 500,
                                                   "%s:[map=%s,partition=%d,lowerSequencesStaleThan=%ld,lastReceivedSequence=%ld]",
                                                   "Stale sequences updated", name, partition,
                                                   metaData.getStaleSequence(), metaData.getSequence());
                                    logger.finest(msg);
                                }
                            }

                            // more than one thread can concurrently call this method: one is anti-entropy, other one is event service thread
                            void checkOrRepairUuid(int partition, const boost::shared_ptr<util::UUID> &newUuid) {
                                assert(newUuid.get() != NULL);

                                MetaDataContainer metaData = getMetaDataContainer(partition);

                                do {
                                    boost::shared_ptr<util::UUID> prevUuid = metaData.getUuid();
                                    if (prevUuid.get() != NULL && prevUuid->equals(*newUuid)) {
                                        break;
                                    }

                                    if (metaData.casUuid(prevUuid, newUuid)) {
                                        metaData.resetSequence();
                                        metaData.resetStaleSequence();

                                        if (logger.isFinestEnabled()) {
                                            char msg[500];
                                            util::snprintf(msg, 500,
                                                           "%s:[name=%s,partition=%d,prevUuid=%s,newUuid=%s]",
                                                           "Invalid uuid, lost remote partition data unexpectedly",
                                                           name, partition, *prevUuid, *newUuid);
                                            logger.finest(msg);
                                        }

                                        break;
                                    }
                                } while (true);

                            }

                            /**
                             * Checks {@code nextSequence} against current one. And updates current sequence if next one is bigger.
                             */
                            // more than one thread can concurrently call this method: one is anti-entropy, other one is event service thread
                            void checkOrRepairSequence(int partition, int64_t nextSequence, bool viaAntiEntropy) {
                                assert(nextSequence > 0);

                                MetaDataContainer &metaData = getMetaDataContainer(partition);

                                do {
                                    int64_t currentSequence = metaData.getSequence();
                                    if (currentSequence >= nextSequence) {
                                        break;
                                    }

                                    if (metaData.casSequence(currentSequence, nextSequence)) {
                                        int64_t sequenceDiff = nextSequence - currentSequence;
                                        if (viaAntiEntropy || sequenceDiff > 1L) {
                                            // we have found at least one missing sequence between current and next sequences. if miss is detected by
                                            // anti-entropy, number of missed sequences will be `miss = next - current`, otherwise it means miss is
                                            // detected by observing received invalidation event sequence numbers and number of missed sequences will be
                                            // `miss = next - current - 1`.
                                            int64_t missCount = viaAntiEntropy ? sequenceDiff : sequenceDiff - 1;
                                            int64_t totalMissCount = metaData.addAndGetMissedSequenceCount(missCount);

                                            if (logger.isFinestEnabled()) {
                                                char msg[500];
                                                util::snprintf(msg, 500,
                                                               "%s:[map=%s,partition=%d,currentSequence=%ld,nextSequence=%ld,totalMissCount=%ld]",
                                                               "Invalid sequence", name, partition, currentSequence,
                                                               nextSequence, totalMissCount);
                                                logger.finest(msg);
                                            }
                                        }

                                        break;
                                    }
                                } while (true);
                            }

                            std::ostream &operator<<(std::ostream &out) {
                                out << "RepairingHandler{"
                                << "name='" << name << '\''
                                << ", localUuid='" << localUuid << '\''
                                << '}';

                                return out;
                            }
                        private:
                            int getPartitionIdOrDefault(serialization::pimpl::Data *key) {
                                if (key == NULL) {
                                    // `name` is used to determine partition-id of map-wide events like clear.
                                    // since key is null, we are using `name` to find partition-id
                                    return partitionService.getPartitionId(name);
                                }
                                return partitionService.getPartitionId(*key);
                            }

                            const int partitionCount;
                            const std::string name;
                            const std::string localUuid;
                            NearCache<K, V> &nearCache;
                            util::ILogger &logger;
                            MinimalPartitionService &partitionService;
                            std::vector<MetaDataContainer> metaDataContainers;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_INVALIDATION_REPAIRINGHANDLER_H_ */

