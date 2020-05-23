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
#include<boost/thread/barrier.hpp>

#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <regex>
#include <vector>
#include "ringbuffer/StartsWithStringFilter.h"
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/exception/IllegalStateException.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/impl/Partition.h>
#include <hazelcast/client/spi/impl/ClientInvocation.h>
#include <gtest/gtest.h>
#include <thread>
#include <hazelcast/client/spi/ClientContext.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/protocol/Principal.h>
#include <hazelcast/client/connection/Connection.h>
#include <ClientTestSupport.h>
#include <memory>
#include <hazelcast/client/proxy/PNCounterImpl.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressUtil.h>
#include <hazelcast/util/RuntimeAvailableProcessors.h>
#include <hazelcast/client/serialization/pimpl/DataOutput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>

#include <ClientTestSupportBase.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <ctime>
#include <cerrno>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <cstdint>
#include "customSerialization/TestCustomSerializerX.h"
#include "customSerialization/TestCustomXSerializable.h"
#include "customSerialization/TestCustomPersonSerializer.h"
#include "serialization/ChildTemplatedPortable2.h"
#include "serialization/ParentTemplatedPortable.h"
#include "serialization/ChildTemplatedPortable1.h"
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <hazelcast/client/query/FalsePredicate.h>
#include <hazelcast/client/query/EqualPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <HazelcastServer.h>
#include "TestHelperFunctions.h"
#include <cmath>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithoutBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/CallIdSequenceWithBackpressure.h>
#include <hazelcast/client/spi/impl/sequence/FailFastCallIdSequence.h>
#include <iostream>
#include <string>
#include "executor/tasks/SelectAllMembers.h"
#include "executor/tasks/IdentifiedFactory.h"
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/client/serialization/serialization.h>
#include "executor/tasks/CancellationAwareTask.h"
#include "executor/tasks/NullCallable.h"
#include "executor/tasks/SerializedCounterCallable.h"
#include "executor/tasks/MapPutPartitionAwareCallable.h"
#include "executor/tasks/SelectNoMembers.h"
#include "executor/tasks/GetMemberUuidTask.h"
#include "executor/tasks/FailingCallable.h"
#include "executor/tasks/AppendCallable.h"
#include "executor/tasks/TaskWithUnserializableResponse.h"
#include <executor/tasks/CancellationAwareTask.h>
#include <executor/tasks/FailingCallable.h>
#include <executor/tasks/SelectNoMembers.h>
#include <executor/tasks/SerializedCounterCallable.h>
#include <executor/tasks/TaskWithUnserializableResponse.h>
#include <executor/tasks/GetMemberUuidTask.h>
#include <executor/tasks/AppendCallable.h>
#include <executor/tasks/SelectAllMembers.h>
#include <executor/tasks/MapPutPartitionAwareCallable.h>
#include <executor/tasks/NullCallable.h>
#include <cstdlib>
#include <fstream>
#include <boost/asio.hpp>
#include <cassert>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/connection/ClientConnectionManagerImpl.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/connection/Connection.h"

