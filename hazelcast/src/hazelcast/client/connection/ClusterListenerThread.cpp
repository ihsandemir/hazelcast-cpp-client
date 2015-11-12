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
// Created by sancar koyunlu on 5/23/13.


#include <stdio.h>
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/protocol/parameters/MemberAttributeChangeResultParameters.h"
#include "hazelcast/client/protocol/parameters/AddListenerResultParameters.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/impl/SerializableCollection.h"
#include "hazelcast/client/LifecycleEvent.h"
#include "hazelcast/client/spi/ServerListenerService.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/protocol/parameters/RegisterMembershipListenerParameters.h"
#include "hazelcast/client/protocol/parameters/MemberListResultParameters.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread(spi::ClientContext& clientContext)
            : startLatch(1)
            , clientContext(clientContext)
            , deletingConnection(false) {

            }

            void ClusterListenerThread::staticRun(util::ThreadArgs& args) {
                ClusterListenerThread *clusterListenerThread = (ClusterListenerThread *)args.arg0;
                clusterListenerThread->run(args.currentThread);
            }

            void ClusterListenerThread::setThread(util::Thread *thread) {
                clusterListenerThread.reset(thread);
            }

            void ClusterListenerThread::run(util::Thread *currentThread) {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        if (conn.get() == NULL) {
                            try {
                                conn = clientContext.getClusterService().connectToOne();
                            } catch (std::exception& e) {
                                util::ILogger::getLogger().severe(std::string("Error while connecting to cluster! =>") + e.what());
                                isStartedSuccessfully = false;
                                startLatch.countDown();
                                return;
                            }
                        }

                        loadInitialMemberList();
                        clientContext.getServerListenerService().triggerFailedListeners();
                        isStartedSuccessfully = true;
                        startLatch.countDown();
                        listenMembershipEvents();
                        currentThread->interruptibleSleep(1);
                    } catch (std::exception& e) {
                        if (clientContext.getLifecycleService().isRunning()) {
                            util::ILogger::getLogger().warning(std::string("Error while listening cluster events! -> ") + e.what());
                        }

                        clientContext.getConnectionManager().onCloseOwnerConnection();
                        if (deletingConnection.compareAndSet(false, true)) {
                            util::IOUtil::closeResource(conn.get());
                            conn.reset();
                            deletingConnection = false;
                            clientContext.getLifecycleService().fireLifecycleEvent(LifecycleEvent::CLIENT_DISCONNECTED);
                        }
                        currentThread->interruptibleSleep(1);
                    }

                }

            }


            void ClusterListenerThread::stop() {
                if (deletingConnection.compareAndSet(false, true)) {
                    util::IOUtil::closeResource(conn.get());
                    conn.reset();
                    deletingConnection = false;
                }
                clusterListenerThread->interrupt();
                clusterListenerThread->join();
            }

            std::vector<Address> ClusterListenerThread::getSocketAddresses() {
                std::vector<Address> addresses;
                if (NULL != members.get() && !members->empty()) {
                    std::vector<Address> clusterAddresses = getClusterAddresses();
                    addresses.insert(addresses.begin(), clusterAddresses.begin(), clusterAddresses.end());
                }
                std::vector<Address> configAddresses = getConfigAddresses();
                addresses.insert(addresses.end(), configAddresses.begin(), configAddresses.end());
                return addresses;
            }
            
            void ClusterListenerThread::loadInitialMemberList() {
                std::auto_ptr<protocol::ClientMessage> request =
                        protocol::parameters::RegisterMembershipListenerParameters::encode();

                std::auto_ptr<protocol::ClientMessage> response = conn->sendAndReceive(*request);

                assert(protocol::MEMBER_LIST_RESULT == response->getMessageType());

                std::auto_ptr<protocol::parameters::MemberListResultParameters> memberListResultParameters =
                        protocol::parameters::MemberListResultParameters::decode(*response);

                initialMembers(*memberListResultParameters);
            }

            void ClusterListenerThread::listenMembershipEvents() {
                while (clientContext.getLifecycleService().isRunning()) {
                    std::auto_ptr<protocol::ClientMessage> clientMessage = conn->readBlocking();
                    if (!clientContext.getLifecycleService().isRunning())
                        break;

                    protocol::ClientMessageType type = (protocol::ClientMessageType)clientMessage->getMessageType();
                    switch (type) {
                        case protocol::MEMBER_LIST_RESULT:
                            {
                                std::auto_ptr<protocol::parameters::MemberListResultParameters> memberListResultParameters =
                                        protocol::parameters::MemberListResultParameters::decode(*clientMessage);

                                initialMembers(*memberListResultParameters);
                            }
                            break;
                        case protocol::MEMBER_RESULT:
                            {
                                handleMember(*clientMessage);
                            }
                            break;
                        case protocol::MEMBER_ATTRIBUTE_RESULT:
                            {
                                std::auto_ptr<protocol::parameters::MemberAttributeChangeResultParameters> changeParameters =
                                        protocol::parameters::MemberAttributeChangeResultParameters::decode(*clientMessage);
                                memberAttributeChanged(*changeParameters->attributeChange);
                            }
                            break;
                        case protocol::ADD_LISTENER_RESULT:
                            {
                                // Just ignore the result, nothing to do
                            }
                            break;
                        default:
                            {
                                char msg[150];
                                sprintf(msg, "[ClusterListenerThread::listenMembershipEvents()] Unknown message type : %d\n", clientMessage->getMessageType());
                                util::ILogger::getLogger().warning(msg);
                            }
                    }
                }
            }

            void ClusterListenerThread::handleMember(protocol::ClientMessage &response) {
                std::auto_ptr<protocol::parameters::MemberResultParameters> memberResultParameters =
                        protocol::parameters::MemberResultParameters::decode(response);

                MembershipEvent::MembershipEventType eventType = (MembershipEvent::MembershipEventType)memberResultParameters->event;
                if (MembershipEvent::MEMBER_ADDED == eventType) {
                    clientContext.getConnectionManager().removeEndpoint(memberResultParameters->member->getAddress());
                    members->push_back(memberResultParameters->member.get());
                    updateMembers(*memberResultParameters, eventType);

                    // memory ownership is moved to the members vector
                    memberResultParameters->member.release();
                } else if (MembershipEvent::MEMBER_REMOVED == eventType) {
                    for (std::vector<Member *>::iterator it = members->begin() ;
                         members->end() != it; ++it) {
                        if(*(*it) == *memberResultParameters->member) {
                            members->erase(it);
                            updateMembers(*memberResultParameters, eventType);
                            break;
                        }
                    }
                }
            }

            void ClusterListenerThread::updateMembers(
                    protocol::parameters::MemberResultParameters & memberResultParameters,
                                          MembershipEvent::MembershipEventType & eventType) {
                updateMembersRef();
                MembershipEvent membershipEvent(this->clientContext.getCluster(), eventType,
                                                    *memberResultParameters.member);
                this->clientContext.getClusterService().fireMembershipEvent(membershipEvent);
            }

            void ClusterListenerThread::fireMemberAttributeEvent(hazelcast::client::impl::MemberAttributeChange const& memberAttributeChange, Member& target) {
                MemberAttributeEvent::MemberAttributeOperationType operationType = memberAttributeChange.getOperationType();
                const std::string& value = memberAttributeChange.getValue();
                const std::string& key = memberAttributeChange.getKey();
                if (operationType == MemberAttributeEvent::PUT) {//PUT
                    target.setAttribute(key, value);
                } else if (operationType == MemberAttributeEvent::REMOVE) {//REMOVE
                    target.removeAttribute(key);
                }
                MemberAttributeEvent memberAttributeEvent(clientContext.getCluster(), target, operationType, key, value);
                clientContext.getClusterService().fireMemberAttributeEvent(memberAttributeEvent);
            }

            void ClusterListenerThread::updateMembersRef() {
                std::map<Address, Member, addressComparator> addrMap;
                std::stringstream memberInfo;
                memberInfo << std::endl << "Members [" << members->size() << "]  {" << std::endl;
                for (MEMBERVECTOR_TYPE::const_iterator it = members->begin(); it != members->end(); ++it) {
                    Member &member = **it;
                    (memberInfo << "\t" << member << std::endl);
                    addrMap[member.getAddress()] = member;
                }
                memberInfo << "}" << std::endl;
                util::ILogger::getLogger().info(memberInfo.str());
                clientContext.getClusterService().setMembers(addrMap);

            }

            std::vector<Address> ClusterListenerThread::getClusterAddresses() const {
                std::vector<Address> socketAddresses;
                for (MEMBERVECTOR_TYPE::const_iterator it = members->begin(); it != members->end(); ++it) {
                    socketAddresses.push_back((**it).getAddress());
                }
                return socketAddresses;
            }

            std::vector<Address>  ClusterListenerThread::getConfigAddresses() const {
                std::vector<Address> socketAddresses;
                std::set<Address, addressComparator>& configAddresses = clientContext.getClientConfig().getAddresses();
                std::set<Address, addressComparator>::iterator it;

                for (it = configAddresses.begin(); it != configAddresses.end(); ++it) {
                    socketAddresses.push_back((*it));
                }

                if (socketAddresses.size() == 0) {
                    socketAddresses.push_back(Address("127.0.0.1", 5701));
                }
                std::random_shuffle(socketAddresses.begin(), socketAddresses.end());
                return socketAddresses;
            }

            void ClusterListenerThread::initialMembers(protocol::parameters::MemberListResultParameters &memberListResultParameters) {
                std::auto_ptr<MEMBERS_TYPE> originalMembers = members;

                members = memberListResultParameters.memberList;

                updateMembersRef();

                std::vector<MembershipEvent> events;

                std::map<std::string, const Member* > prevMembers;
                if (NULL != originalMembers.get() && !originalMembers->empty()) {
                    for (MEMBERVECTOR_TYPE::const_iterator it = originalMembers->begin();
                         it != originalMembers->end(); ++it) {
                        prevMembers[(*it)->getUuid()] = *it;
                    }

                    std::map<std::string, const Member* >::const_iterator prevEnd = prevMembers.end();
                    for (MEMBERVECTOR_TYPE::const_iterator it = members->begin(); it != members->end(); ++it) {
                        std::map<std::string, const Member* >::iterator foundMember = prevMembers.find((*it)->getUuid());

                        if (prevEnd == foundMember) {
                            events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_ADDED, *(*it)));
                        } else {
                            prevMembers.erase(foundMember);
                        }
                    }

                    for (std::map<std::string, const Member* >::const_iterator it = prevMembers.begin(); it != prevMembers.end(); ++it) {
                        const Member *member = it->second;
                        events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_REMOVED, *member));
                        if (clientContext.getClusterService().isMemberExists(member->getAddress())) {
                            clientContext.getConnectionManager().removeEndpoint(member->getAddress());
                        }
                    }
                } else { // i.e. if we originally had an empty members list, just add all as member added
                    for (MEMBERVECTOR_TYPE::const_iterator it = members->begin(); it != members->end(); ++it) {
                        events.push_back(MembershipEvent(clientContext.getCluster(), MembershipEvent::MEMBER_ADDED, *(*it)));
                    }
                }

                for (std::vector<MembershipEvent>::iterator it = events.begin(); it != events.end(); ++it) {
                    clientContext.getClusterService().fireMembershipEvent((*it));
                }
            }

            void ClusterListenerThread::memberAttributeChanged(const impl::MemberAttributeChange &change) {
                for (MEMBERVECTOR_TYPE::const_iterator it = members->begin(); it != members->end(); ++it) {
                    Member& target = *(*it);
                    if (target.getUuid() == change.getUuid()) {
                        fireMemberAttributeEvent(change, target);
                    }
                }
            }
        }
    }
}

