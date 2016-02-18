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
// Created by sancar koyunlu on 8/27/13.

#ifndef HAZELCAST_ApiTest
#define HAZELCAST_ApiTest

#include "ClientTestSupport.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServer.h"

#include <memory>

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace test {
            class ApiTest : public ClientTestSupport {
            public:
                ApiTest() : instance(*g_srvFactory)
                        , client(getNewClient()) {
                }

                ~ApiTest() {}
            protected:
                HazelcastServer instance;
                std::auto_ptr<HazelcastClient> client;
            };


/*
            template <typename K, typename V>
            TEST_F(ApiTest, NonBoostApi) {

                IMap2 map2 = client->getMap("nonboostmap");
                IMap<int, int> map = client->getMap<int, int>("regularMap");

                (void) map;
                (void) map2;

                for (int i = 0; i < 1000000; ++i) {
                    boost::shared_ptr<V> value = map.get(K);
                }

                for (int i = 0; i < 1000000; ++i) {
                    std::auto_ptr<V> value = map.get(K);
                }

            }
*/

            class Person : public hazelcast::client::serialization::IdentifiedDataSerializable {
            public:
                int getFactoryId() const {
                    return 0;
                }

                int getClassId() const {
                    return 5;
                }

                void writeData(serialization::ObjectDataOutput &writer) const {
                }

                void readData(serialization::ObjectDataInput &reader) {
                }
            };

            class MyAllocator : public hazelcast::client::api::IAllocator {
                void *allocate(api::SerializationType type) {
                    if (type == hazelcast::client::api::IdentifiedDataSerializable) {
                        return malloc(sizeof(Person));
                    }

                    return NULL;
                }

                void deallocate(void *ptr) {
                    free(ptr);
                }
            };

            std::ostream &operator << (std::ostream &out, const Person &p) {
                out << "Printing Person object";
                return out;
            }

            TEST_F(ApiTest, testNewAPI) {
                std::auto_ptr<hazelcast::client::api::IMap> map1(client->getMap("experimental map"));

                hazelcast::client::IMap<int, std::string> map2 = client->getMap<int, std::string>("original map");

                std::string key1("myKey");
                struct hazelcast::client::api::ObjectType result = map1->get(&key1);

                if (NULL != result.object) {
                    std::cout << "The returned object for key " << key1 << " is " << (std::string *) result.object << std::endl;
                } else {
                    std::cout << "The returned object for key " << key1 << " is NULL" << std::endl;
                }

                int key2 = 5;
                result = map1->get(&key2);

                if (NULL != result.object) {
                    std::cout << "The returned object for key " << key2 << " is " << *((int *) result.object) << std::endl;
                } else {
                    std::cout << "The returned object for key " << key2 << " is NULL" << std::endl;
                }

                MyAllocator allocator;
                int key3 = 9;
                result = map1->get(&key3, &allocator);

                if (NULL != result.object) {
                    std::cout << "The returned object for key " << key3 << " is " << *((Person *) result.object) << std::endl;
                } else {
                    std::cout << "The returned object for key " << key3 << " is NULL." << std::endl;
                }

                map2.put(7, "ihsan");
                boost::shared_ptr<std::string> val = map2.get(7);
                if (NULL != val.get()) {
                    std::cout << "Original implementation: The returned object for key 7 " << " is " << (*val) << std::endl;
                } else {
                    std::cout << "Original implementation: The returned object for key 7 " << " is NULL." << std::endl;
                }
            }
        }
    }
}


#endif //HAZELCAST_ApiTest

