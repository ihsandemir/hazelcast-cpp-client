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

#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/connection/DefaultClientConnectionStrategy.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/AuthenticationStatus.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/exception/AuthenticationException.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/connection/ConnectionListener.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/impl/ClientClusterServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/connection/AuthenticationFuture.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/connection/HeartbeatManager.h"
#include "hazelcast/client/impl/HazelcastClientInstanceImpl.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/internal/socket/TcpSocket.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/impl/BuildInfo.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/config/SSLConfig.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        SocketInterceptor::~SocketInterceptor() {
        }

        namespace connection {
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC = 2;
            int ClientConnectionManagerImpl::DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC = 20;

            ClientConnectionManagerImpl::ClientConnectionManagerImpl(spi::ClientContext &client,
                                                                     const std::shared_ptr<AddressTranslator> &addressTranslator,
                                                                     const std::vector<std::shared_ptr<AddressProvider> > &addressProviders)
                    : logger(client.getLogger()), client(client),
                      socketInterceptor(client.getClientConfig().getSocketInterceptor()),
                      executionService(client.getClientExecutionService()),
                      translator(addressTranslator), clusterConnectionExecutor(1), connectionIdGen(0),
                      socketFactory(client, ioContext), heartbeat(client) {
                config::ClientNetworkConfig &networkConfig = client.getClientConfig().getNetworkConfig();

                int64_t connTimeout = networkConfig.getConnectionTimeout();
                connectionTimeoutMillis = connTimeout == 0 ? INT64_MAX : connTimeout;

                credentials = client.getClientConfig().getCredentials();

                connectionStrategy = initializeStrategy(client);

                ClientProperties &clientProperties = client.getClientProperties();
                shuffleMemberList = clientProperties.getBoolean(clientProperties.getShuffleMemberList());

                ClientConnectionManagerImpl::addressProviders = addressProviders;

                connectionAttemptPeriod = networkConfig.getConnectionAttemptPeriod();

                int connAttemptLimit = networkConfig.getConnectionAttemptLimit();
                bool isAsync = client.getClientConfig().getConnectionStrategyConfig().isAsyncStart();

                if (connAttemptLimit < 0) {
                    this->connectionAttemptLimit = isAsync ? DEFAULT_CONNECTION_ATTEMPT_LIMIT_ASYNC
                                                           : DEFAULT_CONNECTION_ATTEMPT_LIMIT_SYNC;
                } else {
                    this->connectionAttemptLimit = connAttemptLimit == 0 ? INT32_MAX : connAttemptLimit;
                }

                ioThreadCount = clientProperties.getInteger(clientProperties.getIOThreadCount());
            }

            bool ClientConnectionManagerImpl::start() {
                util::LockGuard guard(lock);
                if (alive) {
                    return true;
                }
                alive.store(true);

                if (!socketFactory.start()) {
                    return false;
                }

                socketInterceptor = client.getClientConfig().getSocketInterceptor();

                for (int j = 0; j < ioThreadCount; ++j) {
                    ioThreads.emplace_back([=]() {
                        boost::asio::executor_work_guard<decltype(ioContext.get_executor())> work{
                                ioContext.get_executor()};
                        ioContext.run();
                    });
                }

                heartbeat.start();
                connectionStrategy->start();

                return true;
            }

            void ClientConnectionManagerImpl::shutdown() {
                util::LockGuard guard(lock);
                if (!alive) {
                    return;
                }
                alive.store(false);

                ioContext.stop();

                heartbeat.shutdown();

                connectionStrategy->shutdown();

                // let the waiting authentication futures not block anymore
                for (auto &authFutureTuple : connectionsInProgress.values()) {
                    auto &authFuture = std::get<0>(*authFutureTuple);
                    authFuture->onFailure(
                            std::make_exception_ptr(
                                    exception::IllegalStateException("ClientConnectionManagerImpl::shutdown",
                                                                     "Client is shutting down")));
                    std::get<1>(*authFutureTuple)->close();
                }

                // close connections
                for (auto &connection : activeConnections.values()) {
                    // prevent any exceptions
                    util::IOUtil::closeResource(connection.get(), "Hazelcast client is shutting down");
                }

                clusterConnectionExecutor.stop();

                connectionListeners.clear();
                activeConnections.clear();

                std::for_each(ioThreads.begin(), ioThreads.end(), [](std::thread &t) { t.join(); });
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address) {
                return getOrConnect(address, false);
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getOwnerConnection() {
                std::shared_ptr<Address> address = ownerConnectionAddress;
                if (address.get() == NULL) {
                    return std::shared_ptr<Connection>();
                }
                std::shared_ptr<Connection> connection = getActiveConnection(*address);
                return connection;

            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::connectAsOwner(const Address &address) {
                std::shared_ptr<Connection> connection;
                try {
                    logger.info("Trying to connect to ", address, " as owner member");
                    connection = getOrConnect(address, true);
                    if (connection == nullptr) {
                        return nullptr;
                    }
                    client.onClusterConnect(connection);
                    fireConnectionEvent(LifecycleEvent::CLIENT_CONNECTED);
                    connectionStrategy->onConnectToCluster();
                } catch (exception::IException &e) {
                    logger.warning("Exception during initial connection to ", address, ", exception ", e);
                    if (NULL != connection.get()) {
                        std::ostringstream reason;
                        reason << "Could not connect to " << address << " as owner";
                        connection->close(reason.str().c_str(), std::current_exception());
                    }
                    return std::shared_ptr<Connection>();
                }
                return connection;
            }


            std::vector<std::shared_ptr<Connection> > ClientConnectionManagerImpl::getActiveConnections() {
                return activeConnections.values();
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getOrTriggerConnect(const Address &target) {
                std::shared_ptr<Connection> connection = getConnection(target, false);
                if (connection.get() != NULL) {
                    return connection;
                }
                triggerConnect(target, false);
                return std::shared_ptr<Connection>();
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getConnection(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeGetConnection(target);
                }
                if (!asOwner && getOwnerConnection().get() == NULL) {
                    throw exception::IOException("ConnectionManager::getConnection",
                                                 "Owner connection is not available!");
                }

                std::shared_ptr<Connection> connection = activeConnections.get(target);

                if (connection.get() != NULL) {
                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }

                    connection->reAuthenticateAsOwner();
                    return connection;
                }
                return std::shared_ptr<Connection>();
            }

            std::shared_ptr<Connection> ClientConnectionManagerImpl::getActiveConnection(const Address &target) {
                return activeConnections.get(target);
            }

            std::shared_ptr<Address> ClientConnectionManagerImpl::getOwnerConnectionAddress() {
                return ownerConnectionAddress;
            }

            std::shared_ptr<ClientConnectionManagerImpl::FutureTuple>
            ClientConnectionManagerImpl::triggerConnect(const Address &target, bool asOwner) {
                if (!asOwner) {
                    connectionStrategy->beforeOpenConnection(target);
                }
                if (!alive) {
                    throw exception::HazelcastException("ConnectionManager::triggerConnect",
                                                        "ConnectionManager is not active!");
                }

                Address address = translator->translate(target);
                std::shared_ptr<AuthenticationFuture> future(new AuthenticationFuture(address, connectionsInProgress));

                auto connection = std::make_shared<Connection>(address, client, ++connectionIdGen, future,
                                                               socketFactory, ioContext, asOwner, *this,
                                                               connectionTimeoutMillis);

                auto authTuple = std::make_shared<FutureTuple>(future, connection);
                auto oldFutureTuple = connectionsInProgress.putIfAbsent(address, authTuple);
                if (oldFutureTuple.get() == NULL) {
                    // double check here
                    auto activeConnection = activeConnections.get(target);
                    if (activeConnection.get()) {
                        connectionsInProgress.remove(address);
                        return std::make_shared<FutureTuple>(nullptr, activeConnection);
                    }

                    connection->asyncStart();

                    return authTuple;
                }
                return oldFutureTuple;
            }

            std::shared_ptr<Connection>
            ClientConnectionManagerImpl::getOrConnect(const Address &address, bool asOwner) {
                while (true) {
                    std::shared_ptr<Connection> connection = getConnection(address, asOwner);
                    if (connection.get() != NULL) {
                        return connection;
                    }
                    auto firstCallbackTuple = triggerConnect(address, asOwner);
                    auto firstCallback = std::get<0>(*firstCallbackTuple);
                    if (firstCallback == nullptr) {
                        auto activeConnection = std::get<1>(*firstCallbackTuple);
                        if (asOwner && !activeConnection->isAuthenticatedAsOwner()) {
                            activeConnection->reAuthenticateAsOwner();
                        }

                        return activeConnection;
                    }

                    if (!alive) {
                        std::get<1>(*firstCallbackTuple)->close("Client is being shutdown.");
                        firstCallback->onFailure(
                                std::make_exception_ptr(exception::IllegalStateException(
                                        "ClientConnectionManagerImpl::getOrConnect",
                                        "Client is being shutdown.")));
                        return nullptr;
                    }
                    connection = firstCallback->get();

                    // call the interceptor from user thread
                    if (socketInterceptor != NULL) {
                        socketInterceptor->onConnect(connection->getSocket());
                    }

                    if (!asOwner) {
                        return connection;
                    }
                    if (connection->isAuthenticatedAsOwner()) {
                        return connection;
                    }
                }
            }

            void
            ClientConnectionManagerImpl::authenticate(const Address &target, std::shared_ptr<Connection> &connection,
                                                      bool asOwner, std::shared_ptr<AuthenticationFuture> &future) {
                std::shared_ptr<protocol::Principal> clientPrincipal = getPrincipal();
                std::unique_ptr<protocol::ClientMessage> clientMessage = encodeAuthenticationRequest(asOwner,
                                                                                                     client.getSerializationService(),
                                                                                                     clientPrincipal.get());
                std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                        client, clientMessage, "", connection);
                auto invocationFuture = clientInvocation->invokeUrgent();

                auto authCallback = std::make_shared<AuthCallback>(connection, asOwner, target, future, *this);
                // TODO: Get rid of AuthenticationFuture completely
                // we need to set this so that the invocation is not destructed
                connection->setAuthenticationInvocationFuture(
                        invocationFuture.then(launch::sync, [=](boost::future<protocol::ClientMessage> f) {
                            try {
                                authCallback->onResponse(f.get());
                            } catch (exception::IException &e) {
                                authCallback->onFailure(std::current_exception());
                            }
                        }));
            }

            void
            ClientConnectionManagerImpl::reAuthenticate(const Address &target, std::shared_ptr<Connection> &connection,
                                                        bool asOwner, std::shared_ptr<AuthenticationFuture> &future) {
                future.reset(new AuthenticationFuture(target, connectionsInProgress));
                authenticate(target, connection, asOwner, future);
            }

            const std::shared_ptr<protocol::Principal> ClientConnectionManagerImpl::getPrincipal() {
                return principal;
            }

            std::unique_ptr<protocol::ClientMessage>
            ClientConnectionManagerImpl::encodeAuthenticationRequest(bool asOwner,
                                                                     serialization::pimpl::SerializationService &ss,
                                                                     const protocol::Principal *principal) {
                byte serializationVersion = ss.getVersion();
                const std::string *uuid = NULL;
                const std::string *ownerUuid = NULL;
                if (principal != NULL) {
                    uuid = principal->getUuid();
                    ownerUuid = principal->getOwnerUuid();
                }
                std::unique_ptr<protocol::ClientMessage> clientMessage;
                if (credentials == NULL) {
                    // TODO: Change UsernamePasswordCredentials to implement Credentials interface so that we can just
                    // upcast the credentials as done at Java
                    GroupConfig &groupConfig = client.getClientConfig().getGroupConfig();
                    const protocol::UsernamePasswordCredentials cr(groupConfig.getName(), groupConfig.getPassword());
                    clientMessage = protocol::codec::ClientAuthenticationCodec::encodeRequest(
                            cr.getPrincipal(), cr.getPassword(), uuid, ownerUuid, asOwner, protocol::ClientTypes::CPP,
                            serializationVersion, HAZELCAST_VERSION);
                } else {
                    serialization::pimpl::Data data = ss.toData<Credentials>(credentials);
                    clientMessage = protocol::codec::ClientAuthenticationCustomCodec::encodeRequest(data,
                                                                                                    uuid,
                                                                                                    ownerUuid,
                                                                                                    asOwner,
                                                                                                    protocol::ClientTypes::CPP,
                                                                                                    serializationVersion,
                                                                                                    HAZELCAST_VERSION);
                }
                return clientMessage;
            }

            void ClientConnectionManagerImpl::setPrincipal(const std::shared_ptr<protocol::Principal> &principal) {
                ClientConnectionManagerImpl::principal = principal;
            }

            void ClientConnectionManagerImpl::onAuthenticated(const Address &target,
                                                              const std::shared_ptr<Connection> &connection) {
                std::shared_ptr<Connection> oldConnection = activeConnections.put(*connection->getRemoteEndpoint(),
                                                                                  connection);

                if (oldConnection.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Authentication succeeded for ", *connection,
                                      " and there was no old connection to this end-point");
                    }
                    fireConnectionAddedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Re-authentication succeeded for ", *connection);
                    }
                    assert(*connection == *oldConnection);
                }

                connectionsInProgress.remove(target);
                std::ostringstream out;
                if (connection->getRemoteEndpoint().get()) {
                    out << *connection->getRemoteEndpoint();
                } else {
                    out << "null";
                }
                logger.info("Authenticated with server ", out.str(), ", server version:",
                            connection->getConnectedServerVersionString(), " Local address: ",
                            (connection->getLocalSocketAddress().get() != NULL
                             ? connection->getLocalSocketAddress()->toString() : "null"));

                /* check if connection is closed by remote before authentication complete, if that is the case
                we need to remove it back from active connections.
                Race description from https://github.com/hazelcast/hazelcast/pull/8832.(A little bit changed)
                - open a connection client -> member
                - send auth message
                - receive auth reply -> reply processing is offloaded to an executor. Did not start to run yet.
                - member closes the connection -> the connection is trying to removed from map
                                                                     but it was not there to begin with
                - the executor start processing the auth reply -> it put the connection to the connection map.
                - we end up with a closed connection in activeConnections map */
                if (!connection->isAlive()) {
                    removeFromActiveConnections(connection);
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionAddedEvent(const std::shared_ptr<Connection> &connection) {
                for (const std::shared_ptr<ConnectionListener> &connectionListener : connectionListeners.toArray()) {
                    connectionListener->connectionAdded(connection);
                }
                connectionStrategy->onConnect(connection);
            }

            void
            ClientConnectionManagerImpl::removeFromActiveConnections(const std::shared_ptr<Connection> &connection) {
                std::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();

                if (endpoint.get() == NULL) {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying ", *connection, ", but it has end-point set to null ",
                                      "-> not removing it from a connection map");
                    }
                    return;
                }

                if (activeConnections.remove(*endpoint, connection)) {
                    logger.info("Removed connection to endpoint: ", *endpoint, ", connection: ", *connection);
                    fireConnectionRemovedEvent(connection);
                } else {
                    if (logger.isFinestEnabled()) {
                        logger.finest("Destroying a connection, but there is no mapping ", endpoint, " -> ",
                                      *connection, " in the connection map.");
                    }
                }
            }

            void
            ClientConnectionManagerImpl::fireConnectionRemovedEvent(const std::shared_ptr<Connection> &connection) {
                if (connection->isAuthenticatedAsOwner()) {
                    disconnectFromCluster(connection);
                }

                for (const std::shared_ptr<ConnectionListener> &listener : connectionListeners.toArray()) {
                    listener->connectionRemoved(connection);
                }
                connectionStrategy->onDisconnect(connection);
            }

            void ClientConnectionManagerImpl::disconnectFromCluster(const std::shared_ptr<Connection> connection) {
                boost::asio::post(clusterConnectionExecutor, [=]() {
                    std::shared_ptr<Address> endpoint = connection->getRemoteEndpoint();
                    // it may be possible that while waiting on executor queue, the client got connected (another connection),
                    // then we do not need to do anything for cluster disconnect.
                    std::shared_ptr<Address> ownerAddress = ownerConnectionAddress;
                    if (ownerAddress.get() && (endpoint.get() && *endpoint != *ownerAddress)) {
                        return;
                    }

                    setOwnerConnectionAddress(std::shared_ptr<Address>());

                    try {
                        connectionStrategy->onDisconnectFromCluster();

                        if (client.getLifecycleService().isRunning()) {
                            fireConnectionEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                        }
                    } catch (exception::IException &e) {
                        logger.info("ClientConnectionManagerImpl::disconnectFromCluster Exception occured: ", e.what());
                    }
                });
            }

            void
            ClientConnectionManagerImpl::setOwnerConnectionAddress(
                    const std::shared_ptr<Address> &ownerConnectionAddress) {
                previousOwnerConnectionAddress = this->ownerConnectionAddress.get();
                ClientConnectionManagerImpl::ownerConnectionAddress = ownerConnectionAddress;
            }

            void
            ClientConnectionManagerImpl::fireConnectionEvent(
                    const hazelcast::client::LifecycleEvent::LifeCycleState &state) {
                spi::LifecycleService &lifecycleService = client.getLifecycleService();
                lifecycleService.fireLifecycleEvent(state);
            }

            std::future<bool> ClientConnectionManagerImpl::connectToClusterAsync() {
                std::packaged_task<bool()> task([=]() {
                    try {
                        connectToClusterInternal();
                        return true;
                    } catch (exception::IException &e) {
                        logger.warning("Could not connect to cluster, shutting down the client. ",
                                       e.getMessage());

                        static_cast<DefaultClientConnectionStrategy &>(*connectionStrategy).shutdownWithExternalThread(
                                client.getHazelcastClientImplementation());
                    }
                    return false;
                });
                return boost::asio::post(clusterConnectionExecutor, std::move(task));
            }

            void ClientConnectionManagerImpl::connectToClusterInternal() {
                int attempt = 0;
                std::set<Address> triedAddresses;

                while (attempt < connectionAttemptLimit) {
                    attempt++;
                    int64_t nextTry = util::currentTimeMillis() + connectionAttemptPeriod;

                    std::set<Address> addresses = getPossibleMemberAddresses();
                    for (const Address &address : addresses) {
                        if (!client.getLifecycleService().isRunning()) {
                            throw exception::IllegalStateException(
                                    "ConnectionManager::connectToClusterInternal",
                                    "Giving up on retrying to connect to cluster since client is shutdown.");
                        }
                        triedAddresses.insert(address);
                        if (connectAsOwner(address).get() != NULL) {
                            return;
                        }
                    }

                    // If the address providers load no addresses (which seems to be possible), then the above loop is not entered
                    // and the lifecycle check is missing, hence we need to repeat the same check at this point.
                    if (!client.getLifecycleService().isRunning()) {
                        throw exception::IllegalStateException("Client is being shutdown.");
                    }

                    if (attempt < connectionAttemptLimit) {
                        const int64_t remainingTime = nextTry - util::currentTimeMillis();
                        logger.warning("Unable to get alive cluster connection, try in ",
                                       (remainingTime > 0 ? remainingTime : 0), " ms later, attempt ", attempt,
                                       " of ", connectionAttemptLimit, ".");

                        if (remainingTime > 0) {
                            // TODO use a condition variable here
                            std::this_thread::sleep_for(std::chrono::milliseconds(remainingTime));
                        }
                    } else {
                        logger.warning("Unable to get alive cluster connection, attempt ", attempt, " of ",
                                       connectionAttemptLimit, ".");
                    }
                }
                std::ostringstream out;
                out << "Unable to connect to any address! The following addresses were tried: { ";
                for (const std::set<Address>::value_type &address : triedAddresses) {
                    out << address << " , ";
                }
                out << "}";
                throw exception::IllegalStateException("ConnectionManager::connectToClusterInternal", out.str());
            }

            std::set<Address> ClientConnectionManagerImpl::getPossibleMemberAddresses() {
                std::set<Address> addresses;

                std::vector<Member> memberList = client.getClientClusterService().getMemberList();
                std::vector<Address> memberAddresses;
                for (const Member &member : memberList) {
                    memberAddresses.push_back(member.getAddress());
                }

                if (shuffleMemberList) {
                    shuffle(memberAddresses);
                }

                addresses.insert(memberAddresses.begin(), memberAddresses.end());

                std::set<Address> providerAddressesSet;
                for (std::shared_ptr<AddressProvider> &addressProvider : addressProviders) {
                    std::vector<Address> addrList = addressProvider->loadAddresses();
                    providerAddressesSet.insert(addrList.begin(), addrList.end());
                }

                std::vector<Address> providerAddresses(providerAddressesSet.begin(), providerAddressesSet.end());

                if (shuffleMemberList) {
                    shuffle(memberAddresses);
                }

                addresses.insert(providerAddresses.begin(), providerAddresses.end());

                std::shared_ptr<Address> previousAddress = previousOwnerConnectionAddress.get();
                if (previousAddress.get() != NULL) {
                    /*
                     * Previous owner address is moved to last item in set so that client will not try to connect to same one immediately.
                     * It could be the case that address is removed because it is healthy(it not responding to heartbeat/pings)
                     * In that case, trying other addresses first to upgrade make more sense.
                     */
                    addresses.erase(*previousAddress);
                    addresses.insert(*previousAddress);
                }
                return addresses;
            }

            void ClientConnectionManagerImpl::shuffle(
                    std::vector<Address> &memberAddresses) const {// obtain a time-based seed:
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::shuffle(memberAddresses.begin(), memberAddresses.end(), std::default_random_engine(seed));
            }

            std::unique_ptr<ClientConnectionStrategy>
            ClientConnectionManagerImpl::initializeStrategy(spi::ClientContext &client) {
                // TODO: Add a way so that this strategy can be configurable as in Java
                return std::unique_ptr<ClientConnectionStrategy>(new DefaultClientConnectionStrategy(client, logger,
                                                                                                     client.getClientConfig().getConnectionStrategyConfig()));
            }

            void ClientConnectionManagerImpl::connectToCluster() {
                connectToClusterAsync().get();
            }

            bool ClientConnectionManagerImpl::isAlive() {
                return alive;
            }

            void ClientConnectionManagerImpl::onClose(Connection &connection) {
                removeFromActiveConnections(connection.shared_from_this());
            }

            void
            ClientConnectionManagerImpl::addConnectionListener(
                    const std::shared_ptr<ConnectionListener> &connectionListener) {
                connectionListeners.add(connectionListener);
            }

            ClientConnectionManagerImpl::~ClientConnectionManagerImpl() {
                shutdown();
            }

            util::ILogger &ClientConnectionManagerImpl::getLogger() {
                return client.getLogger();
            }

            ClientConnectionManagerImpl::AuthCallback::AuthCallback(const std::shared_ptr<Connection> &connection,
                                                                    bool asOwner,
                                                                    const Address &target,
                                                                    std::shared_ptr<AuthenticationFuture> &future,
                                                                    ClientConnectionManagerImpl &connectionManager)
                    : connection(connection), asOwner(asOwner),
                      target(target), future(future),
                      connectionManager(connectionManager),
                      cancelled(false) {
                scheduleTimeoutTask();
            }

            void ClientConnectionManagerImpl::AuthCallback::cancelTimeoutTask() {
                {
                    std::lock_guard<std::mutex> g(timeoutMutex);
                    cancelled = true;
                }
                timeoutCondition.notify_one();
                timeoutTaskFuture.get();
            }

            void ClientConnectionManagerImpl::AuthCallback::scheduleTimeoutTask() {
                timeoutTaskFuture = std::async([&] {
                    std::unique_lock<std::mutex> uniqueLock(timeoutMutex);
                    if (cancelled) {
                        return;
                    }

                    if (timeoutCondition.wait_for(uniqueLock,
                                                  std::chrono::milliseconds(connectionManager.connectionTimeoutMillis),
                                                  [&] { return cancelled; })) {
                        return;
                    }

                    future->onFailure(std::make_exception_ptr((exception::ExceptionBuilder<exception::TimeoutException>(
                            "ClientConnectionManagerImpl::authenticate")
                            << "Authentication response did not come back in "
                            << connectionManager.connectionTimeoutMillis
                            << " millis").build()));
                });
            }

            void ClientConnectionManagerImpl::AuthCallback::onResponse(protocol::ClientMessage response) {
                cancelTimeoutTask();

                std::unique_ptr<protocol::codec::ClientAuthenticationCodec::ResponseParameters> result;
                try {
                    result.reset(new protocol::codec::ClientAuthenticationCodec::ResponseParameters(
                            protocol::codec::ClientAuthenticationCodec::ResponseParameters::decode(response)));
                } catch (exception::IException &e) {
                    handleAuthenticationException(std::current_exception());
                    return;
                }
                protocol::AuthenticationStatus authenticationStatus = (protocol::AuthenticationStatus) result->status;
                switch (authenticationStatus) {
                    case protocol::AUTHENTICATED: {
                        connection->setConnectedServerVersion(result->serverHazelcastVersion);
                        connection->setRemoteEndpoint(std::shared_ptr<Address>(std::move(result->address)));
                        if (asOwner) {
                            connection->setIsAuthenticatedAsOwner();
                            std::shared_ptr<protocol::Principal> principal(
                                    new protocol::Principal(result->uuid, result->ownerUuid));
                            connectionManager.setPrincipal(principal);
                            //setting owner connection is moved to here(before onAuthenticated/before connected event)
                            //so that invocations that requires owner connection on this connection go through
                            connectionManager.setOwnerConnectionAddress(connection->getRemoteEndpoint());
                            connectionManager.logger.info("Setting ", *connection, " as owner with principal ",
                                                          *principal);
                        }
                        connectionManager.onAuthenticated(target, connection);
                        future->onSuccess(connection);
                        break;
                    }
                    case protocol::CREDENTIALS_FAILED: {
                        std::shared_ptr<protocol::Principal> p = connectionManager.principal;
                        if (p.get()) {
                            handleAuthenticationException(std::make_exception_ptr(
                                    (exception::ExceptionBuilder<exception::AuthenticationException>(
                                            "ConnectionManager::AuthCallback::onResponse")
                                            << "Invalid credentials! Principal: "
                                            << *p).build()));
                        } else {
                            handleAuthenticationException(std::make_exception_ptr(exception::AuthenticationException(
                                    "ConnectionManager::AuthCallback::onResponse",
                                    "Invalid credentials! No principal.")));
                        }
                        break;
                    }
                    default: {
                        handleAuthenticationException(std::make_exception_ptr(
                                (exception::ExceptionBuilder<exception::AuthenticationException>(
                                        "ConnectionManager::AuthCallback::onResponse")
                                        << "Authentication status code not supported. status: "
                                        << authenticationStatus).build()));
                    }
                }
            }

            void
            ClientConnectionManagerImpl::AuthCallback::onFailure(std::exception_ptr e) {
                cancelTimeoutTask();
                handleAuthenticationException(e);
            }

            void ClientConnectionManagerImpl::AuthCallback::handleAuthenticationException(
                    std::exception_ptr e) {
                this->onAuthenticationFailed(this->target, this->connection, e);
                this->future->onFailure(e);
            }

            void ClientConnectionManagerImpl::AuthCallback::onAuthenticationFailed(const Address &target,
                                                                                   const std::shared_ptr<Connection> &connection,
                                                                                   std::exception_ptr cause) {
                try {
                    std::rethrow_exception(cause);
                } catch (exception::IException &ie) {
                    if (connectionManager.logger.isFinestEnabled()) {
                        connectionManager.logger.finest("Authentication of ", connection, " failed.", ie);
                    }
                    connection->close("", std::current_exception());
                    connectionManager.connectionsInProgress.remove(target);
                }
            }

            ClientConnectionManagerImpl::AuthCallback::~AuthCallback() {
            }

            AuthenticationFuture::AuthenticationFuture(const Address &address,
                                                       util::SynchronizedMap<Address, FutureTuple> &connectionsInProgress)
                    : countDownLatch(new util::CountDownLatch(1)), address(address),
                      connectionsInProgress(connectionsInProgress), isSet(false) {
            }

            void AuthenticationFuture::onSuccess(const std::shared_ptr<Connection> &connection) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                this->connection = connection;
                countDownLatch->countDown();
            }

            void AuthenticationFuture::onFailure(std::exception_ptr throwable) {
                bool expected = false;
                if (!isSet.compare_exchange_strong(expected, true)) {
                    return;
                }
                connectionsInProgress.remove(address);
                this->throwable = throwable;
                countDownLatch->countDown();
            }

            std::shared_ptr<Connection> AuthenticationFuture::get() {
                countDownLatch->await();
                auto connPtr = connection.get();
                if (connPtr.get() != NULL) {
                    return connPtr;
                }

                auto exceptionPtr = throwable.get();
                assert(exceptionPtr);
                try {
                    std::rethrow_exception(exceptionPtr);
                } catch (exception::IException &ie) {
                    std::throw_with_nested(
                            exception::ExecutionException("AuthenticationFuture::get", "Could not be authenticated."));
                }
            }

            ReadHandler::ReadHandler(Connection &connection, size_t bufferSize)
                    : buffer(new char[bufferSize]), byteBuffer(buffer, bufferSize), builder(connection),
                      lastReadTime(std::chrono::steady_clock::now()) {
            }

            ReadHandler::~ReadHandler() {
                delete[] buffer;
            }

            void ReadHandler::handle() {
                lastReadTime = std::chrono::steady_clock::now();

                if (byteBuffer.position() == 0)
                    return;

                byteBuffer.flip();

                // it is important to check the onData return value since there may be left data less than a message
                // header size, and this may cause an infinite loop.
                while (byteBuffer.hasRemaining() && builder.onData(byteBuffer)) {
                }

                if (byteBuffer.hasRemaining()) {
                    byteBuffer.compact();
                } else {
                    byteBuffer.clear();
                }
            }

            const std::chrono::steady_clock::time_point ReadHandler::getLastReadTime() const {
                return lastReadTime;
            }

            Connection::Connection(const Address &address, spi::ClientContext &clientContext, int connectionId,
                                   const std::shared_ptr<AuthenticationFuture> &authFuture,
                                   internal::socket::SocketFactory &socketFactory, boost::asio::io_context &ioContext,
                                   bool asOwner,
                                   ClientConnectionManagerImpl &clientConnectionManager, int64_t connectTimeoutInMillis)
                    : readHandler(*this, 16 << 10),
                      startTimeInMillis(util::currentTimeMillis()), closedTimeMillis(0),
                      clientContext(clientContext),
                      invocationService(clientContext.getInvocationService()),
                      authFuture(authFuture),
                      connectionId(connectionId),
                      connectedServerVersion(impl::BuildInfo::UNKNOWN_HAZELCAST_VERSION),
                      logger(clientContext.getLogger()), asOwner(asOwner),
                      connectionManager(clientConnectionManager) {
                socket = socketFactory.create(address, connectTimeoutInMillis);
            }

            Connection::~Connection() {
            }

            void Connection::authenticate() {
                auto thisConnection = shared_from_this();
                connectionManager.authenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
            }

            void Connection::reAuthenticateAsOwner() {
                asOwner = true;
                auto thisConnection = shared_from_this();
                connectionManager.reAuthenticate(socket->getRemoteEndpoint(), thisConnection, asOwner, authFuture);
                authFuture->get();
            }

            void Connection::asyncStart() {
                socket->asyncStart(shared_from_this(), authFuture);
            }

            void Connection::close() {
                close("");
            }

            void Connection::close(const std::string &reason) {
                close(reason, nullptr);
            }

            void Connection::close(const std::string &reason, std::exception_ptr cause) {
                int64_t expected = 0;
                if (!closedTimeMillis.compare_exchange_strong(expected, util::currentTimeMillis())) {
                    return;
                }

                closeCause = cause;
                closeReason = reason;

                logClose();

                try {
                    innerClose();
                } catch (exception::IException &e) {
                    clientContext.getLogger().warning("Exception while closing connection", e.getMessage());
                }

                clientContext.getConnectionManager().onClose(*this);
            }

            void Connection::write(const std::shared_ptr<spi::impl::ClientInvocation> &clientInvocation) {
                socket->asyncWrite(shared_from_this(), clientInvocation);
            }

            const std::shared_ptr<Address> &Connection::getRemoteEndpoint() const {
                return remoteEndpoint;
            }

            void Connection::setRemoteEndpoint(const std::shared_ptr<Address> &remoteEndpoint) {
                this->remoteEndpoint = remoteEndpoint;
            }

            void Connection::handleClientMessage(const std::shared_ptr<protocol::ClientMessage> &message) {
                auto correlationId = message->getCorrelationId();
                auto invocationIterator = invocations.find(correlationId);
                if (invocationIterator == invocations.end()) {
                    logger.warning("No invocation' for callId: ", correlationId, ". Dropping this message: ", *message);
                    return;
                }
                auto invocation = invocationIterator->second;
                if (!invocation->getEventHandler()) {
                    // erase only for non-event messages
                    invocations.erase(invocationIterator);
                }
                if (message->isFlagSet(protocol::ClientMessage::LISTENER_EVENT_FLAG)) {
                    spi::impl::listener::AbstractClientListenerService &listenerService =
                            (spi::impl::listener::AbstractClientListenerService &) clientContext.getClientListenerService();
                    listenerService.handleClientMessage(invocation, message);
                } else {
                    invocationService.handleClientMessage(invocation, message);
                }
            }

            int Connection::getConnectionId() const {
                return connectionId;
            }

            bool Connection::isAlive() {
                return closedTimeMillis == 0;
            }

            const std::string &Connection::getCloseReason() const {
                return closeReason;
            }

            void Connection::logClose() {
                std::ostringstream message;
                message << *this << " closed. Reason: ";
                if (!closeReason.empty()) {
                    message << closeReason;
                } else if (closeCause) {
                    try {
                        std::rethrow_exception(closeCause);
                    } catch (exception::IException &ie) {
                        message << ie.getSource() << "[" + ie.getMessage() << "]";
                    }
                } else {
                    message << "Socket explicitly closed";
                }

                util::ILogger &logger = clientContext.getLogger();
                if (clientContext.getLifecycleService().isRunning()) {
                    if (!closeCause) {
                        logger.info(message.str());
                    } else {
                        try {
                            std::rethrow_exception(closeCause);
                        } catch (exception::IException &ie) {
                            logger.warning(message.str(), ie);
                        }
                    }
                } else {
                    if (logger.isFinestEnabled()) {
                        if (!closeCause) {
                            logger.finest(message.str());
                        } else {
                            try {
                                std::rethrow_exception(closeCause);
                            } catch (exception::IException &ie) {
                                logger.finest(message.str(), ie);
                            }
                        }
                    }
                }
            }

            bool Connection::isAuthenticatedAsOwner() {
                return authenticatedAsOwner;
            }

            void Connection::setIsAuthenticatedAsOwner() {
                authenticatedAsOwner.store(true);
            }

            bool Connection::operator==(const Connection &rhs) const {
                return connectionId == rhs.connectionId;
            }

            bool Connection::operator!=(const Connection &rhs) const {
                return !(rhs == *this);
            }

            const std::string &Connection::getConnectedServerVersionString() const {
                return connectedServerVersionString;
            }

            void Connection::setConnectedServerVersion(const std::string &connectedServerVersionString) {
                Connection::connectedServerVersionString = connectedServerVersionString;
                connectedServerVersion = impl::BuildInfo::calculateVersion(connectedServerVersionString);
            }

            int Connection::getConnectedServerVersion() const {
                return connectedServerVersion;
            }

            std::unique_ptr<Address> Connection::getLocalSocketAddress() const {
                return socket->localSocketAddress();
            }

            const std::chrono::steady_clock::time_point Connection::lastReadTime() {
                return readHandler.getLastReadTime();
            }

            void Connection::innerClose() {
                if (!socket.get()) {
                    return;;
                }

                socket->close();
            }

            std::ostream &operator<<(std::ostream &os, const Connection &connection) {
                Connection &conn = const_cast<Connection &>(connection);
                auto lastRead = conn.lastReadTime();
                int64_t closedTime = conn.closedTimeMillis;
                os << "ClientConnection{"
                   << "alive=" << conn.isAlive()
                   << ", connectionId=" << connection.getConnectionId()
                   << ", remoteEndpoint=";
                if (connection.getRemoteEndpoint().get()) {
                    os << *connection.getRemoteEndpoint();
                } else {
                    os << "null";
                }
                os << ", lastReadTime=" << util::StringUtil::timeToStringFriendly(lastRead.time_since_epoch().count())
                   << ", closedTime=" << util::StringUtil::timeToStringFriendly(closedTime)
                   << ", connected server version=" << conn.connectedServerVersionString
                   << '}';

                return os;
            }

            bool Connection::operator<(const Connection &rhs) const {
                return connectionId < rhs.connectionId;
            }

            int64_t Connection::getStartTimeInMillis() const {
                return startTimeInMillis;
            }

            const Socket &Connection::getSocket() const {
                return *socket;
            }

            void Connection::setAuthenticationInvocationFuture(future<void> authenticationInvocationFuture) {
                Connection::authenticationInvocationFuture = std::move(authenticationInvocationFuture);
            }

            ClientConnectionStrategy::ClientConnectionStrategy(spi::ClientContext &clientContext, util::ILogger &logger,
                                                               const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : clientContext(clientContext), logger(logger),
                      clientConnectionStrategyConfig(clientConnectionStrategyConfig) {
            }

            ClientConnectionStrategy::~ClientConnectionStrategy() {
            }

            HeartbeatManager::HeartbeatManager(spi::ClientContext &client) : client(client), clientConnectionManager(
                    client.getConnectionManager()), logger(client.getLogger()) {
                ClientProperties &clientProperties = client.getClientProperties();
                int timeoutSeconds = clientProperties.getInteger(clientProperties.getHeartbeatTimeout());
                heartbeatTimeoutSeconds = std::chrono::seconds(
                        timeoutSeconds > 0 ? timeoutSeconds : util::IOUtil::to_value<int>(
                                (std::string) ClientProperties::PROP_HEARTBEAT_TIMEOUT_DEFAULT));

                int intervalSeconds = clientProperties.getInteger(clientProperties.getHeartbeatInterval());
                heartbeatIntervalSeconds = std::chrono::seconds(
                        intervalSeconds > 0 ? intervalSeconds : util::IOUtil::to_value<int>(
                                (std::string) ClientProperties::PROP_HEARTBEAT_INTERVAL_DEFAULT));
            }

            void HeartbeatManager::start() {
                spi::impl::ClientExecutionServiceImpl &clientExecutionService = client.getClientExecutionService();

                clientExecutionService.scheduleWithRepetition([=]() {
                    if (!clientConnectionManager.isAlive()) {
                        return;
                    }

                    for (std::shared_ptr<Connection> connection : clientConnectionManager.getActiveConnections()) {
                        checkConnection(connection);
                    }
                }, heartbeatIntervalSeconds, heartbeatIntervalSeconds);
            }

            void HeartbeatManager::checkConnection(const std::shared_ptr<Connection> &connection) {
                if (!connection->isAlive()) {
                    return;
                }

                auto now = std::chrono::steady_clock::now();
                if (now - connection->lastReadTime() > heartbeatTimeoutSeconds) {
                    if (connection->isAlive()) {
                        logger.warning("Heartbeat failed over the connection: ", *connection);
                        onHeartbeatStopped(connection, "Heartbeat timed out");
                    }
                }

                if (now - connection->lastReadTime() > heartbeatIntervalSeconds) {
                    std::unique_ptr<protocol::ClientMessage> request = protocol::codec::ClientPingCodec::encodeRequest();
                    std::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, "", connection);
                    clientInvocation->invokeUrgent();
                }
            }

            void
            HeartbeatManager::onHeartbeatStopped(const std::shared_ptr<Connection> &connection,
                                                 const std::string &reason) {
                connection->close(reason.c_str(), std::make_exception_ptr(
                        (exception::ExceptionBuilder<exception::TargetDisconnectedException>(
                                "HeartbeatManager::onHeartbeatStopped") << "Heartbeat timed out to connection "
                                                                        << *connection).build()));
            }

            void HeartbeatManager::shutdown() {
            }

            DefaultClientConnectionStrategy::DefaultClientConnectionStrategy(spi::ClientContext &clientContext,
                                                                             util::ILogger &logger,
                                                                             const config::ClientConnectionStrategyConfig &clientConnectionStrategyConfig)
                    : ClientConnectionStrategy(clientContext, logger, clientConnectionStrategyConfig),
                      isShutdown(false) {
            }

            void DefaultClientConnectionStrategy::start() {
                clientStartAsync = clientConnectionStrategyConfig.isAsyncStart();
                reconnectMode = clientConnectionStrategyConfig.getReconnectMode();
                if (clientStartAsync) {
                    clientContext.getConnectionManager().connectToClusterAsync();
                } else {
                    clientContext.getConnectionManager().connectToCluster();
                }
            }

            void DefaultClientConnectionStrategy::beforeGetConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeGetConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (clientStartAsync && !disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is connecting to cluster.");
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline.");
                }
            }

            void DefaultClientConnectionStrategy::beforeOpenConnection(const Address &target) {
                checkShutdown("DefaultClientConnectionStrategy::beforeOpenConnection");

                if (isClusterAvailable()) {
                    return;
                }
                if (reconnectMode == config::ClientConnectionStrategyConfig::ASYNC && disconnectedFromCluster) {
                    throw exception::HazelcastClientOfflineException(
                            "DefaultClientConnectionStrategy::beforeGetConnection", "Client is offline");
                }
            }

            void DefaultClientConnectionStrategy::onConnectToCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onConnectToCluster");

                disconnectedFromCluster.store(false);
            }

            void DefaultClientConnectionStrategy::onDisconnectFromCluster() {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnectFromCluster");

                disconnectedFromCluster.store(true);
                if (reconnectMode == config::ClientConnectionStrategyConfig::OFF) {
                    shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    return;
                }
                if (clientContext.getLifecycleService().isRunning()) {
                    try {
                        clientContext.getConnectionManager().connectToClusterAsync();
                    } catch (exception::RejectedExecutionException &) {
                        shutdownWithExternalThread(clientContext.getHazelcastClientImplementation());
                    }
                }
            }

            void DefaultClientConnectionStrategy::onConnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onConnect");
            }

            void DefaultClientConnectionStrategy::onDisconnect(const std::shared_ptr<Connection> &connection) {
                checkShutdown("DefaultClientConnectionStrategy::onDisconnect");
            }

            void DefaultClientConnectionStrategy::shutdown() {
                isShutdown = true;
            }

            bool DefaultClientConnectionStrategy::isClusterAvailable() const {
                return clientContext.getConnectionManager().getOwnerConnectionAddress().get() != NULL;
            }

            void
            DefaultClientConnectionStrategy::shutdownWithExternalThread(
                    std::weak_ptr<client::impl::HazelcastClientInstanceImpl> clientImpl) {

                std::thread shutdownThread([=] {
                    std::shared_ptr<client::impl::HazelcastClientInstanceImpl> clientInstance = clientImpl.lock();
                    if (!clientInstance.get() || !clientInstance->getLifecycleService().isRunning()) {
                        return;
                    }

                    try {
                        clientInstance->getLifecycleService().shutdown();
                    } catch (exception::IException &exception) {
                        clientInstance->getLogger()->severe("Exception during client shutdown task ",
                                                            clientInstance->getName() + ".clientShutdown-", ":",
                                                            exception);
                    }
                });

                shutdownThread.detach();
            }

            void DefaultClientConnectionStrategy::checkShutdown(const std::string &methodName) {
                if (isShutdown) {
                    throw exception::IllegalStateException(methodName, "Client is shutdown.");
                }
            }

        }

        namespace internal {
            namespace socket {
                SocketFactory::SocketFactory(spi::ClientContext &clientContext, boost::asio::io_context &io)
                        : clientContext(clientContext), io(io) {
                }

                bool SocketFactory::start() {
#ifdef HZ_BUILD_WITH_SSL
                    const client::config::SSLConfig &sslConfig = clientContext.getClientConfig().getNetworkConfig().getSSLConfig();
                    if (sslConfig.isEnabled()) {
                        sslContext = std::unique_ptr<boost::asio::ssl::context>(new boost::asio::ssl::context(
                                (boost::asio::ssl::context_base::method) sslConfig.getProtocol()));

                        const std::vector<std::string> &verifyFiles = sslConfig.getVerifyFiles();
                        bool success = true;
                        util::ILogger &logger = clientContext.getLogger();
                        for (std::vector<std::string>::const_iterator it = verifyFiles.begin(); it != verifyFiles.end();
                             ++it) {
                            boost::system::error_code ec;
                            sslContext->load_verify_file(*it, ec);
                            if (ec) {
                                logger.warning(
                                        std::string("SocketFactory::start: Failed to load CA "
                                                    "verify file at ") + *it + " "
                                        + ec.message());
                                success = false;
                            }
                        }

                        if (!success) {
                            sslContext.reset();
                            logger.warning("SocketFactory::start: Failed to load one or more "
                                           "configured CA verify files (PEM files). Please "
                                           "correct the files and retry.");
                            return false;
                        }

                        // set cipher list if the list is set
                        const std::string &cipherList = sslConfig.getCipherList();
                        if (!cipherList.empty()) {
                            if (!SSL_CTX_set_cipher_list(sslContext->native_handle(), cipherList.c_str())) {
                                logger.warning(
                                        std::string("SocketFactory::start: Could not load any "
                                                    "of the ciphers in the config provided "
                                                    "ciphers:") + cipherList);
                                return false;
                            }
                        }

                    }
#else
                    (void) clientContext;
#endif

                    return true;
                }

                std::unique_ptr<Socket> SocketFactory::create(const Address &address, int64_t connectTimeoutInMillis) {
#ifdef HZ_BUILD_WITH_SSL
                    if (sslContext.get()) {
                        return std::unique_ptr<Socket>(new internal::socket::SSLSocket(io, *sslContext, address,
                                                                                       clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                       connectTimeoutInMillis));
                    }
#endif

                    return std::unique_ptr<Socket>(new internal::socket::TcpSocket(io, address,
                                                                                   clientContext.getClientConfig().getNetworkConfig().getSocketOptions(),
                                                                                   connectTimeoutInMillis));
                }

#ifdef HZ_BUILD_WITH_SSL

                SSLSocket::SSLSocket(boost::asio::io_context &ioService, boost::asio::ssl::context &sslContext,
                                     const client::Address &address, client::config::SocketOptions &socketOptions,
                                     int64_t connectTimeoutInMillis)
                        : BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
                        std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
                                new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(ioService, sslContext)),
                        address, socketOptions, ioService, connectTimeoutInMillis) {
                }

                std::vector<SSLSocket::CipherInfo> SSLSocket::getCiphers() const {
                    STACK_OF(SSL_CIPHER) *ciphers = SSL_get_ciphers(socket_->native_handle());
                    std::vector<CipherInfo> supportedCiphers;
                    for (int i = 0; i < sk_SSL_CIPHER_num(ciphers); ++i) {
                        struct SSLSocket::CipherInfo info;
                        SSL_CIPHER *cipher = const_cast<SSL_CIPHER *>(sk_SSL_CIPHER_value(ciphers, i));
                        info.name = SSL_CIPHER_get_name(cipher);
                        info.numberOfBits = SSL_CIPHER_get_bits(cipher, 0);
                        info.version = SSL_CIPHER_get_version(cipher);
                        char descBuf[256];
                        info.description = SSL_CIPHER_description(cipher, descBuf, 256);
                        supportedCiphers.push_back(info);
                    }
                    return supportedCiphers;
                }

                void SSLSocket::async_handle_connect(const std::shared_ptr<connection::Connection> connection,
                                                     const std::shared_ptr<connection::AuthenticationFuture> authFuture) {
                    socket_->async_handshake(boost::asio::ssl::stream_base::client,
                                             [=](const boost::system::error_code &ec) {
                                                 if (ec) {
                                                     authFuture->onFailure(
                                                             std::make_exception_ptr(exception::IOException(
                                                                     "Connection::do_connect", (boost::format(
                                                                             "Handshake with server %1% failed. %2%") %
                                                                                                remoteEndpoint %
                                                                                                ec).str())));
                                                     return;
                                                 }

                                                 BaseSocket<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>::async_handle_connect(
                                                         connection, authFuture);
                                             });
                }

                std::ostream &operator<<(std::ostream &out, const SSLSocket::CipherInfo &info) {
                    out << "Cipher{"
                           "Name: " << info.name <<
                        ", Bits:" << info.numberOfBits <<
                        ", Version:" << info.version <<
                        ", Description:" << info.description << "}";

                    return out;
                }

#endif // HZ_BUILD_WITH_SSL

                TcpSocket::TcpSocket(boost::asio::io_context &io, const Address &address,
                                     client::config::SocketOptions &socketOptions, int64_t connectTimeoutInMillis)
                        : BaseSocket<boost::asio::ip::tcp::socket>(
                        std::unique_ptr<boost::asio::ip::tcp::socket>(new boost::asio::ip::tcp::socket(io)),
                        address, socketOptions, io, connectTimeoutInMillis) {
                }

            }
        }
    }
}

