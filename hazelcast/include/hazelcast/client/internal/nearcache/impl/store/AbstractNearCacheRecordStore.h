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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_

#include <stdint.h>
#include <memory>

#include <hazelcast/client/config/NearCacheConfig.h>
#include <hazelcast/client/internal/eviction/EvictionPolicyEvaluator.h>
#include <hazelcast/client/internal/eviction/MaxSizeChecker.h>
#include <hazelcast/client/internal/eviction/EvictionChecker.h>
#include <hazelcast/client/internal/eviction/EvictionStrategy.h>
#include <hazelcast/client/internal/eviction/EvictionPolicyType.h>
#include <hazelcast/client/serialization/pimpl/SerializationService.h>
#include "hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h"
#include "hazelcast/client/internal/eviction/EvictionListener.h"

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
                        template<typename K, typename V, typename KS, typename R, typename NCRM>
                        class AbstractNearCacheRecordStore
                                : public NearCacheRecordStore<K, V>, eviction::EvictionListener<KS, R> {
                        public:
                            AbstractNearCacheRecordStore(NearCacheConfig nearCacheConfig, SerializationService serializationService) {
                                this(nearCacheConfig, new NearCacheStatsImpl(), serializationService, classLoader);
                            }
                        protected:
                            AbstractNearCacheRecordStore(const boost::shared_ptr<config::NearCacheConfig> &cacheConfig,
/*
                                                         NearCacheStatsImpl nearCacheStats,
*/
                                                         serialization::pimpl::SerializationService &ss)
                                    : nearCacheConfig(cacheConfig),
                                      timeToLiveMillis(cacheConfig->getTimeToLiveSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      maxIdleMillis(cacheConfig->getMaxIdleSeconds() * MILLI_SECONDS_IN_A_SECOND),
                                      serializationService(ss),
                                      evictionPolicyType(cacheConfig->getEvictionConfig()->getEvictionPolicyType()) {
/*
                                this->nearCacheStats = nearCacheStats;
*/

                            }
/*
                        static const int REFERENCE_SIZE = MEM_AVAILABLE ? MEM.arrayIndexScale(Object[].class) : (Integer.SIZE / Byte.SIZE);
*/
                            const int64_t timeToLiveMillis;
                            const int64_t maxIdleMillis;
                            const boost::shared_ptr<config::NearCacheConfig> nearCacheConfig;
                            const serialization::pimpl::SerializationService &serializationService;
/*
                        const NearCacheStatsImpl nearCacheStats;
*/

                            std::auto_ptr<eviction::MaxSizeChecker> maxSizeChecker;
                            std::auto_ptr<eviction::EvictionPolicyEvaluator<KS, R> > evictionPolicyEvaluator;
                            std::auto_ptr<eviction::EvictionChecker> evictionChecker;
                            std::auto_ptr<eviction::EvictionStrategy<KS, R, NCRM> > evictionStrategy;
                            eviction::EvictionPolicyType evictionPolicyType;
                            std::auto_ptr<NCRM> records;

/*
                            volatile StaleReadDetector staleReadDetector = ALWAYS_FRESH;
*/
                        private:
                            static const int MILLI_SECONDS_IN_A_SECOND = 1000;
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_ABSTRACTNEARCACHERESCORDSTORE_H_ */

