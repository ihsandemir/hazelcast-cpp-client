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
#include <hazelcast/client/spi/impl/SmartClientInvocationService.h>
#include <hazelcast/client/spi/impl/NonSmartClientInvocationService.h>
#include <hazelcast/client/spi/impl/listener/NonSmartClientListenerService.h>
#include <hazelcast/client/spi/impl/listener/SmartClientListenerService.h>
#include <hazelcast/client/spi/impl/ClientExecutionServiceImpl.h>
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IdGenerator.h"
#include "hazelcast/client/ICountDownLatch.h"
#include "hazelcast/client/ISemaphore.h"
#include "hazelcast/client/ILock.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/mixedtype/impl/HazelcastClientImpl.h"

#ifndef HAZELCAST_VERSION
#define HAZELCAST_VERSION "NOT_FOUND"
#endif

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4355) //for strerror	
#endif

namespace hazelcast {
    namespace client {
        util::Atomic<int32_t> HazelcastClient::CLIENT_ID(0);

        HazelcastClient::HazelcastClient(ClientConfig &config)
        : clientConfig(config)
        , clientProperties(clientConfig)
        , shutdownLatch(1)
        , clientContext(*this)
        , serializationService(clientConfig.getSerializationConfig())
        , connectionManager(new connection::ClientConnectionManagerImpl(clientContext, clientConfig.isSmart()))
        , nearCacheManager(serializationService)
        , clusterService(clientContext)
        , partitionService(clientContext)
        , cluster(clusterService)
        , lifecycleService(clientContext, clientConfig.getLifecycleListeners(), shutdownLatch,
                           clientConfig.getLoadBalancer(), cluster)
        , proxyManager(clientContext)
        , TOPIC_RB_PREFIX("_hz_rb_")
        , id(++CLIENT_ID) {
            const boost::shared_ptr<std::string> &name = config.getInstanceName();
            if (name.get() != NULL) {
                instanceName = *name;
            } else {
                instanceName = "hz.client_" + id;
            }

            std::stringstream prefix;
            prefix << instanceName << "[" << clientConfig.getGroupConfig().getName() << "] [" << HAZELCAST_VERSION << "]";
            util::ILogger::getLogger().setPrefix(prefix.str());

            executionService = initExecutionService();
            invocationService = initInvocationService();
            listenerService = initListenerService();

            try {
                if (!lifecycleService.start()) {
                    throw exception::IllegalStateException("HazelcastClient","HazelcastClient could not be started!");
                }
            } catch (exception::IException &) {
                lifecycleService.shutdown();
                throw;
            }
            mixedTypeSupportAdaptor.reset(new mixedtype::impl::HazelcastClientImpl(*this));
        }

        HazelcastClient::~HazelcastClient() {
            lifecycleService.shutdown();
            /**
             * We can not depend on the destruction order of the variables. lifecycleService may be destructed later
             * than the clientContext which is accessed by different service threads, hence we need to explicitly wait
             * for shutdown completion.
             */
            shutdownLatch.await();
        }


        ClientConfig &HazelcastClient::getClientConfig() {
            return clientConfig;
        }

        Cluster &HazelcastClient::getCluster() {
            return cluster;
        }

        void HazelcastClient::addLifecycleListener(LifecycleListener *lifecycleListener) {
            lifecycleService.addLifecycleListener(lifecycleListener);
        }

        bool HazelcastClient::removeLifecycleListener(LifecycleListener *lifecycleListener) {
            return lifecycleService.removeLifecycleListener(lifecycleListener);
        }

        void HazelcastClient::shutdown() {
            lifecycleService.shutdown();

            serializationService.dispose();
        }

        IdGenerator HazelcastClient::getIdGenerator(const std::string &instanceName) {
            return getDistributedObject< IdGenerator >(instanceName);
        }

        IAtomicLong HazelcastClient::getIAtomicLong(const std::string &instanceName) {
            return getDistributedObject< IAtomicLong >(instanceName);
        }

        ICountDownLatch HazelcastClient::getICountDownLatch(const std::string &instanceName) {
            return getDistributedObject< ICountDownLatch >(instanceName);
        }

        ISemaphore HazelcastClient::getISemaphore(const std::string &instanceName) {
            return getDistributedObject< ISemaphore >(instanceName);
        }

        ILock HazelcastClient::getILock(const std::string &instanceName) {
            return getDistributedObject< ILock >(instanceName);
        }

        TransactionContext HazelcastClient::newTransactionContext() {
            TransactionOptions defaultOptions;
            return newTransactionContext(defaultOptions);
        }

        TransactionContext HazelcastClient::newTransactionContext(const TransactionOptions &options) {
            return TransactionContext(clientContext, options);
        }

        internal::nearcache::NearCacheManager &HazelcastClient::getNearCacheManager() {
            return nearCacheManager;
        }

        serialization::pimpl::SerializationService &HazelcastClient::getSerializationService() {
            return serializationService;
        }

        boost::shared_ptr<spi::ClientProxy> HazelcastClient::getDistributedObjectForService(
                const std::string &serviceName,
                const std::string &name,
                spi::ClientProxyFactory &factory) {
            return proxyManager.getOrCreateProxy(serviceName, name, factory);
        }

        mixedtype::HazelcastClient &HazelcastClient::toMixedType() const {
            return *mixedTypeSupportAdaptor;
        }

        const spi::impl::sequence::CallIdSequence &HazelcastClient::getCallIdSequence() const {
            return callIdSequence;
        }

        const protocol::ClientExceptionFactory &HazelcastClient::getExceptionFactory() const {
            return exceptionFactory;
        }

        std::auto_ptr<spi::ClientListenerService> HazelcastClient::initListenerService() {
            int eventQueueCapacity = clientProperties.getEventQueueCapacity().getInteger();
            int eventThreadCount = clientProperties.getEventThreadCount().getInteger();
            config::ClientNetworkConfig &networkConfig = clientConfig.getNetworkConfig();
            if (networkConfig.isSmartRouting()) {
                return std::auto_ptr<spi::ClientListenerService>(
                        new spi::impl::listener::SmartClientListenerService(clientContext, eventThreadCount,
                                                                            eventQueueCapacity));
            } else {
                return std::auto_ptr<spi::ClientListenerService>(
                        new spi::impl::listener::NonSmartClientListenerService(clientContext, eventThreadCount,
                                                                               eventQueueCapacity));
            }
        }

        std::auto_ptr<spi::ClientInvocationService> HazelcastClient::initInvocationService() {
            if (clientConfig.getNetworkConfig().isSmartRouting()) {
                return std::auto_ptr<spi::ClientInvocationService>(
                        new spi::impl::SmartClientInvocationService(clientContext));
            } else {
                return std::auto_ptr<spi::ClientInvocationService>(
                        new spi::impl::NonSmartClientInvocationService(clientContext));
            }
        }

        std::auto_ptr<spi::impl::ClientExecutionServiceImpl> HazelcastClient::initExecutionService() {
            return std::auto_ptr<spi::impl::ClientExecutionServiceImpl>(
                    new spi::impl::ClientExecutionServiceImpl(instanceName, clientProperties,
                                                              clientConfig.getExecutorPoolSize()));
        }

        spi::ClientExecutionService &HazelcastClient::getClientExecutionService() const {
            return *executionService;
        }

        void HazelcastClient::onClusterConnect(const boost::shared_ptr<connection::Connection> &ownerConnection) {
            partitionService.listenPartitionTable(ownerConnection);
            clusterService.listenMembershipEvents(ownerConnection);
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

