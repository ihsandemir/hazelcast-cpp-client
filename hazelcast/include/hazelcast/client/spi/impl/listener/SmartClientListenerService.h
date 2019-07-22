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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_

#include "hazelcast/client/spi/impl/listener/AbstractClientListenerService.h"
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/client/spi/impl/ListenerMessageCodec.h"
#include "hazelcast/client/protocol/codec/ClientLocalBackupListenerCodec.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }

        namespace spi {
            namespace impl {
                namespace listener {
                    class SmartClientListenerService : public AbstractClientListenerService {
                    public:
                        SmartClientListenerService(ClientContext &clientContext, int32_t eventThreadCount,
                                                   int32_t eventQueueCapacity);

                        virtual void start();

                        virtual std::string
                        registerListener(const boost::shared_ptr<impl::ListenerMessageCodec> &listenerMessageCodec,
                                         const boost::shared_ptr<EventHandler<protocol::ClientMessage> > &handler);

                        void asyncConnectToAllMembersInternal();

                    protected:

                        virtual bool registersLocalOnly() const;

                    private:
                        class BackupListenerMessageCodec : public ListenerMessageCodec {
                        public:
                            virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                            virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                            virtual std::auto_ptr<protocol::ClientMessage>
                            encodeRemoveRequest(const std::string &realRegistrationId) const;

                            virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;
                        };

                        class BackupEventHandler
                                : public protocol::codec::ClientLocalBackupListenerCodec::AbstractEventHandler {
                        public:
                            BackupEventHandler(
                                    const boost::shared_ptr<AbstractClientInvocationService> &invocationService,
                                    util::ILogger &logger);

                        public:
                            virtual void handleBackupEventV19(const int64_t &backupId);

                        private:
                            boost::weak_ptr<spi::impl::AbstractClientInvocationService> invocationService;
                            util::ILogger &logger;
                        };

                        class AsyncConnectToAllMembersTask : public util::Runnable {
                        public:
                            AsyncConnectToAllMembersTask(
                                    const boost::shared_ptr<SmartClientListenerService> &listenerService);

                            virtual void run();

                            virtual const std::string getName() const;

                        private:
                            boost::shared_ptr<SmartClientListenerService> listenerService;
                        };

                        void trySyncConnectToAllMembers();

                        void timeOutOrSleepBeforeNextTry(int64_t startMillis, const Member &lastFailedMember,
                                                         boost::shared_ptr<exception::IException> &lastException);

                        void
                        throwOperationTimeoutException(int64_t startMillis, int64_t nowInMillis, int64_t elapsedMillis,
                                                       const Member &lastFailedMember,
                                                       boost::shared_ptr<exception::IException> &lastException);

                        void sleepBeforeNextTry();

                        void addBackupListener();

                        static const boost::shared_ptr<impl::ListenerMessageCodec> backupListener;
                    };
                }
            }
        }
    }
}

#endif // HAZELCAST_CLIENT_SPI_IMPL_LISTENER_SMARTCLIENTLISTERNERSERVICE_H_