#include "hazelcast/client/MembershipListener.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/EntryAdapter.h"
#include "hazelcast/client/LifecycleListener.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/Cluster.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/query/SqlPredicate.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/SerializationConfig.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/ITopic.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/query/OrPredicate.h"
#include "hazelcast/client/query/RegexPredicate.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/query/QueryConstants.h"
#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/NotEqualPredicate.h"
#include "hazelcast/client/query/InPredicate.h"
#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/LikePredicate.h"
#include "hazelcast/client/query/GreaterLessPredicate.h"
#include "hazelcast/client/query/AndPredicate.h"
#include "hazelcast/client/query/BetweenPredicate.h"
#include "hazelcast/client/query/EqualPredicate.h"
#include "hazelcast/client/query/TruePredicate.h"
#include "hazelcast/client/query/FalsePredicate.h"
#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/ItemListener.h"
#include "hazelcast/client/MultiMap.h"
#include "hazelcast/util/LittleEndianBufferWrapper.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/client/EntryEvent.h"
#include "hazelcast/client/HazelcastJsonValue.h"
#include "hazelcast/client/IList.h"
#include "hazelcast/client/IQueue.h"
#include "hazelcast/client/ClientProperties.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/ISet.h"
#include "hazelcast/client/ReliableTopic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientMultiMapTest : public ClientTestSupport {
            protected:
                class MyMultiMapListener : public EntryAdapter {
                public:
                    MyMultiMapListener(boost::latch &addedLatch, boost::latch &removedLatch) : addedLatch(addedLatch),
                                                                                               removedLatch(removedLatch) {}

                    void entryAdded(const EntryEvent &event) override {
                        addedLatch.count_down();
                    }

                    void entryRemoved(const EntryEvent &event) override {
                        removedLatch.count_down();
                    }

                private:
                    boost::latch &addedLatch;
                    boost::latch &removedLatch;
                };

                static void fillData() {
                    ASSERT_TRUE(mm->put("key1", "value1").get());
                    ASSERT_TRUE(mm->put("key1", "value2").get());
                    ASSERT_TRUE(mm->put("key1", "value3").get());
                    ASSERT_TRUE(mm->put("key2", "value4").get());
                    ASSERT_TRUE(mm->put("key2", "value5").get());
                }
                
                void TearDown() override {
                    // clear mm
                    mm->clear();
                }

                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    mm = client->getMultiMap("MyMultiMap");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    mm = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<MultiMap> mm;
            };

            HazelcastServer *ClientMultiMapTest::instance = nullptr;
            HazelcastClient *ClientMultiMapTest::client = nullptr;

            TEST_F(ClientMultiMapTest, testPutGetRemove) {
                fillData();
                ASSERT_EQ(3, mm->valueCount("key1").get());
                ASSERT_EQ(2, mm->valueCount("key2").get());
                ASSERT_EQ(5, mm->size().get());

                auto coll = mm->get<std::string, std::string>("key1").get();
                ASSERT_EQ(3, (int) coll.size());

                coll = mm->remove<std::string, std::string>("key2").get();
                ASSERT_EQ(2, (int) coll.size());
                ASSERT_EQ(0, mm->valueCount("key2").get());
                ASSERT_EQ(0, ((int) mm->get<std::string, std::string>("key2").get().size()));

                ASSERT_FALSE(mm->remove("key1", "value4").get());
                ASSERT_EQ(3, mm->size().get());

                ASSERT_TRUE(mm->remove("key1", "value2").get());
                ASSERT_EQ(2, mm->size().get());

                ASSERT_TRUE(mm->remove("key1", "value1").get());
                ASSERT_EQ(1, mm->size().get());
                ASSERT_EQ("value3", (mm->get<std::string, std::string>("key1").get()[0]));
            }


            TEST_F(ClientMultiMapTest, testKeySetEntrySetAndValues) {
                fillData();
                ASSERT_EQ(2, (int) mm->keySet<std::string>().get().size());
                ASSERT_EQ(5, (int) mm->values<std::string>().get().size());
                ASSERT_EQ(5, ((int) mm->entrySet<std::string, std::string>().get().size()));
            }


            TEST_F(ClientMultiMapTest, testContains) {
                fillData();
                ASSERT_FALSE(mm->containsKey<std::string>("key3").get());
                ASSERT_TRUE(mm->containsKey<std::string>("key1").get());

                ASSERT_FALSE(mm->containsValue<std::string>("value6").get());
                ASSERT_TRUE(mm->containsValue<std::string>("value4").get());

                ASSERT_FALSE(mm->containsEntry("key1", "value4").get());
                ASSERT_FALSE(mm->containsEntry("key2", "value3").get());
                ASSERT_TRUE(mm->containsEntry("key1", "value1").get());
                ASSERT_TRUE(mm->containsEntry("key2", "value5").get());
            }

            TEST_F(ClientMultiMapTest, testListener) {
                boost::latch latch1Add(8);
                boost::latch latch1Remove(4);
                boost::latch latch2Add(3);
                boost::latch latch2Remove(3);
                MyMultiMapListener listener1(latch1Add, latch1Remove);
                MyMultiMapListener listener2(latch2Add, latch2Remove);

                std::string id1 = mm->addEntryListener(listener1, true).get();
                std::string id2 = mm->addEntryListener(listener2, "key3", true).get();

                fillData();

                mm->remove("key1", "value2").get();

                mm->put("key3", "value6").get();
                mm->put("key3", "value7").get();
                mm->put("key3", "value8").get();

                mm->remove<std::string, std::string>("key3").get();

                ASSERT_OPEN_EVENTUALLY(latch1Add);
                ASSERT_OPEN_EVENTUALLY(latch1Remove);
                ASSERT_OPEN_EVENTUALLY(latch2Add);
                ASSERT_OPEN_EVENTUALLY(latch2Remove);

                ASSERT_TRUE(mm->removeEntryListener(id1).get());
                ASSERT_TRUE(mm->removeEntryListener(id2).get());
            }

            TEST_F(ClientMultiMapTest, testLock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread t([&]() {
                    if (!mm->tryLock("key1").get()) {
                        latch1.count_down();
                    }
                });
                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testLockTtl) {
                mm->lock("key1", std::chrono::seconds(3)).get();
                boost::latch latch1(2);
                std::thread t([&]() {
                    if (!mm->tryLock("key1").get()) {
                        latch1.count_down();
                    }
                    if (mm->tryLock("key1", std::chrono::seconds(5)).get()) {
                        latch1.count_down();
                    }
                });

                ASSERT_OPEN_EVENTUALLY(latch1);
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testTryLock) {
                ASSERT_TRUE(mm->tryLock("key1", std::chrono::seconds(2)).get());
                boost::latch latch1(1);
                std::thread t([&]() {
                    if (!mm->tryLock("key1", std::chrono::seconds(2)).get()) {
                        latch1.count_down();
                    }
                });
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_TRUE(mm->isLocked("key1").get());

                boost::latch latch2(1);
                boost::barrier b(2);
                std::thread t2([&]() {
                    b.count_down_and_wait();
                    if (!mm->tryLock("key1", std::chrono::seconds(20)).get()) {
                        latch2.count_down();
                    }
                });
                b.count_down_and_wait();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                mm->unlock("key1").get();
                ASSERT_OPEN_EVENTUALLY(latch2);
                ASSERT_TRUE(mm->isLocked("key1").get());
                mm->forceUnlock("key1").get();
            }

            TEST_F(ClientMultiMapTest, testForceUnlock) {
                mm->lock("key1").get();
                boost::latch latch1(1);
                std::thread t2([&]() {
                    mm->forceUnlock("key1").get();
                });
                ASSERT_OPEN_EVENTUALLY(latch1);
                ASSERT_FALSE(mm->isLocked("key1").get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientListTest : public ClientTestSupport {
            protected:
                class MyListItemListener : public ItemListener {
                public:
                    MyListItemListener(boost::latch &latch1) : latch1(latch1) {}

                    void itemAdded(const ItemEvent &itemEvent) {
                        auto type = itemEvent.getEventType();
                        ASSERT_EQ((int) ItemEventType::ADDED, type);
                        ASSERT_EQ("MyList", itemEvent.getName());
                        std::string host = itemEvent.getMember().getAddress().getHost();
                        ASSERT_TRUE(host == "localhost" || host == "127.0.0.1");
                        ASSERT_EQ(5701, itemEvent.getMember().getAddress().getPort());
                        ASSERT_EQ("item-1", itemEvent.getItem().get<std::string>().value());
                        latch1.count_down();
                    }

                    void itemRemoved(const ItemEvent &item) {}
                private:
                    boost::latch &latch1;
                };

                virtual void TearDown() {
                    // clear list
                    list->clear();
                }

                static void SetUpTestCase() {
#ifdef HZ_BUILD_WITH_SSL
                    sslFactory = new HazelcastServerFactory(g_srvFactory->getServerAddress(), getSslFilePath());
                    instance = new HazelcastServer(*sslFactory);
#else
                    instance = new HazelcastServer(*g_srvFactory);
#endif
                    ClientConfig clientConfig = getConfig();

#ifdef HZ_BUILD_WITH_SSL
                    config::ClientNetworkConfig networkConfig;
                    config::SSLConfig sslConfig;
                    sslConfig.setEnabled(true).addVerifyFile(getCAFilePath()).setCipherList("HIGH");
                    networkConfig.setSSLConfig(sslConfig);
                    clientConfig.setNetworkConfig(networkConfig);
#endif // HZ_BUILD_WITH_SSL

                    client = new HazelcastClient(clientConfig);
                    list = client->getList("MyList");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;
                    delete sslFactory;

                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IList> list;
                static HazelcastServerFactory *sslFactory;
            };

            HazelcastServer *ClientListTest::instance = nullptr;
            HazelcastClient *ClientListTest::client = nullptr;
            HazelcastServerFactory *ClientListTest::sslFactory = nullptr;

            TEST_F(ClientListTest, testAddAll) {
                std::vector<std::string> l;
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_TRUE(list->addAll(l));

                ASSERT_TRUE(list->addAll(1, l));
                ASSERT_EQ(4, list->size());

                auto item = list->get<std::string>(0).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
                item = list->get<std::string>(1).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item2", item.value());
                item = list->get<std::string>(2).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item2", item.value());
                item = list->get<std::string>(3).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item3", item.value());
            }

            TEST_F(ClientListTest, testAddSetRemove) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                list->add(0, "item3").get();
                ASSERT_EQ(3, list->size().get());
                boost:optional<std::string> temp = list->set(2, "item4").get();
                ASSERT_EQ("item2", *temp);

                ASSERT_EQ(3, list->size());
                ASSERT_EQ("item3", list->get<std::string>(0).get().value());
                ASSERT_EQ("item1", list->get<std::string>(1).get().value());
                ASSERT_EQ("item4", list->get<std::string>(2).get().value());

                ASSERT_FALSE(list->remove("item2").get());
                ASSERT_TRUE(list->remove("item3").get());

                temp = list->remove<std::string>(1).get();
                ASSERT_TRUE(temp.has_value());
                ASSERT_EQ("item4", temp.value());

                ASSERT_EQ(1, list->size());
                ASSERT_EQ("item1", list->get<std::string>(0).get().value());
            }

            TEST_F(ClientListTest, testIndexOf) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                ASSERT_EQ(-1, list->indexOf("item5").get());
                ASSERT_EQ(0, list->indexOf("item1").get());

                ASSERT_EQ(-1, list->lastIndexOf("item6").get());
                ASSERT_EQ(2, list->lastIndexOf("item1").get());
            }

            TEST_F(ClientListTest, testToArray) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> ar = list->toArray<std::string>().get();

                ASSERT_EQ("item1", ar[0]);
                ASSERT_EQ("item2", ar[1]);
                ASSERT_EQ("item1", ar[2]);
                ASSERT_EQ("item4", ar[3]);

                std::vector<std::string> arr2 = list->subList<std::string>(1, 3).get();

                ASSERT_EQ(2, (int) arr2.size());
                ASSERT_EQ("item2", arr2[0]);
                ASSERT_EQ("item1", arr2[1]);
            }

            TEST_F(ClientListTest, testContains) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                ASSERT_FALSE(list->contains("item3").get());
                ASSERT_TRUE(list->contains("item2").get());

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_FALSE(list->containsAll(l).get());
                ASSERT_TRUE(list->add("item3").get());
                ASSERT_TRUE(list->containsAll(l).get());
            }

            TEST_F(ClientListTest, testRemoveRetainAll) {
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item2").get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_TRUE(list->add("item4").get());

                std::vector<std::string> l;
                l.push_back("item4");
                l.push_back("item3");

                ASSERT_TRUE(list->removeAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());
                ASSERT_FALSE(list->removeAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                l.push_back("item1");
                l.push_back("item2");
                ASSERT_FALSE(list->retainAll(l).get());
                ASSERT_EQ(3, (int) list->size().get());

                l.clear();
                ASSERT_TRUE(list->retainAll(l).get());
                ASSERT_EQ(0, (int) list->size().get());
            }

            TEST_F(ClientListTest, testListener) {
                boost::latch latch1(1);

                MyListItemListener listener(latch1);
                std::string registrationId = list->addItemListener(listener, true).get();

                list->add("item-1").get();

                ASSERT_EQ(boost::cv_status::no_timeout, latch1.wait_for(boost::chrono::seconds(20)));

                ASSERT_TRUE(list->removeItemListener(registrationId).get());
            }

            TEST_F(ClientListTest, testIsEmpty) {
                ASSERT_TRUE(list->isEmpty().get());
                ASSERT_TRUE(list->add("item1").get());
                ASSERT_FALSE(list->isEmpty().get());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientQueueTest : public ClientTestSupport {
            protected:
                void offer(int numberOfItems) {
                    for (int i = 1; i <= numberOfItems; ++i) {
                        ASSERT_TRUE(q->offer(std::string("item") + std::to_string(i)).get());
                    }
                }
                
                virtual void TearDown() {
                    q->clear();
                }
                
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(getConfig());
                    q = client->getQueue("MyQueue");
                }

                static void TearDownTestCase() {
                    delete client;
                    delete instance;

                    q = nullptr;
                    client = nullptr;
                    instance = nullptr;
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static std::shared_ptr<IQueue> q;
            };

            HazelcastServer *ClientQueueTest::instance = nullptr;
            HazelcastClient *ClientQueueTest::client = nullptr;

            class QueueTestItemListener : public ItemListener {
            public:
                explicit QueueTestItemListener(boost::latch &latch1) : latch1(latch1) {}

                void itemAdded(const ItemEvent &itemEvent) override {
                    latch1.count_down();
                }

                void itemRemoved(const ItemEvent &item) override {}
            private:
                boost::latch &latch1;
            };

            TEST_F(ClientQueueTest, testListener) {
                ASSERT_EQ(0, q->size());

                boost::latch latch1(5);

                std::string id = q->addItemListener(QueueTestItemListener(latch1), true).get();
                
                for (int i = 0; i < 5; i++) {
                    ASSERT_TRUE(q->offer(std::string("event_item") + std::to_string(i)));
                }

                ASSERT_EQ(boost::cv_status::no_timeout, latch1.wait_for(boost::chrono::seconds(5)));
                ASSERT_TRUE(q->removeItemListener(id).get());

                // added for test coverage
                ASSERT_NO_THROW(q->destroy().get());
            }

            void testOfferPollThread2(hazelcast::util::ThreadArgs &args) {
                auto *q = (IQueue *) args.arg0;
                hazelcast::util::sleep(2);
                q->offer("item1");
            }

            TEST_F(ClientQueueTest, testOfferPoll) {
                for (int i = 0; i < 10; i++) {
                    ASSERT_TRUE(q->offer("item").get());
                }
                ASSERT_EQ(10, q->size().get());
                q->poll<std::string>().get();
                ASSERT_TRUE(q->offer("item", std::chrono::milliseconds(5)).get());

                for (int i = 0; i < 10; i++) {
                    ASSERT_TRUE(q->poll<std::string>().get().has_value());
                }
                ASSERT_EQ(0, q->size().get());

                hazelcast::util::StartedThread t2(testOfferPollThread2, q.get());

                boost::optional<std::string> item = q->poll<std::string>(std::chrono::seconds(30)).get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
                t2.join();
            }

            TEST_F(ClientQueueTest, testPeek) {
                offer(3);
                boost::optional<std::string> item = q->peek<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());
            }

            TEST_F(ClientQueueTest, testTake) {
                q->put("peek 1").get();
                ASSERT_TRUE(q->offer("peek 2").get());
                ASSERT_TRUE(q->offer("peek 3").get());

                boost::optional<std::string> item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 1", item.value());

                item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 2", item.value());

                item = q->take<std::string>().get();
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("peek 3", item.value());

                ASSERT_TRUE(q->isEmpty().get());

// start a thread to insert an item
                hazelcast::util::StartedThread t2(testOfferPollThread2, q.get());

                item = q->take<std::string>().get();  //  should block till it gets an item
                ASSERT_TRUE(item.has_value());
                ASSERT_EQ("item1", item.value());

                t2.join();
            }

            TEST_F(ClientQueueTest, testRemainingCapacity) {
                int capacity = q->remainingCapacity().get();
                ASSERT_TRUE(capacity > 10000);
                q->offer("item");
                ASSERT_EQ(capacity - 1, q->remainingCapacity().get());
            }


            TEST_F(ClientQueueTest, testRemove) {
                offer(3);
                ASSERT_FALSE(q->remove("item4").get());
                ASSERT_EQ(3, q->size().get());

                ASSERT_TRUE(q->remove("item2").get());

                ASSERT_EQ(2, q->size().get());

                ASSERT_EQ("item1", q->poll<std::string>().get().value());
                ASSERT_EQ("item3", q->poll<std::string>().get().value());
            }


            TEST_F(ClientQueueTest, testContains) {
                offer(5);
                ASSERT_TRUE(q->contains("item3").get());
                ASSERT_FALSE(q->contains("item").get());

                std::vector<std::string> list;
                list.emplace_back("item4");
                list.emplace_back("item2");

                ASSERT_TRUE(q->containsAll(list).get());

                list.emplace_back("item");
                ASSERT_FALSE(q->containsAll(list).get());
            }

            TEST_F(ClientQueueTest, testDrain) {
                offer(5);
                std::vector<std::string> list;
                size_t result = q->drainTo(list, 2).get();
                ASSERT_EQ(2U, result);
                ASSERT_EQ("item1", list[0]);
                ASSERT_EQ("item2", list[1]);

                std::vector<std::string> list2;
                result = q->drainTo(list2).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item3", list2[0]);
                ASSERT_EQ("item4", list2[1]);
                ASSERT_EQ("item5", list2[2]);

                offer(3);
                list2.clear();
                result = q->drainTo(list2, 5).get();
                ASSERT_EQ(3U, result);
                ASSERT_EQ("item1", list2[0]);
                ASSERT_EQ("item2", list2[1]);
                ASSERT_EQ("item3", list2[2]);
            }

            TEST_F(ClientQueueTest, testToArray) {
                offer(5);
                std::vector<std::string> array = q->toArray<std::string>();
                size_t size = array.size();
                for (size_t i = 0; i < size; i++) {
                    ASSERT_EQ(std::string("item") + std::to_string(i + 1), array[i]);
                }
            }

            TEST_F(ClientQueueTest, testAddAll) {
                std::vector<std::string> coll;
                coll.emplace_back("item1");
                coll.emplace_back("item2");
                coll.emplace_back("item3");
                coll.emplace_back("item4");

                ASSERT_TRUE(q->addAll(coll));
                int size = q->size().get();
                ASSERT_EQ(size, (int) coll.size());
            }

            TEST_F(ClientQueueTest, testRemoveRetain) {
                offer(5);
                std::vector<std::string> list;
                list.emplace_back("item8");
                list.emplace_back("item9");
                ASSERT_FALSE(q->removeAll(list).get());
                ASSERT_EQ(5, q->size());

                list.emplace_back("item3");
                list.emplace_back("item4");
                list.emplace_back("item1");
                ASSERT_TRUE(q->removeAll(list));
                ASSERT_EQ(2, q->size());

                list.clear();
                list.emplace_back("item2");
                list.emplace_back("item5");
                ASSERT_FALSE(q->retainAll(list).get());
                ASSERT_EQ(2, q->size());

                list.clear();
                ASSERT_TRUE(q->retainAll(list).get());
                ASSERT_EQ(0, q->size().get());
            }

            TEST_F(ClientQueueTest, testClear) {
                offer(5);
                q->clear().get();
                ASSERT_EQ(0, q->size());
                ASSERT_FALSE(q->poll<std::string>().get().has_value());
            }

            TEST_F(ClientQueueTest, testIsEmpty) {
                ASSERT_TRUE(q->isEmpty().get());
                ASSERT_TRUE(q->offer("item1").get());
                ASSERT_FALSE(q->isEmpty().get());
            }

            TEST_F(ClientQueueTest, testPut) {
                q->put("item1").get();
                ASSERT_EQ(1, q->size().get());
            }
        }
    }
}

