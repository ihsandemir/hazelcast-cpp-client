/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <random>

#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/pipelining.h>

using namespace std;
using namespace hazelcast::client;
using namespace hazelcast::util;

struct MyKey{
    int64_t Id;
    int64_t SubSeq;
    int32_t Scen;

    friend bool operator==(const MyKey &lhs, const MyKey &rhs) {
        return lhs.Id == rhs.Id;
    }
}; //All necessary operators and hz_serializer in place

struct MyVal{
    double Amount;
    char Code;
}; //All necessary operators and hz_serializer in place

namespace hazelcast { namespace client { namespace serialization {
    template<>
    struct hz_serializer<MyKey> : public identified_data_serializer {
        static int32_t get_factory_id() noexcept {
            return 1;
        }

        static int32_t get_class_id() noexcept {
            return 1;
        }

        static void write_data(const MyKey &object, object_data_output &out) {
            out.write(object.Id);
            out.write(object.SubSeq);
            out.write(object.Scen);
        }

        static MyKey read_data(object_data_input &in) {
            return MyKey{in.read<int64_t>(), in.read<int64_t>(), in.read<int32_t>()};
        }
    };
    template<>
    struct hz_serializer<MyVal> : public identified_data_serializer {
        static int32_t get_factory_id() noexcept {
            return 1;
        }

        static int32_t get_class_id() noexcept {
            return 2;
        }

        static void write_data(const MyVal &object, object_data_output &out) {
            out.write(object.Amount);
            out.write(object.Code);
        }

        static MyVal read_data(object_data_input &in) {
            return MyVal{in.read<double>(), in.read<char>()};
        }
    };
} }};

namespace std {
    template<> struct hash<MyKey> {
        std::size_t operator()(const MyKey &object) const noexcept {
            auto hash_value = std::hash<int64_t>()(object.Id);
            return hash_value;
        }
    };
}

class pipelining_load_test {
public:
    pipelining_load_test() : client_(hazelcast::new_client(new_config()).get()), map_(client_.get_map("map").get()){}

    client_config new_config() {
        hazelcast::client::client_config config;
        // set heartbeat timeout to 10 minutes
        config.set_property(hazelcast::client::client_properties::PROP_HEARTBEAT_TIMEOUT, "600000");
        return config;
    }

    //Approach1: With pipelining
    void with_pipelining() {
        using MyMap = std::unordered_map<MyKey, MyVal>;
        using MySet = std::unordered_set<MyKey>;

        MyKey myKey;
        MyVal myVal;

        MySet mySet;
        MyMap myMap;

        //Sample Data Generation
        using namespace std::chrono;
        auto start = steady_clock::now();
        for(int i=0; i<10000000;++i){
            myKey.Id = i;
            myKey.SubSeq = i%2;
            myKey.Scen = 100;
            myVal.Amount = i *0.3;
            myVal.Code = 'A';
            mySet.emplace(myKey);
            auto result = myMap.insert({std::move(myKey), std::move(myVal)});
            if (!result.second) {
                std::cout << "Insertion failed for " << i << std::endl;
            }
        }

        int depth=30;
        std::shared_ptr<pipelining<int> > p = pipelining<int>::create(depth);
        MyMap smallMap;
        smallMap.reserve(10 * 1000); // reserve enough to avoid resizing
        int counter = 0;
        for(auto  item : myMap){
            smallMap.emplace(item);
            ++counter;
            if(counter % 10000 == 0){ // smallMap.size() may be more costly
                p->add(map_->put_all<MyKey, MyVal>(smallMap).then([](boost::future<void> f) { f.get(); return boost::make_optional<int>(0); }));
                smallMap.clear();
                std::cout << "Counter: " << counter << std::endl;
            }
        }
        try {
            p->results();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }

        std::cout << "Finished\n";
    }

    //Approach1: With pipelining
    void with_wait_for_all() {
        using MyMap = std::unordered_map<MyKey, MyVal>;
        using MySet = std::unordered_set<MyKey>;

        MyKey myKey;
        MyVal myVal;

        MySet mySet;
        MyMap myMap;

        //Sample Data Generation
        using namespace std::chrono;
        auto start = steady_clock::now();
        for(int i=0; i<10000000;++i){
            myKey.Id = i;
            myKey.SubSeq = i%2;
            myKey.Scen = 100;
            myVal.Amount = i *0.3;
            myVal.Code = 'A';
            mySet.emplace(myKey);
            auto result = myMap.insert({std::move(myKey), std::move(myVal)});
            if (!result.second) {
                std::cout << "Insertion failed for " << i << std::endl;
            }
        }

        MyMap smallMap;
        smallMap.reserve(10 * 1000); // reserve enough to avoid resizing
        std::vector<boost::future<void>> all_futures;
        int counter = 0;
        for(auto  item : myMap){
            smallMap.emplace(item);
            ++counter;
            if(counter % 10000 == 0){ // smallMap.size() may be more costly
                all_futures.push_back(map_->put_all<MyKey, MyVal>(smallMap));
                smallMap.clear();
                std::cout << "Counter: " << counter << std::endl;
            }
        }
        try {
            boost::wait_for_all(all_futures.begin(), all_futures.end());
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }

private:
    hazelcast_client client_;
    std::shared_ptr<imap> map_;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "USAGE: pipelining_load [use_pipeline|use_futures]" << std::endl;
        return -1;
    }
    std::string scenario = argv[1];
    std::cout << "Scenario: " << scenario << std::endl;

    pipelining_load_test main;
    using namespace std::chrono;
    auto start = steady_clock::now();
    if (scenario == "use_pipeline") {
        main.with_pipelining();
    } else if (scenario == "use_futures") {
        main.with_wait_for_all();
    }

    cout << "Finished. Total time:" << duration_cast<milliseconds>(steady_clock::now() - start).count() << " msecs." << endl;

    return 0;
}
