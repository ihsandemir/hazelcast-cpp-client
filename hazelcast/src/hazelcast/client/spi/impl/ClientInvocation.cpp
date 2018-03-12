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

#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/spi/LifecycleService.h>
#include <hazelcast/client/spi/ClusterService.h>
#include <hazelcast/client/spi/ClientInvocationService.h>

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                static int ClientInvocation::MAX_FAST_INVOCATION_COUNT = 5;
                static int ClientInvocation::UNASSIGNED_PARTITION = -1;

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   int partitionId) : logger(util::ILogger::getLogger()),
                                                                      lifecycleService(
                                                                              clientContext.getLifecycleService()),
                                                                      clientClusterService(
                                                                              clientContext.getClientClusterService()),
                                                                      invocationService(
                                                                              clientContext.getInvocationService()),
                                                                      clientMessage(clientMessage),
                                                                      callIdSequence(clientContext.getCallIdSequence()),
                                                                      partitionId(partitionId),
                                                                      startTimeMillis(util::currentTimeMillis()),
                                                                      objectName(objectName),
                                                                      clientInvocationFuture(logger, *this,
                                                                                             callIdSequence,
                                                                                             clientContext.getClientExecutionService()) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   boost::shared_ptr<connection::Connection> &connection) : logger(
                        util::ILogger::getLogger()),
                                                                                                            lifecycleService(
                                                                                                                    clientContext.getLifecycleService()),
                                                                                                            clientClusterService(
                                                                                                                    clientContext.getClientClusterService()),
                                                                                                            invocationService(
                                                                                                                    clientContext.getInvocationService()),
                                                                                                            clientMessage(
                                                                                                                    clientMessage),
                                                                                                            callIdSequence(
                                                                                                                    clientContext.getCallIdSequence()),
                                                                                                            partitionId(
                                                                                                                    UNASSIGNED_PARTITION),
                                                                                                            startTimeMillis(
                                                                                                                    util::currentTimeMillis()),
                                                                                                            objectName(
                                                                                                                    objectName),
                                                                                                            clientInvocationFuture(
                                                                                                                    logger,
                                                                                                                    *this,
                                                                                                                    callIdSequence,
                                                                                                                    clientContext.getClientExecutionService()),
                                                                                                            connection(connection) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName) : logger(util::ILogger::getLogger()),
                                                                                    lifecycleService(
                                                                                            clientContext.getLifecycleService()),
                                                                                    clientClusterService(
                                                                                            clientContext.getClientClusterService()),
                                                                                    invocationService(
                                                                                            clientContext.getInvocationService()),
                                                                                    clientMessage(clientMessage),
                                                                                    callIdSequence(clientContext.getCallIdSequence()),
                                                                                    partitionId(UNASSIGNED_PARTITION),
                                                                                    startTimeMillis(util::currentTimeMillis()),
                                                                                    objectName(objectName),
                                                                                    clientInvocationFuture(logger,
                                                                                                           *this,
                                                                                                           callIdSequence,
                                                                                                           <#initializer#>) {

                }

                ClientInvocationFuture &ClientInvocation::invoke(boost::shared_ptr<ClientInvocation> &invocation) {
                    assert (invocation->clientMessage.get() != NULL);
                    invocation->clientMessage->setCorrelationId(invocation->callIdSequence.next());
                    invocation->invokeOnSelection(invocation);
                    return invocation->clientInvocationFuture;
                }

                ClientInvocationFuture &
                ClientInvocation::invokeUrgent(boost::shared_ptr<ClientInvocation> &invocation) {
                    assert (invocation->clientMessage.get() != NULL);
                    invocation->clientMessage->setCorrelationId(invocation->callIdSequence.forceNext());
                    invocation->invokeOnSelection(invocation);
                    return invocation->clientInvocationFuture;
                }

                void ClientInvocation::invokeOnSelection(boost::shared_ptr<ClientInvocation> &invocation) {
                    try {
                        if (invocation->isBindToSingleConnection()) {
                            invocation->invocationService.invokeOnConnection(invocation, invocation->connection);
                        } else if (invocation->partitionId != -1) {
                            invocation->invocationService.invokeOnPartitionOwner(invocation, invocation->partitionId);
                        } else if (invocation->address.get() != NULL) {
                            invocation->invocationService.invokeOnTarget(invocation, *invocation->address);
                        } else {
                            invocation->invocationService.invokeOnRandomTarget(invocation);
                        }
                    } catch (exception::HazelcastOverloadException &e) {
                        throw;
                    } catch (exception::IException &e) {
                        invocation->notifyException(e);
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection.get() != NULL;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    clientMessage->setCorrelationId(callIdSequence.forceNext());
                    //we release the old slot
                    callIdSequence.complete();

                    try {
                        invokeOnSelection(<#initializer#>);
                    } catch (exception::IException &e) {
                        clientInvocationFuture.complete(e);
                    }
                }

                void ClientInvocation::notifyException(exception::IException &exception) {
                    if (!lifecycleService.isRunning()) {
                        clientInvocationFuture.complete(exception::HazelcastClientNotActiveException(
                                exception.getSource(), exception.getMessage()));
                        return;
                    }

                    if (isNotAllowedToRetryOnSelection(exception)) {
                        clientInvocationFuture.complete(exception);
                        return;
                    }

                    bool retry = isRetrySafeException(exception)
                                 || invocationService.isRedoOperation()
                                 || isDisconnectedRetryable(exception, *clientMessage);

                    if (!retry) {
                        clientInvocationFuture.complete(exception);
                        return;
                    }

                    int64_t timePassed = util::currentTimeMillis() - startTimeMillis;
                    if (timePassed > invocationService.getInvocationTimeoutMillis()) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Exception will not be retried because invocation timed out. " << exception.what();
                            logger.finest(out.str());
                        }

                        clientInvocationFuture.complete(newOperationTimeoutException(exception));
                        return;
                    }

                    // TODO: Change this to use an executor as done at Java
                    run();
                }

                bool ClientInvocation::isNotAllowedToRetryOnSelection(exception::IException &exception) {
                    if (isBindToSingleConnection()) {
                        try {
                            throw exception;
                        } catch (exception::IOException &e) {
                            return true;
                        } catch (...) {
                        }
                    }

                    if (address.get() != NULL) {
                        try {
                            throw exception;
                        } catch (exception::TargetNotMemberException &e) {
                            if (!clientClusterService.isMemberExists(*address)) {
                                //when invocation send over address
                                //if exception is target not member and
                                //address is not available in member list , don't retry
                                return true;
                            }
                        } catch (...) {
                        }
                    }

                    return false;
                }

                bool ClientInvocation::isRetrySafeException(exception::IException &exception) {
                    try {
                        throw exception;
                    } catch (exception::IOException &) {
                        return true;
                    } catch (exception::HazelcastInstanceNotActiveException &) {
                        return true;
                    } catch (exception::RetryableIOException &) {
                        return true;
                    } catch (...) {
                        return false;
                    }
                }

                bool ClientInvocation::isDisconnectedRetryable(exception::IException &exception,
                                                               protocol::ClientMessage &message) {
                    try {
                        throw exception;
                    } catch (exception::TargetDisconnectedException &) {
                        if (message.isRetryable()) {
                            return true;
                        }
                    } catch (...) {
                    }
                    return false;
                }

                exception::IException
                ClientInvocation::newOperationTimeoutException(exception::IException &exception) {
                    std::ostringstream sb;
                    int64_t nowInMillis = util::currentTimeMillis();
                    sb << *this << " timed out because exception occurred after client invocation timeout "
                       << invocationService.getInvocationTimeoutMillis() << util::StringUtil::timeToString(nowInMillis)
                       << ". " << "Start time: " << util::StringUtil::timeToString(startTimeMillis)
                       << ". Total elapsed time: "
                       << (nowInMillis - startTimeMillis) << " ms. ";
                    return exception::OperationTimeoutException("ClientInvocation::newOperationTimeoutException",
                                                                sb.str());
                }

                std::ostream &impl::operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.isBindToSingleConnection()) {
                        target << "connection " << invocation.connection;
                    } else if (invocation.partitionId != -1) {
                        target << "partition " << invocation.partitionId;
                    } else if (invocation.address.get() != NULL) {
                        target << "address " << *invocation.address;
                    } else {
                        target << "random";
                    }

                    boost::shared_ptr<connection::Connection> sendConnection = invocation.sendConnection;

                    os << "ClientInvocation{"
                       << "clientMessage = " << *invocation.clientMessage
                       << ", objectName = " << invocation.objectName
                       << ", target = " << target
                       << ", sendConnection = " << *sendConnection << '}';

                    return os;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             int partitionId) {
                    return boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, partitionId));
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             boost::shared_ptr<connection::Connection> &connection) {
                    return boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName, connection));
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName) {
                    return boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext, clientMessage, objectName));
                }

                const util::Atomic<boost::shared_ptr<connection::Connection>> &
                ClientInvocation::getSendConnection() const {
                    return sendConnection;
                }

                void
                ClientInvocation::setSendConnection(const boost::shared_ptr<connection::Connection> &sendConnection) {
                    ClientInvocation::sendConnection = sendConnection;
                }

                void ClientInvocation::notify(const boost::shared_ptr<protocol::ClientMessage> &clientMessage) {
                    if (clientMessage.get() == NULL) {
                        throw exception::IllegalArgumentException("response can't be null");
                    }
                    clientInvocationFuture.complete(clientMessage);
                }

                const boost::shared_ptr<protocol::ClientMessage> &ClientInvocation::getClientMessage() const {
                    return clientMessage;
                }

                bool ClientInvocation::shouldBypassHeartbeatCheck() const {
                    return bypassHeartbeatCheck;
                }

                void ClientInvocation::setBypassHeartbeatCheck(bool bypassHeartbeatCheck) {
                    ClientInvocation::bypassHeartbeatCheck = bypassHeartbeatCheck;
                }

                const boost::shared_ptr<EventHandler> &ClientInvocation::getEventHandler() const {
                    return eventHandler;
                }

                void ClientInvocation::setEventHandler(const boost::shared_ptr<EventHandler> &eventHandler) {
                    ClientInvocation::eventHandler = eventHandler;
                }

            }
        }
    }
}