int hazelcast::client::test::executor::tasks::SelectAllMembers::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SelectAllMembers::getClassId() const {
    return IdentifiedFactory::SELECT_ALL_MEMBERS;
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::SelectAllMembers::SelectAllMembers() {}

bool hazelcast::client::test::executor::tasks::SelectAllMembers::select(const hazelcast::client::Member &member) const {
    return true;
}

void hazelcast::client::test::executor::tasks::SelectAllMembers::toString(std::ostream &os) const {
    os << "SelectAllMembers";
}

int hazelcast::client::test::executor::tasks::CancellationAwareTask::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::CancellationAwareTask::getClassId() const {
    return IdentifiedFactory::CANCELLATION_AWARE_TASK;
}

void hazelcast::client::test::executor::tasks::CancellationAwareTask::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeLong(sleepTime);
}

void hazelcast::client::test::executor::tasks::CancellationAwareTask::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    sleepTime = reader.readLong();
}

hazelcast::client::test::executor::tasks::CancellationAwareTask::CancellationAwareTask(int64_t sleepTime) : sleepTime(
        sleepTime) {}

hazelcast::client::test::executor::tasks::CancellationAwareTask::CancellationAwareTask() {}



int hazelcast::client::test::executor::tasks::NullCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::NullCallable::getClassId() const {
    return IdentifiedFactory::NULL_CALLABLE;
}

