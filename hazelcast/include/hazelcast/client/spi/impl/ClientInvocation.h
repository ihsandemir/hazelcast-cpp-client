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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_

#include <ostream>
#include <boost/shared_ptr.hpp>

#include <hazelcast/util/Atomic.h>
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"
#include "hazelcast/client/spi/EventHandler.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }

    namespace client {
        class Address;

        namespace connection {
            class Connection;
        }

        namespace protocol {
            class ClientMessage;
        }

        namespace spi {
            class LifecycleService;
            class ClusterService;
            class ClientInvocationService;
            class ClientContext;

            namespace impl {
                /**
                 * Handles the routing of a request from a Hazelcast client.
                 * <p>
                 * 1) Where should request be sent?<br>
                 * 2) Should it be retried?<br>
                 * 3) How many times is it retried?
                 */
                class HAZELCAST_API ClientInvocation {
                public:

                    static boost::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName, int partitionId);


                    static boost::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName, boost::shared_ptr<connection::Connection> &connection);


                    static boost::shared_ptr<ClientInvocation> create(spi::ClientContext &clientContext,
                                                                      const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                                                      const std::string &objectName);

                    static ClientInvocationFuture &invoke(boost::shared_ptr<ClientInvocation> &invocation);

                    static ClientInvocationFuture &invokeUrgent(boost::shared_ptr<ClientInvocation> &invocation);

                    void run();

                    void notify(const boost::shared_ptr<protocol::ClientMessage> &clientMessage);

                    void notifyException(exception::IException &exception);

                    const util::Atomic<boost::shared_ptr<connection::Connection> > &getSendConnection() const;

                    void
                    setSendConnection(const boost::shared_ptr<connection::Connection> &sendConnection);

                    const boost::shared_ptr<protocol::ClientMessage> &getClientMessage() const;

                    bool shouldBypassHeartbeatCheck() const;

                    void setBypassHeartbeatCheck(bool bypassHeartbeatCheck);

                    const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &getEventHandler() const;

                    void setEventHandler(const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &eventHandler);

                    friend std::ostream &operator<<(std::ostream &os, const ClientInvocation &invocation);

                private:
                    ClientInvocation(spi::ClientContext &clientContext,
                                     const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName, int partitionId);

                    ClientInvocation(spi::ClientContext &clientContext,
                                     const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName, boost::shared_ptr<connection::Connection> &connection);

                    ClientInvocation(spi::ClientContext &clientContext,
                                     const std::auto_ptr<protocol::ClientMessage> &clientMessage,
                                     const std::string &objectName);

                    static void invokeOnSelection(boost::shared_ptr<ClientInvocation> &invocation);

                    bool isBindToSingleConnection() const;

                    void retry();

                    static int MAX_FAST_INVOCATION_COUNT;
                    static int UNASSIGNED_PARTITION;

                    ClientInvocationFuture clientInvocationFuture;
                    util::ILogger &logger;
                    LifecycleService &lifecycleService;
                    ClusterService &clientClusterService;
                    ClientInvocationService &invocationService;
/*
                ClientExecutionService executionService;
*/
                    boost::shared_ptr<protocol::ClientMessage> clientMessage;
                    sequence::CallIdSequence &callIdSequence;
                    std::auto_ptr<Address> address;
                    int partitionId;
                    boost::shared_ptr<connection::Connection> connection;
                    int64_t startTimeMillis;
                    int64_t retryPauseMillis;
                    std::string objectName;
                    util::Atomic<boost::shared_ptr<connection::Connection> > sendConnection;
                    bool bypassHeartbeatCheck;
                    boost::shared_ptr<EventHandler<protocol::ClientMessage> > eventHandler;
                    util::Atomic<int64_t> invokeCount;

                    bool isNotAllowedToRetryOnSelection(exception::IException &exception);

                    bool isRetrySafeException(exception::IException &exception);

                    bool isDisconnectedRetryable(exception::IException &exception, protocol::ClientMessage &message);

                    exception::IException newOperationTimeoutException(exception::IException &exception);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTINVOCATION_H_
