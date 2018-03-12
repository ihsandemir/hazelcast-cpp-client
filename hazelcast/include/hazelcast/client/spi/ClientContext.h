/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 5/23/13.

#ifndef HAZELCAST_CLIENT_CONTEXT
#define HAZELCAST_CLIENT_CONTEXT

#include <string>
#include "hazelcast/util/HazelcastDll.h"
#include "ClientInvocationService.h"
#include "ClientClusterService.h"

namespace hazelcast {
    namespace client {

        class Cluster;

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        class HazelcastClient;

        class ClientConfig;

        class ClientProperties;

        namespace connection {
            class ClientConnectionManagerImpl;
        }

        namespace internal {
            namespace nearcache {
                class NearCacheManager;
            }
        }

        namespace protocol {
            class ClientExceptionFactory;
        }
        namespace spi {
            class InvocationService;

            class PartitionService;

            class ClientListenerService;

            class LifecycleService;

            namespace impl {
                class ClientExecutionServiceImpl;
                class ClientClusterServiceImpl;

                namespace sequence {
                    class CallIdSequence;
                }
            }

            class HAZELCAST_API ClientContext {
            public:

                ClientContext(HazelcastClient &hazelcastClient);

                serialization::pimpl::SerializationService &getSerializationService();

                impl::ClientClusterServiceImpl &getClientClusterService();

                hazelcast::client::spi::ClientInvocationService & getInvocationService();

                ClientConfig &getClientConfig();

                PartitionService &getPartitionService();

                LifecycleService &getLifecycleService();

                ClientListenerService &getClientListenerService();

                connection::ClientConnectionManagerImpl &getConnectionManager();

                internal::nearcache::NearCacheManager &getNearCacheManager() const;

                ClientProperties &getClientProperties();

                Cluster &getCluster();

                impl::sequence::CallIdSequence &getCallIdSequence() const;

                const protocol::ClientExceptionFactory &getClientExceptionFactory() const;

                const std::string &getName() const;

                spi::impl::ClientExecutionServiceImpl &getClientExecutionService() const;

                void onClusterConnect(const boost::shared_ptr<connection::Connection> &ownerConnection);
            private:
                HazelcastClient &hazelcastClient;
            };
        }
    }
}

#endif // HAZELCAST_CLIENT_CONTEXT