void hazelcast::client::test::executor::tasks::NullCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::NullCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::NullCallable::NullCallable() {}



int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SerializedCounterCallable::getClassId() const {
    return IdentifiedFactory::SERIALIZED_COUNTER_CALLABLE;
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeInt(counter + 1);
}

void hazelcast::client::test::executor::tasks::SerializedCounterCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    counter = reader.readInt() + 1;
}

hazelcast::client::test::executor::tasks::SerializedCounterCallable::SerializedCounterCallable() : counter(0) {}



int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getClassId() const {
    return IdentifiedFactory::MAP_PUTPARTITIONAWARE_CALLABLE;
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&mapName);
    writer.writeObject<std::string>(&partitionKey);
}

void hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // no need to implement at client side for the tests
}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable() {}

hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::MapPutPartitionAwareCallable(
        const std::string &mapName, const std::string &partitionKey) : mapName(mapName), partitionKey(partitionKey) {}

const std::string *hazelcast::client::test::executor::tasks::MapPutPartitionAwareCallable::getPartitionKey() const {
    return &partitionKey;
}



int hazelcast::client::test::executor::tasks::SelectNoMembers::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::SelectNoMembers::getClassId() const {
    return IdentifiedFactory::SELECT_NO_MEMBERS;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::SelectNoMembers::SelectNoMembers() {}

bool hazelcast::client::test::executor::tasks::SelectNoMembers::select(const hazelcast::client::Member &member) const {
    return false;
}

void hazelcast::client::test::executor::tasks::SelectNoMembers::toString(std::ostream &os) const {
    os << "SelectNoMembers";
}



int hazelcast::client::test::executor::tasks::GetMemberUuidTask::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::GetMemberUuidTask::getClassId() const {
    return IdentifiedFactory::GET_MEMBER_UUID_TASK;
}

void hazelcast::client::test::executor::tasks::GetMemberUuidTask::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::GetMemberUuidTask::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::GetMemberUuidTask::GetMemberUuidTask() {}



int hazelcast::client::test::executor::tasks::FailingCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::FailingCallable::getClassId() const {
    return IdentifiedFactory::FAILING_CALLABLE;
}

void hazelcast::client::test::executor::tasks::FailingCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::FailingCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::FailingCallable::FailingCallable() {}



std::string hazelcast::client::test::executor::tasks::AppendCallable::APPENDAGE = ":CallableResult";

int hazelcast::client::test::executor::tasks::AppendCallable::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::AppendCallable::getClassId() const {
    return IdentifiedFactory::APPEND_CALLABLE;
}

void hazelcast::client::test::executor::tasks::AppendCallable::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
    writer.writeUTF(&msg);
}

