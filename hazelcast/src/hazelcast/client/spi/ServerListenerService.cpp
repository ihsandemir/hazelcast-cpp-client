/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 6/24/13.



#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/InvocationService.h"

#include "hazelcast/client/protocol/parameters/AddListenerResultParameters.h"

namespace hazelcast {
    namespace client {
        namespace spi {

            ServerListenerService::ServerListenerService(spi::ClientContext& clientContext)
            : clientContext(clientContext) {

            }

            std::auto_ptr<std::string> ServerListenerService::listen(std::auto_ptr<protocol::ClientMessage> registrationRequest,
                                                      int partitionId, impl::BaseEventHandler *handler) {

                connection::CallFuture future = clientContext.getInvocationService().invokeOnPartitionOwner(registrationRequest, handler, partitionId);

                std::auto_ptr<protocol::ClientMessage> response = future.get();

                // get the correlationId for the request
                int correlationId = future.getCallId();

                std::auto_ptr<protocol::parameters::AddListenerResultParameters> resultParams = protocol::parameters::AddListenerResultParameters::decode(*response);

                registerListener(*resultParams->registrationId, correlationId);

                return resultParams->registrationId;
            }

            std::auto_ptr<std::string> ServerListenerService::listen(std::auto_ptr<protocol::ClientMessage> registrationRequest, impl::BaseEventHandler *handler) {
                connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(registrationRequest, handler);

                std::auto_ptr<protocol::ClientMessage> response = future.get();

                // get the correlationId for the request
                int correlationId = future.getCallId();

                std::auto_ptr<protocol::parameters::AddListenerResultParameters> resultParams =
                        protocol::parameters::AddListenerResultParameters::decode(*response);
                
                registerListener(*resultParams->registrationId, correlationId);

                return resultParams->registrationId;
            }

            bool ServerListenerService::stopListening(std::auto_ptr<protocol::ClientMessage> request) {
                connection::CallFuture future = clientContext.getInvocationService().invokeOnRandomTarget(request);

                std::auto_ptr<protocol::ClientMessage> response = future.get();

                assert(protocol::BOOLEAN_RESULT == response->getMessageType());
                return response->getBoolean();
            }

            void ServerListenerService::registerListener(const std::string &registrationId, int callId) {
                registrationAliasMap.put(registrationId, boost::shared_ptr<std::string>(new std::string(registrationId)));
                registrationIdMap.put(registrationId, boost::shared_ptr<int>(new int(callId)));
            }

            void ServerListenerService::reRegisterListener(const std::string& registrationId, boost::shared_ptr<std::string> alias, int callId) {
                boost::shared_ptr<const std::string> oldAlias = registrationAliasMap.put(registrationId, alias);
                if (oldAlias.get() != NULL) {
                    registrationIdMap.remove(*oldAlias);
                    registrationIdMap.put(*alias, boost::shared_ptr<int>(new int(callId)));
                }
            }

            bool ServerListenerService::deRegisterListener(std::string& registrationId) {
                boost::shared_ptr<const std::string> uuid = registrationAliasMap.remove(registrationId);
                if (uuid != NULL) {
                    registrationId = *uuid;
                    boost::shared_ptr<int> callId = registrationIdMap.remove(*uuid);
                    clientContext.getInvocationService().removeEventHandler(*callId);
                    return true;
                }
                return false;
            }

            void ServerListenerService::retryFailedListener(boost::shared_ptr<connection::CallPromise> listenerPromise) {
                try {
                    InvocationService& invocationService = clientContext.getInvocationService();
                    boost::shared_ptr<connection::Connection> result = invocationService.resend(listenerPromise, "internalRetryOfUnkownAddress");
                    if (NULL == result.get()) {
                        util::LockGuard lockGuard(failedListenerLock);
                        failedListeners.push_back(listenerPromise);
                    }
                } catch (exception::IException&) {
                    util::LockGuard lockGuard(failedListenerLock);
                    failedListeners.push_back(listenerPromise);
                }
            }

            void ServerListenerService::triggerFailedListeners() {
                std::vector<boost::shared_ptr<connection::CallPromise> >::iterator it;
                std::vector<boost::shared_ptr<connection::CallPromise> > newFailedListeners;
                InvocationService& invocationService = clientContext.getInvocationService();

                util::LockGuard lockGuard(failedListenerLock);
                for (it = failedListeners.begin(); it != failedListeners.end(); ++it) {
                    try {
                        boost::shared_ptr<connection::Connection> result = invocationService.resend(*it, "internalRetryOfUnkownAddress");

                        if (NULL == result.get()) { // resend failed
                            newFailedListeners.push_back(*it);
                        }
                    } catch (exception::IOException&) {
                        newFailedListeners.push_back(*it);
                        continue;
                    }
                }
                failedListeners = newFailedListeners;
            }

        }
    }
}

