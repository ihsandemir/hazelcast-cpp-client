/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"

#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/spi/ClientInvocationService.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"
#include "hazelcast/client/spi/impl/sequence/CallIdSequence.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   int partitionId) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(boost::shared_ptr<protocol::ClientMessage>(clientMessage)),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(partitionId),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), clientMessage,
                                                           clientContext.getCallIdSequence())),
                        backupsAcksExpected(-1) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName,
                                                   const boost::shared_ptr<connection::Connection> &connection) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(boost::shared_ptr<protocol::ClientMessage>(clientMessage)),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        connection(connection),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), clientMessage,
                                                           clientContext.getCallIdSequence())),
                        backupsAcksExpected(-1) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(boost::shared_ptr<protocol::ClientMessage>(clientMessage)),
                        callIdSequence(clientContext.getCallIdSequence()),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), clientMessage,
                                                           clientContext.getCallIdSequence())),
                        backupsAcksExpected(-1) {
                }

                ClientInvocation::ClientInvocation(spi::ClientContext &clientContext,
                                                   const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                   const std::string &objectName, const Address &address) :
                        logger(clientContext.getLogger()),
                        lifecycleService(clientContext.getLifecycleService()),
                        clientClusterService(clientContext.getClientClusterService()),
                        invocationService(clientContext.getInvocationService()),
                        executionService(clientContext.getClientExecutionService().shared_from_this()),
                        clientMessage(boost::shared_ptr<protocol::ClientMessage>(clientMessage)),
                        callIdSequence(clientContext.getCallIdSequence()),
                        address(new Address(address)),
                        partitionId(UNASSIGNED_PARTITION),
                        startTimeMillis(util::currentTimeMillis()),
                        retryPauseMillis(invocationService.getInvocationRetryPauseMillis()),
                        objectName(objectName),
                        invokeCount(0),
                        clientInvocationFuture(
                                new ClientInvocationFuture(clientContext.getClientExecutionService().shared_from_this(),
                                                           clientContext.getLogger(), clientMessage,
                                                           clientContext.getCallIdSequence())),
                        backupsAcksExpected(-1) {
                }

                ClientInvocation::~ClientInvocation() {
                }

                boost::shared_ptr<ClientInvocationFuture> ClientInvocation::invoke() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->next());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                boost::shared_ptr<ClientInvocationFuture> ClientInvocation::invokeUrgent() {
                    assert (clientMessage.get() != NULL);
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    invokeOnSelection(shared_from_this());
                    return clientInvocationFuture;
                }

                void ClientInvocation::invokeOnSelection(const boost::shared_ptr<ClientInvocation> &invocation) {
                    invocation->invokeCount++;
                    try {
                        if (invocation->isBindToSingleConnection()) {
                            invocation->invocationService.invokeOnConnection(invocation, invocation->connection);
                        } else if (invocation->partitionId != UNASSIGNED_PARTITION) {
                            invocation->invocationService.invokeOnPartitionOwner(invocation, invocation->partitionId);
                        } else if (invocation->address.get() != NULL) {
                            invocation->invocationService.invokeOnTarget(invocation, invocation->address);
                        } else {
                            invocation->invocationService.invokeOnRandomTarget(invocation);
                        }
                    } catch (exception::HazelcastOverloadException &) {
                        throw;
                    } catch (exception::IException &e) {
                        invocation->notifyException(boost::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                bool ClientInvocation::isBindToSingleConnection() const {
                    return connection.get() != NULL;
                }

                void ClientInvocation::run() {
                    retry();
                }

                void ClientInvocation::retry() {
                    // retry modifies the client message and should not reuse the client message.
                    // It could be the case that it is in write queue of the connection.
                    clientMessage = copyMessage();
                    // first we force a new invocation slot because we are going to return our old invocation slot immediately after
                    // It is important that we first 'force' taking a new slot; otherwise it could be that a sneaky invocation gets
                    // through that takes our slot!
                    clientMessage.get()->setCorrelationId(callIdSequence->forceNext());
                    //we release the old slot
                    callIdSequence->complete();

                    try {
                        invokeOnSelection(shared_from_this());
                    } catch (exception::IException &e) {
                        complete(boost::shared_ptr<exception::IException>(e.clone()));
                    }
                }

                void ClientInvocation::notifyException(const boost::shared_ptr<exception::IException> &exception) {
                    if (!lifecycleService.isRunning()) {
                        boost::shared_ptr<exception::IException> notActiveException(
                                new exception::HazelcastClientNotActiveException(exception->getSource(),
                                                                                 "Client is shutting down", exception));

                        clientInvocationFuture->complete(notActiveException);
                        return;
                    }

                    if (isNotAllowedToRetryOnSelection(*exception)) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    bool retry = isRetrySafeException(*exception)
                                 || invocationService.isRedoOperation()
                                 || (exception->getErrorCode() == protocol::TARGET_DISCONNECTED &&
                                     clientMessage.get()->isRetryable());

                    if (!retry) {
                        clientInvocationFuture->complete(exception);
                        return;
                    }

                    int64_t timePassed = util::currentTimeMillis() - startTimeMillis;
                    if (timePassed > invocationService.getInvocationTimeoutMillis()) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Exception will not be retried because invocation timed out. " << exception->what();
                            logger.finest(out.str());
                        }

                        clientInvocationFuture->complete(newOperationTimeoutException(*exception));
                        return;
                    }

                    try {
                        execute();
                    } catch (exception::RejectedExecutionException &) {
                        clientInvocationFuture->complete(exception);
                    }

                }

                bool ClientInvocation::isNotAllowedToRetryOnSelection(exception::IException &exception) {
                    if (isBindToSingleConnection() && exception.getErrorCode() == protocol::IO) {
                        return true;
                    }

                    if (address.get() != NULL && exception.getErrorCode() == protocol::TARGET_NOT_MEMBER &&
                        clientClusterService.getMember(*address).get() == NULL) {
                        //when invocation send over address
                        //if exception is target not member and
                        //address is not available in member list , don't retry
                        return true;
                    }
                    return false;
                }

                bool ClientInvocation::isRetrySafeException(exception::IException &exception) {
                    int32_t errorCode = exception.getErrorCode();
                    if (errorCode == protocol::IO || errorCode == protocol::HAZELCAST_INSTANCE_NOT_ACTIVE ||
                        exception.isRetryable()) {
                        return true;
                    }

                    return false;
                }

                boost::shared_ptr<exception::OperationTimeoutException>
                ClientInvocation::newOperationTimeoutException(exception::IException &exception) {
                    int64_t nowInMillis = util::currentTimeMillis();

                    return (exception::ExceptionBuilder<exception::OperationTimeoutException>(
                            "ClientInvocation::newOperationTimeoutException") << *this
                                                                              << " timed out because exception occurred after client invocation timeout "
                                                                              << "Current time :"
                                                                              << invocationService.getInvocationTimeoutMillis()
                                                                              << util::StringUtil::timeToString(
                                                                                      nowInMillis) << ". "
                                                                              << "Start time: "
                                                                              << util::StringUtil::timeToString(
                                                                                      startTimeMillis)
                                                                              << ". Total elapsed time: "
                                                                              << (nowInMillis - startTimeMillis)
                                                                              << " ms. ").buildShared();
                }

                std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation) {
                    std::ostringstream target;
                    if (invocation.isBindToSingleConnection()) {
                        target << "connection " << *invocation.connection;
                    } else if (invocation.partitionId != -1) {
                        target << "partition " << invocation.partitionId;
                    } else if (invocation.address.get() != NULL) {
                        target << "address " << *invocation.address;
                    } else {
                        target << "random";
                    }
                    ClientInvocation &nonConstInvocation = const_cast<ClientInvocation &>(invocation);
                    os << "ClientInvocation{" << "clientMessage = " << *nonConstInvocation.clientMessage.get()
                       << ", objectName = "
                       << invocation.objectName << ", target = " << target.str() << ", sendConnection = ";
                    boost::shared_ptr<connection::Connection> sendConnection = nonConstInvocation.sendConnection.get();
                    if (sendConnection.get()) {
                        os << *sendConnection;
                    } else {
                        os << "null";
                    }
                    os << '}';

                    return os;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             int partitionId) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext,
                                                 boost::shared_ptr<protocol::ClientMessage>(clientMessage), objectName,
                                                 partitionId));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             const boost::shared_ptr<connection::Connection> &connection) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext,
                                                 boost::shared_ptr<protocol::ClientMessage>(clientMessage), objectName,
                                                 connection));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }


                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName,
                                                                             const Address &address) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext,
                                                 boost::shared_ptr<protocol::ClientMessage>(clientMessage), objectName,
                                                 address));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<ClientInvocation> ClientInvocation::create(spi::ClientContext &clientContext,
                                                                             std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                             const std::string &objectName) {
                    boost::shared_ptr<ClientInvocation> invocation = boost::shared_ptr<ClientInvocation>(
                            new ClientInvocation(clientContext,
                                                 boost::shared_ptr<protocol::ClientMessage>(clientMessage),
                                                 objectName));
                    invocation->clientInvocationFuture->setInvocation(invocation);
                    return invocation;
                }

                boost::shared_ptr<connection::Connection> ClientInvocation::getSendConnection() {
                    return sendConnection;
                }

                boost::shared_ptr<connection::Connection> ClientInvocation::getSendConnectionOrWait() {
                    while (sendConnection.get().get() == NULL && !clientInvocationFuture->isDone()) {
                        // TODO: Make sleep interruptible
                        util::sleepmillis(retryPauseMillis);
                    }
                    return sendConnection;
                }

                void
                ClientInvocation::setSendConnection(const boost::shared_ptr<connection::Connection> &sendConnection) {
                    ClientInvocation::sendConnection = sendConnection;
                }

                void ClientInvocation::notify(const boost::shared_ptr<protocol::ClientMessage> &message) {
                    assert(message.get() != NULL);

                    int expectedBackups = message->getNumberOfBackupAcks();

                    // if a regular response comes and there are backups, we need to wait for the backups
                    // when the backups complete, the response will be send by the last backup or backup-timeout-handle mechanism kicks on

                    if (expectedBackups > backupsAcksReceived) {
                        // so the invocation has backups and since not all backups have completed, we need to wait
                        // (it could be that backups arrive earlier than the response)

                        this->pendingResponseReceivedMillis = util::currentTimeMillis();

                        this->backupsAcksExpected.set(expectedBackups);

                        // it is very important that the response is set after the backupsAcksExpected is set, else the system
                        // can assume the invocation is complete because there is a response and no backups need to respond
                        this->pendingResponseMessage = message;

                        if (backupsAcksReceived != expectedBackups) {
                            // we are done since not all backups have completed. Therefor we should not notify the future
                            return;
                        }
                    }

                    // we are going to notify the future that a response is available; this can happen when:
                    // - we had a regular operation (so no backups we need to wait for) that completed
                    // - we had a backup-aware operation that has completed, but also all its backups have completed
                    complete(message);
                }

                const boost::shared_ptr<protocol::ClientMessage> ClientInvocation::getClientMessage() {
                    return clientMessage.get();
                }

                const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &
                ClientInvocation::getEventHandler() const {
                    return eventHandler;
                }

                void ClientInvocation::setEventHandler(
                        const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler) {
                    ClientInvocation::eventHandler = eventHandler;
                }

                void ClientInvocation::execute() {
                    if (invokeCount < MAX_FAST_INVOCATION_COUNT) {
                        // fast retry for the first few invocations
                        executionService->execute(boost::shared_ptr<util::Runnable>(shared_from_this()));
                    } else {
                        // progressive retry delay
                        int64_t delayMillis = util::min<int64_t>(1 << (invokeCount.get() - MAX_FAST_INVOCATION_COUNT),
                                                                 retryPauseMillis);
                        executionService->schedule(shared_from_this(), delayMillis);
                    }
                }

                const std::string ClientInvocation::getName() const {
                    return "ClientInvocation";
                }

                boost::shared_ptr<protocol::ClientMessage> ClientInvocation::copyMessage() {
                    return boost::shared_ptr<protocol::ClientMessage>(
                            new protocol::ClientMessage(*clientMessage.get()));
                }

                boost::shared_ptr<util::Executor> ClientInvocation::getUserExecutor() {
                    return executionService->getUserExecutor();
                }

                void ClientInvocation::notifyBackup() {
                    int newBackupAcksCompleted = ++backupsAcksReceived;

                    boost::shared_ptr<protocol::ClientMessage> pendingResponse = this->pendingResponseMessage;
                    if (pendingResponse.get() == NULL) {
                        // no pendingResponse has been set, so we are done since the invocation on the primary needs to complete first
                        return;
                    }

                    // if a pendingResponse is set, then the backupsAcksExpected has been set (so we can now safely read backupsAcksExpected)
                    int backupAcksExpected = this->backupsAcksExpected;
                    if (backupAcksExpected < newBackupAcksCompleted) {
                        // the backups have not yet completed, so we are done
                        return;
                    }

                    if (backupAcksExpected != newBackupAcksCompleted) {
                        // we managed to complete one backup, but we were not the one completing the last backup, so we are done
                        return;
                    }

                    // we are the lucky one since we just managed to complete the last backup for this invocation and since the
                    // pendingResponse is set, we can set it on the future
                    complete(pendingResponse);
                }

                void ClientInvocation::complete(const boost::shared_ptr<protocol::ClientMessage> &response) {
                    clientInvocationFuture->complete(response);
                    AbstractClientInvocationService &abstractInvocationService = (AbstractClientInvocationService &) invocationService;

                    abstractInvocationService.deRegisterInvocation(clientMessage.get()->getCorrelationId());
                }

                void ClientInvocation::complete(const boost::shared_ptr<exception::IException> &exception) {
                    clientInvocationFuture->complete(exception);
                    AbstractClientInvocationService &abstractInvocationService = (AbstractClientInvocationService &) invocationService;

                    abstractInvocationService.deRegisterInvocation(clientMessage.get()->getCorrelationId());
                }

                bool ClientInvocation::detectAndHandleBackupTimeout(int64_t timeoutMillis) {
                    // if the backups have completed, we are done; this also filters out all non backup-aware operations
                    // since the backupsAcksExpected will always be equal to the backupsAcksReceived
                    bool backupsCompleted = backupsAcksExpected.get() == backupsAcksReceived.get();
                    int64_t responseReceivedMillis = pendingResponseReceivedMillis;

                    // if this has not yet expired (so has not been in the system for a too long period) we ignore it
                    int64_t expirationTime = responseReceivedMillis + timeoutMillis;
                    bool timeout = expirationTime > 0 && expirationTime < util::currentTimeMillis();

                    // if no response has yet been received, we we are done; we are only going to re-invoke an operation
                    // if the response of the primary has been received, but the backups have not replied
                    bool responseReceived = pendingResponseMessage.get().get() != NULL;

                    if (backupsCompleted || !responseReceived || !timeout) {
                        return false;
                    }

                    if (shouldFailOnIndeterminateOperationState()) {
                        boost::shared_ptr<IndeterminateOperationStateException> stateException = (
                                exception::ExceptionBuilder<exception::IndeterminateOperationStateException>(
                                        "ClientInvocation::detectAndHandleBackupTimeout") << *this
                                                                                          << " failed because backup acks missed.").buildShared();
                        complete(stateException);
                        return true;
                    }

                    // the backups have not yet completed, but we are going to release the future anyway if a pendingResponse has been set
                    complete(pendingResponseMessage);
                    return true;
                }

                bool ClientInvocation::shouldFailOnIndeterminateOperationState() {
                    return ((spi::impl::AbstractClientInvocationService &) invocationService).isShouldFailOnIndeterminateOperationState();
                }

            }
        }
    }
}