void hazelcast::client::test::executor::tasks::AppendCallable::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
    // No need to implement this part for the client
}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable() {}

hazelcast::client::test::executor::tasks::AppendCallable::AppendCallable(const std::string &msg) : msg(msg) {}



int hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::getFactoryId() const {
    return IdentifiedFactory::FACTORY_ID;
}

int hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::getClassId() const {
    return IdentifiedFactory::TASK_WITH_UNSERIALIZABLE_RESPONSE;
}

void hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::writeData(
        hazelcast::client::serialization::ObjectDataOutput &writer) const {
}

void hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::readData(
        hazelcast::client::serialization::ObjectDataInput &reader) {
}

hazelcast::client::test::executor::tasks::TaskWithUnserializableResponse::TaskWithUnserializableResponse() {}





namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExecutorServiceTest : public ClientTestSupport {
            protected:
                static const size_t numberOfMembers;

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                         "hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    client = new HazelcastClient;
                }

                static void TearDownTestCase() {
                    delete client;
                    for (HazelcastServer *server : instances) {
                        server->shutdown();
                        delete server;
                    }

                    client = nullptr;
                }

                class FailingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    FailingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(
                            latch1) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                    }

                    virtual void onFailure(std::exception_ptr e) {
                        exception = e;
                        latch1->count_down();
                    }

                    std::exception_ptr getException() {
                        return exception;
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                    hazelcast::util::Sync<std::exception_ptr> exception;
                };

                class SuccessfullExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    SuccessfullExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(latch1) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        latch1->count_down();
                    }

                    virtual void onFailure(std::exception_ptr e) {
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                };

                class ResultSettingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    ResultSettingExecutionCallback(const std::shared_ptr<boost::latch> &latch1) : latch1(latch1) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        result.set(response);
                        latch1->count_down();
                    }

                    virtual void onFailure(std::exception_ptr e) {
                    }

                    std::shared_ptr<std::string> getResult() {
                        return result.get();
                    }

                private:
                    const std::shared_ptr<boost::latch> latch1;
                    hazelcast::util::Sync<std::shared_ptr<std::string>> result;
                };

                class MultiExecutionCompletionCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionCompletionCallback(const std::string &msg,
                                                     const std::shared_ptr<boost::latch> &responseLatch,
                                                     const std::shared_ptr<boost::latch> &completeLatch) : msg(msg),
                                                                                                           responseLatch(
                                                                                                                   responseLatch),
                                                                                                           completeLatch(
                                                                                                                   completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() && *response == msg + executor::tasks::AppendCallable::APPENDAGE) {
                            responseLatch->count_down();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, std::exception_ptr exception) {
                    }

                    virtual void onComplete(const std::unordered_map<Member, std::shared_ptr<std::string> > &values,
                                            const std::unordered_map<Member, std::exception_ptr> &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        std::string expectedValue(msg + executor::tasks::AppendCallable::APPENDAGE);
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() && *entry.second == expectedValue) {
                                completeLatch->count_down();
                            }
                        }
                    }

                private:
                    std::string msg;
                    const std::shared_ptr<boost::latch> responseLatch;
                    const std::shared_ptr<boost::latch> completeLatch;
                };

                class MultiExecutionNullCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionNullCallback(const std::shared_ptr<boost::latch> &responseLatch,
                                               const std::shared_ptr<boost::latch> &completeLatch)
                            : responseLatch(responseLatch), completeLatch(completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() == NULL) {
                            responseLatch->count_down();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, std::exception_ptr exception) {
                    }

                    virtual void onComplete(const std::unordered_map<Member, std::shared_ptr<std::string> > &values,
                                            const std::unordered_map<Member, std::exception_ptr> &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() == NULL) {
                                completeLatch->count_down();
                            }
                        }
                    }

                private:
                    const std::shared_ptr<boost::latch> responseLatch;
                    const std::shared_ptr<boost::latch> completeLatch;
                };

                static std::vector<HazelcastServer *> instances;
                static HazelcastClient *client;
                static HazelcastServerFactory *factory;
            };

            std::vector<HazelcastServer *>ClientExecutorServiceTest::instances;
            HazelcastClient *ClientExecutorServiceTest::client = nullptr;
            HazelcastServerFactory *ClientExecutorServiceTest::factory = nullptr;
            const size_t ClientExecutorServiceTest::numberOfMembers = 4;

            TEST_F(ClientExecutorServiceTest, testIsTerminated) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isTerminated());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                ASSERT_EQ(boost::future_status::timeout, promise.get_future().wait_for(boost::chrono::seconds(1)));
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);
                auto future = promise.get_future();

                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_FALSE(future.is_ready());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                auto promise = service->submit<executor::tasks::CancellationAwareTask, bool>(task);

                auto future = promise.get_future();
                ASSERT_EQ(boost::future_status::timeout, future.wait_for(boost::chrono::seconds(1)));

                ASSERT_TRUE(promise.cancel(true));

                ASSERT_THROW(future.get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::FailingCallable task;

                auto future = service->submit<executor::tasks::FailingCallable, std::string>(task).get_future();

                ASSERT_THROW(future.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                executor::tasks::FailingCallable task;
                std::shared_ptr<ExecutionCallback<std::string> > callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                auto failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable()).get_future();

                ASSERT_THROW(failingFuture.get(), exception::IllegalStateException);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string mapName = randomMapName();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(mapName, randomString()), selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                auto future = service->submitToKeyOwner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name).get_future();
                std::shared_ptr<int> value = future.get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_FALSE(members.empty());
                auto future = service->submitToMember<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]).get_future();
                auto value = future.get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                auto future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse).get_future();

                ASSERT_THROW(future.get(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));

                std::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch1));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);

                auto exception = callback->getException();
                ASSERT_THROW(std::rethrow_exception(exception), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                auto future = service->submitToMember<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members[0]).get_future();

                std::shared_ptr<std::string> uuid = future.get();
                ASSERT_NOTNULL(uuid.get(), std::string);
                ASSERT_EQ(members[0].getUuid(), *uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                auto futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(task,
                                                                                                            members);

                for (const Member &member : members) {
                    ASSERT_EQ(1U, futuresMap.count(member));
                    auto it = futuresMap.find(member);
                    ASSERT_NE(futuresMap.end(), it);
                    std::shared_ptr<std::string> uuid = (*it).second.get_future().get();
                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selectAll;

                auto f = service->submit<executor::tasks::AppendCallable, std::string>(callable,
                                                                                       selectAll).get_future();

                std::shared_ptr<std::string> result = f.get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                auto futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, selectAll);

                for (auto &pair : futuresMap) {
                    const Member &member = pair.first;
                    auto future = pair.second.get_future();

                    std::shared_ptr<std::string> uuid = future.get();

                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                auto futuresMap = service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable);

                for (auto &pair : futuresMap) {
                    auto future = pair.second.get_future();

                    std::shared_ptr<std::string> result = future.get();

                    ASSERT_NOTNULL(result.get(), std::string);
                    ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, randomString());

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch1));

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submitToMember<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable,
                                                                                                    members[0],
                                                                                                    callback);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                ASSERT_EQ(1, map.size());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;
                std::shared_ptr<boost::latch> responseLatch(new boost::latch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(
                        new ResultSettingExecutionCallback(responseLatch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector,
                                                                              std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                std::shared_ptr<std::string> message = callback->getResult();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(
                        new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(
                        new boost::latch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<boost::latch> responseLatch(new boost::latch(numberOfMembers));
                std::shared_ptr<boost::latch> completeLatch(new boost::latch(numberOfMembers));

                executor::tasks::NullCallable callable;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                auto result = service->submit<executor::tasks::AppendCallable, std::string>(callable).get_future();

                std::shared_ptr<std::string> message = result.get();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch1));

                service->submit<executor::tasks::AppendCallable, std::string>(callable,
                                                                              std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                auto f = service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable,
                                                                                                              "key").get_future();

                std::shared_ptr<std::string> result = f.get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch1));

                service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key",
                                                                                                     std::static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                                             callback));

                ASSERT_OPEN_EVENTUALLY(*latch1);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                auto f = service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(
                        callable).get_future();

                std::shared_ptr<std::string> result = f.get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(member.getUuid(), *result);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                std::shared_ptr<boost::latch> latch1(new boost::latch(1));
                std::shared_ptr<ExecutionCallback<std::string>> callback(new ResultSettingExecutionCallback(latch1));

                service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch1);
                std::shared_ptr<std::string> value = std::static_pointer_cast<ResultSettingExecutionCallback>(
                        callback)->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(member.getUuid(), *value);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"));

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"), selector);
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string targetUuid = member.getUuid();
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                service->executeOnKeyOwner<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, key);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMember) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                Member &member = members[0];
                std::string targetUuid = member.getUuid();

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMember<executor::tasks::MapPutPartitionAwareCallable>(callable, member);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> allMembers = client->getCluster().getMembers();
                std::vector<Member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, members);

                ASSERT_TRUE_EVENTUALLY(map->containsKey(members[0].getUuid()).get() && map->containsKey(members[1].getUuid()).get());
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

               auto map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable,
                                                                                         std::vector<Member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                auto map = client->getMap(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnAllMembers<executor::tasks::MapPutPartitionAwareCallable>(callable);

                assertSizeEventually((int) numberOfMembers, map);
            }
        }
    }
}



