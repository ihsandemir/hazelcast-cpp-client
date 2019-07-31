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
//
// Created by sancar koyunlu on 26/02/14.
//
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServer.h"

#include "ClientTestSupport.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/LifecycleListener.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class Ticket5344Test : public ClientTestSupport {
            protected:
                class ClientDisconnectedLifeCycleListener : public LifecycleListener {
                public:
                    ClientDisconnectedLifeCycleListener(CountDownLatch &latch) : latch(latch) {}

                public:
                    virtual void stateChanged(const LifecycleEvent &lifecycleEvent) {
                        if (lifecycleEvent.getState() == LifecycleEvent::CLIENT_DISCONNECTED) {
                            latch.countDown();
                        }
                    }

                private:
                    util::CountDownLatch &latch;
                };

                class WaitListenerInfiniteTask : public util::Runnable {
                public:
                    virtual const string getName() const {
                        return "WaitListenerInfiniteTask";
                    }

                    virtual void run() {
                        ClientConfig *config = 0;
                        HazelcastClient *client = 0;

                        while (true) {
                            util::CountDownLatch disconnectedLatch(1);
                            ClientDisconnectedLifeCycleListener listener(disconnectedLatch);

                            if (config == 0) {
                                config = new ClientConfig;

                                GroupConfig &groupConfig = config->getGroupConfig();
                                groupConfig.setName("dev");
                                groupConfig.setPassword("dev-pass");
                                config->addAddress(Address("127.0.0.1", 5701));
                                config->addListener(&listener);

                                client = new HazelcastClient(*config);
                            }

                            disconnectedLatch.await();

                            client->removeLifecycleListener(&listener);
                            client->shutdown();
                            delete client;
                            client = 0;
                            delete config;
                            config = 0;
                        }
                    }
                };

                class MapPutGetTask : public Runnable {
                public:
                    MapPutGetTask(HazelcastClient &client) : client(client) {}

                    virtual const string getName() const {
                        return "MapPutGetTask";
                    }

                    virtual void run() {
                        IMap<int, int> map = client.getMap<int, int>("MyMap");
                        while (true) {
                            int key = rand() % 1000;

                            try {
                                if (key % 2) {
                                    map.get(key);
                                    std::cout << "Get for " << key << std::endl;
                                } else {
                                    map.put(key, key);
                                    std::cout << "Put for " << key << std::endl;
                                }
                            } catch (exception::IException &e) {
                                std::cout << "Operation caused exception:" << e << std::endl;
                                std::cout << "Test continues." << std::endl;
                            }
                            sleep(5);
                        }
                    }

                private:
                    HazelcastClient &client;
                };

                void terminateRandomNode(HazelcastClient &client) {
                    size_t index = rand() % servers.size();
                    vector<boost::shared_ptr<HazelcastServer> >::iterator iterator = servers.begin();
                    for (size_t i = 0; i <= index; ++i) {
                        ++iterator;
                    }
                    (*iterator)->terminate();
                    servers.erase(iterator);
                }

            protected:
                std::vector<boost::shared_ptr<HazelcastServer> > servers;
            };

            TEST_F(Ticket5344Test, testTicket5344_PutGet) {
/*
                servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
                servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
                servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
                servers.push_back(boost::shared_ptr<HazelcastServer>(new HazelcastServer(*g_srvFactory)));
*/
                boost::shared_ptr<HazelcastServer> server(new HazelcastServer(*g_srvFactory));

                ClientConfig config;
                config.getNetworkConfig().setConnectionAttemptLimit(INT32_MAX);
                HazelcastClient client;

                util::Thread clientThread(boost::shared_ptr<util::Runnable>(new MapPutGetTask(client)), getLogger());
                clientThread.start();

                while (true) {
                    sleep(30);

                    //terminateRandomNode(client);
                    server->terminate();

                    // start a new server
                    server.reset(new HazelcastServer(*g_srvFactory));

                    //servers.push_back();
                }
            }

            TEST_F(Ticket5344Test, testTicket5344_WaitForLifeCycleListener) {
                boost::shared_ptr<HazelcastServer> server(new HazelcastServer(*g_srvFactory));

                util::Thread clientThread(boost::shared_ptr<util::Runnable>(new WaitListenerInfiniteTask()),
                                          getLogger());
                clientThread.start();

                while (true) {
                    sleep(10);

                    server->terminate();

                    // start a new server
                    server.reset(new HazelcastServer(*g_srvFactory));
                }

            }
        }
    }
}


