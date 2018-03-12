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

#include <hazelcast/client/spi/impl/ClientClusterServiceImpl.h>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/InitialMembershipEvent.h>
#include <hazelcast/client/InitialMembershipListener.h>
#include <hazelcast/client/spi/impl/ClientMembershipListener.h>

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                impl::ClientClusterServiceImpl::ClientClusterServiceImpl(hazelcast::client::spi::ClientContext &client)
                        : client(client) {
                    ClientConfig &config = client.getClientConfig();
                    std::set<MembershipListener *> const &membershipListeners = config.getMembershipListeners();

                    BOOST_FOREACH(MembershipListener *listener, membershipListeners) {
                                    addMembershipListenerWithoutInit(listener);
                                }

                }

                std::string ClientClusterServiceImpl::addMembershipListenerWithoutInit(MembershipListener *listener) {
                    std::string id = util::UuidUtil::newUnsecureUuidString();
                    listeners.put(id, listener);
                    return id;
                }

                boost::shared_ptr<Member> ClientClusterServiceImpl::getMember(const Address &address) {
                    std::map<Address, boost::shared_ptr<Member> > currentMembers = members.get();
                    const std::map<hazelcast::client::Address, boost::shared_ptr<hazelcast::client::Member> >::iterator &it = currentMembers.find(
                            address);
                    if (it == currentMembers.end()) {
                        return boost::shared_ptr<Member>();
                    }
                    return it->second;
                }

                boost::shared_ptr<Member> ClientClusterServiceImpl::getMember(const std::string &uuid) {
                    std::vector<boost::shared_ptr<Member> > memberList = getMemberList();
                    BOOST_FOREACH(boost::shared_ptr<Member> &member, memberList) {
                                    if (uuid == member->getUuid()) {
                                        return member;
                                    }
                                }
                    return boost::shared_ptr<Member>();
                }

                std::vector<boost::shared_ptr<Member> > ClientClusterServiceImpl::getMemberList() {
                    typedef std::map<Address, boost::shared_ptr<Member> > MemberMap;
                    MemberMap memberMap = members.get();
                    std::vector<boost::shared_ptr<Member> > memberList;
                    BOOST_FOREACH(const MemberMap::value_type &entry, memberMap) {
                                    memberList.push_back(entry.second);
                                }
                }

                boost::shared_ptr<Address> ClientClusterServiceImpl::getMasterAddress() {
                    std::vector<boost::shared_ptr<Member> > memberList = getMemberList();
                    return !memberList.empty() ? boost::shared_ptr<Address>(new Address(memberList[0]->getAddress())
                                               : boost::shared_ptr<Address>();
                }

                size_t ClientClusterServiceImpl::getSize() {
                    return getMemberList().size();
                }

                std::string
                ClientClusterServiceImpl::addMembershipListener(MembershipListener *listener) {
                    if (listener == NULL) {
                        throw exception::NullPointerException("ClientClusterServiceImpl::addMembershipListener",
                                                              "listener can't be null");
                    }

                    util::LockGuard guard(initialMembershipListenerMutex);
                    std::string id = addMembershipListenerWithoutInit(listener);
                    initMembershipListener(listener);
                    return id;
                }

                void ClientClusterServiceImpl::initMembershipListener(MembershipListener *listener) {
                    if (listener->shouldRequestInitialMembers()) {
                        Cluster &cluster = client.getCluster();
                        std::vector<boost::shared_ptr<Member> > memberCollection = getMemberList();
                        InitialMembershipEvent event(cluster, memberCollection);
                        ((InitialMembershipListener *) listener)->init(event);
                    }
                }

                void ClientClusterServiceImpl::start() {
                    clientMembershipListener.reset(new ClientMembershipListener(client));
                }

                void ClientClusterServiceImpl::handleMembershipEvent(const MembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const Member &member = event.getMember();
                    std::map<Address, boost::shared_ptr<Member> > newMap = members.get();
                    if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                        newMap[member.getAddress()] = boost::shared_ptr<Member>(new Member(member));
                    } else {
                        newMap.erase(member.getAddress());
                    }
                    members = newMap;
                    fireMembershipEvent(event);
                }

                void ClientClusterServiceImpl::fireMembershipEvent(const MembershipEvent &event) {
                    BOOST_FOREACH(MembershipListener *listener , listeners.values()) {
                        if (event.getEventType() == MembershipEvent::MEMBER_ADDED) {
                            listener->memberAdded(event);
                        } else {
                            listener->memberRemoved(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::handleInitialMembershipEvent(const InitialMembershipEvent &event) {
                    util::LockGuard guard(initialMembershipListenerMutex);
                    const std::vector<Member> &initialMembers = event.getMembers();
                        std::map<Address, boost::shared_ptr<Member> > newMap;
                        BOOST_FOREACH (const Member &initialMember , initialMembers) {
                            newMap[initialMember.getAddress()] = boost::shared_ptr<Member>(new Member(initialMember));
                        }
                        members.set(newMap);
                        fireInitialMembershipEvent(event);
                    
                }

                void ClientClusterServiceImpl::fireInitialMembershipEvent(const InitialMembershipEvent &event) {
                    BOOST_FOREACH (MembershipListener *listener , listeners.values()) {
                        if (listener->shouldRequestInitialMembers()) {
                            ((InitialMembershipListener *) listener)->init(event);
                        }
                    }
                }

                void ClientClusterServiceImpl::shutdown() {
                }

                void ClientClusterServiceImpl::listenMembershipEvents(
                        const boost::shared_ptr<connection::Connection> &ownerConnection) {
                    ClientMembershipListener::listenMembershipEvents(clientMembershipListener, ownerConnection);
                }
            }
        }
    }
}
