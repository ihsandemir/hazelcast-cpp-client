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
#include <memory>
#include <stdint.h>
#include <vector>

#include "hazelcast/client/crdt/pncounter/PNCounter.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"
#include "hazelcast/client/map/impl/ClientMapProxyFactory.h"
#include "hazelcast/client/map/impl/ReplicatedMapProxyFactory.h"
#include "hazelcast/client/internal/nearcache/NearCacheManager.h"
#include "hazelcast/client/proxy/ClientRingbufferProxy.h"
#include "hazelcast/client/ringbuffer/impl/RingbufferProxyFactory.h"
#include "hazelcast/client/proxy/ClientReplicatedMapProxy.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/client/ReplicatedMap.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/TransactionOptions.h"
#include "hazelcast/client/TransactionContext.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ProxyManager.h"
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/ReliableTopic.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/impl/ClientTransactionManagerServiceImpl.h"
#include "hazelcast/client/impl/statistics/Statistics.h"
#include "hazelcast/client/FlakeIdGenerator.h"
#include "hazelcast/client/IExecutorService.h"
#include "hazelcast/client/Client.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class ClientConnectionManagerImpl;
            class AddressProvider;
        }

        namespace serialization {
            namespace pimpl {
                class SerializationService;
            }
        }
        namespace spi {
            class ClientContext;

            class ClientInvocationService;

            class LifecycleService;

            class ClientListenerService;

            class ClientProxyFactory;

            namespace impl {
                class ClientExecutionServiceImpl;
                class ClientPartitionServiceImpl;
            }
        }

        class ClientConfig;

        class TransactionContext;

        class TransactionOptions;

        class Cluster;


        namespace impl {
            class ClientLockReferenceIdGenerator;

            class HAZELCAST_API HazelcastClientInstanceImpl
                    : public std::enable_shared_from_this<HazelcastClientInstanceImpl> {
                friend class spi::ClientContext;
            public:
                /**
                * Constructs a hazelcastClient with given ClientConfig.
                * Note: ClientConfig will be copied.
                * @param config client configuration to start the client with
                */
                HazelcastClientInstanceImpl(const ClientConfig &config);

                /**
                * Destructor
                */
                ~HazelcastClientInstanceImpl();

                void start();

                /**
                 * Returns the name of this Hazelcast instance.
                 *
                 * @return name of this Hazelcast instance
                 */
                const std::string &getName() const;

                /**
                *
                * @tparam T type of the distributed object
                * @param name name of the distributed object.
                * @returns distributed object
                */
                template<typename T>
                std::shared_ptr<T> getDistributedObject(const std::string& name) {
                    return proxyManager.getOrCreateProxy<T>(T::SERVICE_NAME, name);
                }
                
                /**
                *
                * Returns the distributed map instance with the specified name.
                *
                * @tparam K key type
                * @tparam V value type
                * @param name name of the distributed map
                * @return distributed map instance with the specified name
                */
                std::shared_ptr<IMap> getMap(const std::string &name) {
                    return getDistributedObject<IMap>(name);
                }

                /**
                * Returns the distributed multimap instance with the specified name.
                *
                * @param name name of the distributed multimap
                * @return distributed multimap instance with the specified name
                */
                std::shared_ptr<MultiMap> getMultiMap(const std::string& name) {
                    return getDistributedObject<MultiMap>(name);
                }

                std::shared_ptr<ReplicatedMap> getReplicatedMap(const std::string &name) {
                    return getDistributedObject<proxy::ClientReplicatedMapProxy>(name);
                }

                /**
                * Returns the distributed queue instance with the specified name and entry type E.
                *
                * @param name name of the distributed queue
                * @return distributed queue instance with the specified name
                */
                std::shared_ptr<IQueue> getQueue(const std::string& name) {
                    return getDistributedObject<IQueue>(name);
                }

                /**
                * Returns the distributed set instance with the specified name and entry type E.
                * Set is ordered unique set of entries. similar to std::set
                *
                * @param name name of the distributed set
                * @return distributed set instance with the specified name
                */

                std::shared_ptr<ISet> getSet(const std::string& name) {
                    return getDistributedObject<ISet>(name);
                }

                /**
                * Returns the distributed list instance with the specified name.
                * List is ordered set of entries. similar to std::vector
                *
                * @param name name of the distributed list
                * @return distributed list instance with the specified name
                */
                std::shared_ptr<IList> getList(const std::string& name) {
                    return getDistributedObject<IList>(name);
                }

                /**
                * Returns the distributed topic instance with the specified name and entry type E.
                *
                * @param name name of the distributed topic
                * @return distributed topic instance with the specified name
                */
                std::shared_ptr<ITopic> getTopic(const std::string& name) {
                    return getDistributedObject<ITopic>(name);
                };

                /**
                * Returns the distributed topic instance with the specified name and entry type E.
                *
                * @param name name of the distributed topic
                * @return distributed topic instance with the specified name
                */
                std::shared_ptr<ReliableTopic> getReliableTopic(const std::string& name) {
                    return getDistributedObject<ReliableTopic>(name);
                }

                FlakeIdGenerator getFlakeIdGenerator(const std::string& name);

                /**
                 * Obtain a {@link com::hazelcast::crdt::pncounter::PNCounter} with the given
                 * name.
                 * <p>
                 * The PN counter can be used as a counter with strong eventual consistency
                 * guarantees - if operations to the counters stop, the counter values
                 * of all replicas that can communicate with each other should eventually
                 * converge to the same value.
                 *
                 * @param name the name of the PN counter
                 * @return a {@link com::hazelcast::crdt::pncounter::PNCounter}
                 */
                std::shared_ptr<crdt::pncounter::PNCounter> getPNCounter(const std::string& name);

                /**
                 * Returns the distributed Ringbuffer instance with the specified name.
                 *
                 * @param name name of the distributed Ringbuffer
                 * @return distributed RingBuffer instance with the specified name
                 */
                std::shared_ptr<Ringbuffer<E> > getRingbuffer(const std::string& name) {
                    return getDistributedObject<proxy::ClientRingbufferProxy>(name);
                }

                /**
                 * Creates or returns the distributed executor service for the given name.
                 * Executor service enables you to run your <tt>Runnable</tt>s and <tt>Callable</tt>s
                 * on the Hazelcast cluster.
                 * <p>
                 * <p><b>Note:</b> Note that it doesn't support {@code invokeAll/Any}
                 * and doesn't have standard shutdown behavior</p>
                 *
                 * @param name name of the executor service
                 * @return the distributed executor service for the given name
                 */
                std::shared_ptr<IExecutorService> getExecutorService(const std::string &name);

                /**
                *
                * @return configuration of this Hazelcast client.
                */
                ClientConfig& getClientConfig();

                /**
                * Creates a new TransactionContext associated with the current thread using default options.
                *
                * @return new TransactionContext
                */
                TransactionContext newTransactionContext();

                /**
                * Creates a new TransactionContext associated with the current thread with given options.
                *
                * @param options options for this transaction
                * @return new TransactionContext
                */
                TransactionContext newTransactionContext(const TransactionOptions& options);

                /**
                * Returns the Cluster that connected Hazelcast instance is a part of.
                * Cluster interface allows you to add listener for membership
                * events and learn more about the cluster.
                *
                * @return cluster
                */
                Cluster& getCluster();

                Client getLocalEndpoint() const;

                /**
                * Add listener to listen lifecycle events.
                *
                * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
                * otherwise it will slow down the system.
                *
                * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
                *
                * @param lifecycleListener Listener object
                */
                void addLifecycleListener(LifecycleListener *lifecycleListener);

                /**
                * Remove lifecycle listener
                * @param lifecycleListener
                * @return true if removed successfully
                */
                bool removeLifecycleListener(LifecycleListener *lifecycleListener);

                /**
                * Shuts down this HazelcastClient.
                */
                void shutdown();

                spi::LifecycleService &getLifecycleService();

                internal::nearcache::NearCacheManager &getNearCacheManager();

                serialization::pimpl::SerializationService &getSerializationService();

                const protocol::ClientExceptionFactory &getExceptionFactory() const;

                void onClusterConnect(const std::shared_ptr<connection::Connection> &ownerConnection);

                const std::shared_ptr<ClientLockReferenceIdGenerator> &getLockReferenceIdGenerator() const;

                spi::ProxyManager &getProxyManager();

                const std::shared_ptr <util::ILogger> &getLogger() const;

            private:
                ClientConfig clientConfig;
                ClientProperties clientProperties;
                spi::ClientContext clientContext;
                serialization::pimpl::SerializationService serializationService;
                std::shared_ptr<connection::ClientConnectionManagerImpl> connectionManager;
                std::unique_ptr<internal::nearcache::NearCacheManager> nearCacheManager;
                spi::impl::ClientClusterServiceImpl clusterService;
                std::shared_ptr<spi::impl::ClientPartitionServiceImpl> partitionService;
                std::shared_ptr<spi::impl::ClientExecutionServiceImpl> executionService;
                std::unique_ptr<spi::ClientInvocationService> invocationService;
                std::shared_ptr<spi::ClientListenerService> listenerService;
                spi::impl::ClientTransactionManagerServiceImpl transactionManager;
                Cluster cluster;
                spi::LifecycleService lifecycleService;
                spi::ProxyManager proxyManager;
                std::shared_ptr<spi::impl::sequence::CallIdSequence> callIdSequence;
                std::unique_ptr<statistics::Statistics> statistics;
                protocol::ClientExceptionFactory exceptionFactory;
                std::string instanceName;
                static std::atomic<int32_t> CLIENT_ID;
                int32_t id;
                std::shared_ptr<ClientLockReferenceIdGenerator> lockReferenceIdGenerator;
                std::shared_ptr<util::ILogger> logger;
                HazelcastClientInstanceImpl(const HazelcastClientInstanceImpl& rhs);

                void operator=(const HazelcastClientInstanceImpl& rhs);

                std::shared_ptr<spi::ClientProxy> getDistributedObjectForService(const std::string &serviceName,
                                                                                   const std::string &name,
                                                                                   spi::ClientProxyFactory &factory);

                std::shared_ptr<spi::ClientListenerService> initListenerService();

                std::unique_ptr<spi::ClientInvocationService> initInvocationService();

                std::shared_ptr<spi::impl::ClientExecutionServiceImpl> initExecutionService();

                std::shared_ptr<connection::ClientConnectionManagerImpl> initConnectionManagerService(
                        const std::vector<std::shared_ptr<connection::AddressProvider> > &addressProviders);

                std::vector<std::shared_ptr<connection::AddressProvider> > createAddressProviders();

                void startLogger();

                void initalizeNearCacheManager();
            };

            template<>
            std::shared_ptr<IMap> HazelcastClientInstanceImpl::get(const std::string &name) {
                std::shared_ptr<map::ClientMapProxy> impl;
                if (clientConfig.template getNearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>(
                        name)) {
                    return proxyManager.getOrCreateProxy<map::NearCachedClientMapProxy<serialization::pimpl::Data, serialization::pimpl::Data>>(
                            IMap::SERVICE_NAME, name);
                } 
                    
                return proxyManager.getOrCreateProxy<IMap>(IMap::SERVICE_NAME, name);
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
