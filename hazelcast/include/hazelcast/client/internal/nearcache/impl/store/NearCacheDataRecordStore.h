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
#ifndef HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_
#define HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_

#include "hazelcast/client/internal/nearcache/impl/store/BaseHeapNearCacheRecordStore.h"
#include "hazelcast/client/internal/nearcache/impl/record/NearCacheDataRecord.h"

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
                        template<typename K, typename V>
                        class NearCacheDataRecordStore
                                : public BaseHeapNearCacheRecordStore<K, V, record::NearCacheDataRecord> {
                        public:
                            NearCacheDataRecordStore(const std::string &name,
                                                     const config::NearCacheConfig &config,
                                                     serialization::pimpl::SerializationService &ss)
                                    : BaseHeapNearCacheRecordStore(name, config, ss) {
                            }

                            //@Override
                            const boost::shared_ptr<V> selectToSave(const boost::shared_ptr<V> &value,
                                                                    const boost::shared_ptr<serialization::pimpl::Data> &valueData) const {
                                if (NULL != valueData.get()) {
                                    return valueData;
                                }

                                if (NULL != value.get()) {
                                    return boost::shared_ptr<V>(new serialization::pimpl::Data(
                                            serializationService.toData<V>(value.get())));
                                }

                                return boost::shared_ptr<V>();
                            }
                        protected:
                            //@Override
/*
                        int64_t getKeyStorageMemoryCost(K key) const {
                                if (key instanceof Data) {
                                    return
                                        // reference to this key data inside map ("store" field)
                                            REFERENCE_SIZE
                                            // heap cost of this key data
                                            + ((Data) key).getHeapCost();
                                } else {
                                    // memory cost for non-data typed instance is not supported
                                    return 0L;
                                }
                            }

                            // TODO: we don't handle object header (mark, class definition) for heap memory cost
                            @Override
                        int64_t getRecordStorageMemoryCost(NearCacheDataRecord record) {
                                if (record == null) {
                                    return 0L;
                                }
                                Data value = record.getValue();
                                return
                                    // reference to this record inside map ("store" field)
                                        REFERENCE_SIZE
                                        // reference to "value" field
                                        + REFERENCE_SIZE
                                        // heap cost of this value data
                                        + (value != null ? value.getHeapCost() : 0)
                                        // 3 primitive int64_t typed fields: "creationTime", "expirationTime" and "accessTime"
                                        + (3 * (Long.SIZE / Byte.SIZE))
                                        // reference to "accessHit" field
                                        + REFERENCE_SIZE
                                        // primitive int typed "value" field in "AtomicInteger" typed "accessHit" field
                                        + (Integer.SIZE / Byte.SIZE);
                            }
*/

                            //@Override
                            std::auto_ptr<record::NearCacheDataRecord> valueToRecord(const boost::shared_ptr<V> &value) {
                                const boost::shared_ptr<serialization::pimpl::Data> data = toData<V>(value);
                                int64_t creationTime = util::currentTimeMillis();
                                if (timeToLiveMillis > 0) {
                                    return std::auto_ptr<record::NearCacheDataRecord>(
                                            new record::NearCacheDataRecord(data, creationTime,
                                                                            creationTime + timeToLiveMillis));
                                } else {
                                    return std::auto_ptr<record::NearCacheDataRecord>(
                                            new record::NearCacheDataRecord(data, creationTime,
                                                                            NearCacheRecord::TIME_NOT_SET));
                                }
                            }

                            //@Override
                            boost::shared_ptr<V> recordToValue(const record::NearCacheDataRecord *record) {
                                const boost::shared_ptr<serialization::pimpl::Data> value = record->getValue();
                                if (value.get() == NULL) {
/*
                                    nearCacheStats.incrementMisses();
*/
                                    return NearCache<K, V>::NULL_OBJECT;
                                }
                                return dataToValue(value);
                            }

                            //@Override
                            void putToRecord(boost::shared_ptr<record::NearCacheDataRecord> &record,
                                             const boost::shared_ptr<V> &value) {
                                record->setValue(toData(value));
                            }
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

#endif /* HAZELCAST_CLIENT_INTERNAL_NEARCACHE_IMPL_STORE_NEARCACHEDATARESCORDSTORE_H_ */

