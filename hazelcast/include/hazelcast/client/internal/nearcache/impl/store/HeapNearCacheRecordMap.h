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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_

#include <stdint.h>

#include <hazelcast/client/internal/eviction/Evictable.h>
#include "hazelcast/client/internal/nearcache/impl/SampleableNearCacheRecordMap.h"
#include "hazelcast/util/SampleableConcurrentHashMap.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace internal {
            namespace nearcache {
                namespace impl {
                    namespace store {
                        /**
                         * @param V A type that extends NearCacheRecord
                         */
                        template<typename K, typename V>
                        class HeapNearCacheRecordMap
                                : public util::SampleableConcurrentHashMap<K, V>,
                                  public SampleableNearCacheRecordMap<K, V> {
                        public:
                            HeapNearCacheRecordMap(serialization::pimpl::SerializationService &ss,
                                                   int32_t initialCapacity)
                                    : util::SampleableConcurrentHashMap<K, V>(initialCapacity),
                                      serializationService(ss) {
                            }

                            class NearCacheEvictableSamplingEntry
                                    : public util::SampleableConcurrentHashMap<K, V>::SamplingEntry,
                                      public eviction::EvictionCandidate<K, V> {
                            public:
                                NearCacheEvictableSamplingEntry(const boost::shared_ptr<K> &key,
                                                                const boost::shared_ptr<V> &value,
                                                                serialization::pimpl::SerializationService &ss)
                                        : util::SampleableConcurrentHashMap<K, V>::SamplingEntry(key, value),
                                          serializationService(ss) {
                                }

                                //@Override
                                boost::shared_ptr<K> getAccessor() const {
                                    return key;
                                }

                                //@Override
                                boost::shared_ptr<eviction::Evictable> getEvictable() const {
                                    return boost::static_pointer_cast<eviction::Evictable>(value);
                                }

                                //@Override
                                boost::shared_ptr<K> getKey() const {
                                    return boost::shared_ptr<K>(serializationService.toObject<K>(key.get()));
                                }

                                //@Override
                                std::auto_ptr<V> getValue() {
                                    return serializationService.toObject(value->getValue());
                                }

                                //@Override
                                int64_t getCreationTime() const {
                                    return value->getCreationTime();
                                }

                                //@Override
                                int64_t getLastAccessTime() const {
                                    return value->getLastAccessTime();
                                }

                                //@Override
                                int64_t getAccessHit() const {
                                    return value->getAccessHit();
                                }

                            private:
                                serialization::pimpl::SerializationService &serializationService;
                            };

                            typedef eviction::EvictionCandidate<K, V> C;
                            //@Override
                            int evict(const std::vector<boost::shared_ptr<C> > *evictionCandidates,
                            const boost::shared_ptr<eviction::EvictionListener<K, V> > &evictionListener) {
                                if (evictionCandidates == NULL) {
                                    return 0;
                                }
                                int actualEvictedCount = 0;
                                for (std::vector<boost::shared_ptr<C> >::const_iterator it = evictionCandidates->begin();it != evictionCandidates->end();++it) {
                                    const boost::shared_ptr<C> &evictionCandidate = *it;
                                    if (remove(evictionCandidate->getAccessor()).get() != NULL) {
                                        actualEvictedCount++;
                                        if (evictionListener.get() != NULL) {
                                            evictionListener->onEvict(evictionCandidate->getAccessor(), evictionCandidate->getEvictable(), false);
                                        }
                                    }
                                }
                                return actualEvictedCount;
                            }

                            //@Override
                        Iterable<NearCacheEvictableSamplingEntry> sample(int sampleCount) {
                                return util::SampleableConcurrentHashMap<K, V>::getRandomSamples(sampleCount);
                            }
                            
                            
                        protected:
                            typedef util::SampleableConcurrentHashMap<K, V>::SamplingEntry E;
                            //@Override
                            std::auto_ptr<E> createSamplingEntry(boost::shared_ptr<K> &key, boost::shared_ptr<V> &value) const {
                                return std::auto_ptr<E>(
                                        new NearCacheEvictableSamplingEntry(key, value, serializationService));
                            }

                        private:
                            serialization::pimpl::SerializationService &serializationService;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_HEAPNEARCACHERECORDMAP_H_ */