#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsConfigTest : public ::testing::Test {
                };

                TEST_F (AwsConfigTest, testDefaultValues) {
                    client::config::ClientAwsConfig awsConfig;
                    ASSERT_EQ("", awsConfig.getAccessKey());
                    ASSERT_EQ("us-east-1", awsConfig.getRegion());
                    ASSERT_EQ("ec2.amazonaws.com", awsConfig.getHostHeader());
                    ASSERT_EQ("", awsConfig.getIamRole());
                    ASSERT_EQ("", awsConfig.getSecretKey());
                    ASSERT_EQ("", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("", awsConfig.getTagKey());
                    ASSERT_EQ("", awsConfig.getTagValue());
                    ASSERT_FALSE(awsConfig.isInsideAws());
                    ASSERT_FALSE(awsConfig.isEnabled());
                }

                TEST_F (AwsConfigTest, testSetValues) {
                    client::config::ClientAwsConfig awsConfig;

                    awsConfig.setAccessKey("mykey");
                    awsConfig.setRegion("myregion");
                    awsConfig.setHostHeader("myheader");
                    awsConfig.setIamRole("myrole");
                    awsConfig.setSecretKey("mysecret");
                    awsConfig.setSecurityGroupName("mygroup");
                    awsConfig.setTagKey("mytagkey");
                    awsConfig.setTagValue("mytagvalue");
                    awsConfig.setInsideAws(true);
                    awsConfig.setEnabled(true);

                    ASSERT_EQ("mykey", awsConfig.getAccessKey());
                    ASSERT_EQ("myregion", awsConfig.getRegion());
                    ASSERT_EQ("myheader", awsConfig.getHostHeader());
                    ASSERT_EQ("myrole", awsConfig.getIamRole());
                    ASSERT_EQ("mysecret", awsConfig.getSecretKey());
                    ASSERT_EQ("mygroup", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("mytagkey", awsConfig.getTagKey());
                    ASSERT_EQ("mytagvalue", awsConfig.getTagValue());
                    ASSERT_TRUE(awsConfig.isInsideAws());
                    ASSERT_TRUE(awsConfig.isEnabled()) << awsConfig;
                }

                TEST_F (AwsConfigTest, testSetEmptyValues) {
                    client::config::ClientAwsConfig awsConfig;

                    ASSERT_THROW(awsConfig.setAccessKey(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setRegion(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setHostHeader(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setSecretKey(""), exception::IllegalArgumentException);
                }

                TEST_F (AwsConfigTest, testClientConfigUsage) {
                    ClientConfig clientConfig;
                    client::config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                    awsConfig.setEnabled(true);

                    ASSERT_TRUE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());

                    client::config::ClientAwsConfig newConfig;

                    clientConfig.getNetworkConfig().setAwsConfig(newConfig);
                    // default constructor sets enabled to false
                    ASSERT_FALSE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());
                }

                TEST_F (AwsConfigTest, testInvalidAwsMemberPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "65536";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "-1";

                    ASSERT_THROW(HazelcastClient hazelcastClient(clientConfig),
                                 exception::InvalidConfigurationException);
                }
            }
        }
    }
}


