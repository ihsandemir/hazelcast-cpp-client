/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <atomic>

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/impl/Partition.h"
#include "hazelcast/util/ILogger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            class ClientContext;

            namespace impl {
                class HAZELCAST_API ClientPartitionServiceImpl : public std::enable_shared_from_this<ClientPartitionServiceImpl> {
                public:
                    ClientPartitionServiceImpl(ClientContext &client);

                    /**
                     * The partitions can be empty on the response, client will not apply the empty partition table,
                     */
                    void handle_event(const std::shared_ptr<connection::Connection>& connection, int32_t version,
                                      const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions);

                    boost::uuids::uuid getPartitionOwner(int partitionId);

                    int32_t getPartitionId(const serialization::pimpl::Data &key);

                    int32_t getPartitionCount();

                    std::shared_ptr<client::impl::Partition> getPartition(int32_t partitionId);

                    bool check_and_set_partition_count(int32_t new_partition_count);

                    void reset();
                private:
                    struct partition_table {
                        std::shared_ptr<connection::Connection> connection;
                        int32_t version;
                        std::unordered_map<int32_t, boost::uuids::uuid> partitions;
                    };

                    class PartitionImpl : public client::impl::Partition {
                    public:
                        PartitionImpl(int partitionId, ClientContext &client,
                                      ClientPartitionServiceImpl &partitionService);

                        int getPartitionId() const override;

                        boost::optional<Member> getOwner() const override;

                    private:
                        int partitionId;
                        ClientContext &client;
                        ClientPartitionServiceImpl &partitionService;
                    };

                    bool should_be_applied(const std::shared_ptr<connection::Connection>& connection, int32_t version,
                                           const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions,
                                           const partition_table &current);

                    void log_failure(const std::shared_ptr<connection::Connection> &connection, int32_t version,
                                     const partition_table &current, const std::string &cause);

                    std::unordered_map<int32_t, boost::uuids::uuid>
                    convert_to_map(const std::vector<std::pair<boost::uuids::uuid, std::vector<int>>> &partitions);

                    ClientContext &client;
                    util::ILogger &logger_;
                    std::atomic<int32_t> partitionCount;
                    boost::atomic_shared_ptr<partition_table> partition_table_;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