#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsClientTest : public ::testing::Test {
                };

                TEST_F (AwsClientTest, testClientAwsMemberNonDefaultPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(std::getenv("AWS_ACCESS_KEY_ID")).setSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
#endif
                    HazelcastClient hazelcastClient(clientConfig);
                    auto map = hazelcastClient.getMap("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                TEST_F (AwsClientTest, testClientAwsMemberWithSecurityGroupDefaultIamRole) {
                    ClientConfig clientConfig;
                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setSecurityGroupName("launch-wizard-147");

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                            // The access key and secret will be retrieved from default IAM role at windows machine
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
#else
                    clientConfig.getNetworkConfig().getAwsConfig().setAccessKey(std::getenv("AWS_ACCESS_KEY_ID")).
                            setSecretKey(std::getenv("AWS_SECRET_ACCESS_KEY"));
#endif

                    HazelcastClient hazelcastClient(clientConfig);
                    auto map = hazelcastClient.getMap("myMap");
                    map->put(5, 20).get();
                    auto val = map->get<int, int>(5).get();
                    ASSERT_TRUE(val.has_value());
                    ASSERT_EQ(20, val.value());
                }

                // FIPS_mode_set is not available for Mac OS X built-in openssl library
#ifndef __APPLE__
                                                                                                                                        TEST_F (AwsClientTest, testFipsEnabledAwsDiscovery) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
                    #else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
                    #endif

                    // Turn Fips mode on
                    FIPS_mode_set(1);

                    HazelcastClient hazelcastClient(clientConfig);
                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    std::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }
#endif // ifndef __APPLE__

                /**
                 * Following test can only run from inside the AWS network
                 */
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                        TEST_F (AwsClientTest, testRetrieveCredentialsFromIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setIamRole("cloudbees-role").setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }

                TEST_F (AwsClientTest, testRetrieveCredentialsFromInstanceProfileDefaultIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }
#endif
            }
        }
    }
}

#endif // HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class DescribeInstancesTest : public ClientTestSupport {
                };

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndNonExistentValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue(
                            "non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSetToNonExistentTag) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("non-existent-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSetToNonExistentValue) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("launch-wizard-147");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesNonExistentSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("non-existent-group");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader(), getLogger());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL


#ifdef HZ_BUILD_WITH_SSL



namespace awsutil = hazelcast::client::aws::utility;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class CloudUtilityTest : public ClientTestSupport {
                };

                TEST_F (CloudUtilityTest, testUnmarshallResponseXml) {
                    std::filebuf fb;
                    ASSERT_TRUE(fb.open("hazelcast/test/resources/sample_aws_response.xml", std::ios::in));
                    std::istream responseStream(&fb);

                    config::ClientAwsConfig awsConfig;
                    std::map<std::string, std::string> results = hazelcast::client::aws::utility::CloudUtility::unmarshalTheResponse(
                            responseStream, getLogger());
                    ASSERT_EQ(4U, results.size());
                    ASSERT_NE(results.end(), results.find("10.0.16.13"));
                    ASSERT_EQ("", results["10.0.16.13"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.17"));
                    ASSERT_EQ("54.85.192.215", results["10.0.16.17"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.25"));
                    ASSERT_EQ("", results["10.0.16.25"]);
                    ASSERT_NE(results.end(), results.find("172.30.4.118"));
                    ASSERT_EQ("54.85.192.213", results["172.30.4.118"]);
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
