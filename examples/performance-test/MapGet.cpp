#include <iostream>
#include <vector>
#include <stdint.h>

#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast;
using namespace hazelcast::client;
using namespace std;

struct ThreadParams {
    ThreadParams(const int numberOfKeys, const int durationSeconds, const vector<byte> *value,
                 IMap<int, vector<byte> > *map, util::Atomic<double> *opsPerSecond) : numberOfKeys(numberOfKeys),
                                                                       durationSeconds(durationSeconds), value(value),
                                                                       map(map), opsPerSecond(opsPerSecond) {}

    const int numberOfKeys;
    const int durationSeconds;
    const vector<byte> *value;
    IMap<int, vector<byte> > *map;
    util::Atomic<double> *opsPerSecond;
};

void testMap(util::ThreadArgs &args) {
    ThreadParams *params = static_cast<ThreadParams *>(args.arg0);
    const int numberOfKeys = params->numberOfKeys;
    const int durationSeconds = params->durationSeconds;
    IMap<int, vector<byte> > *map = params->map;
    int64_t count = 0;
    int key = rand() % numberOfKeys;

    int64_t beginTimeMillis = util::currentTimeMillis();
    int64_t endTimeMillis = durationSeconds * 1000 + beginTimeMillis;
    while (true) {
        map->get(key);
        count++;
        if (count % 10000 == 0 && endTimeMillis < util::currentTimeMillis()) {
            break;
        }
    }
    long timePassedInMillis = util::currentTimeMillis() - beginTimeMillis;

    *params->opsPerSecond = (double) count / timePassedInMillis;
}

int main(int argc, char *args[]) {

    if (argc != 6) {
        cerr << "USAGE: SimpleThroughput durationSeconds threadCount valueSizeInBytes numberOfKeys address" << endl;
        return -1;
    }

    const int durationSeconds = atoi(args[1]);
    cout << "durationSeconds = " << durationSeconds << endl;
    const int threadCount = atoi(args[2]);
    cout << "threadCount = " << threadCount << endl;
    const int valueSizeInBytes = atoi(args[3]);
    cout << "valueSizeInBytes = " << valueSizeInBytes << endl;
    const int numberOfKeys = atoi(args[4]);
    cout << "numberOfKeys = " << numberOfKeys << endl;
    const string address = args[5];
    cout << "address = " << address << endl;

    ClientConfig config;
    config.addAddress(Address(address, 5701));

    HazelcastClient hazelcastInstance(config);
    IMap<int, vector<byte> > map = hazelcastInstance.getMap<int, vector<byte> >("test");

    vector<util::Thread *> threads(threadCount);
    const vector<byte> value(valueSizeInBytes);

    for (int i = 0; i < numberOfKeys; i++) {
        map.put(i, value);
    }

    vector<util::Atomic<double> *> opsPerMs;
    for (int i = 0; i < threadCount; i++) {
        opsPerMs.push_back(new util::Atomic<double>(0.0));
    }

    for (int i = 0; i < threadCount; i++) {
        ThreadParams *params = new ThreadParams(numberOfKeys, durationSeconds, &value, &map, opsPerMs[i]);
        threads[i] = new util::Thread(testMap, params);
    }

    for (int i = 0; i < threadCount; i++) {
        threads[i]->join();
    }

    double totalOpsPerMs = 0;
    for (int i = 0; i < threadCount; i++) {
        double result = *(opsPerMs[i]);
        totalOpsPerMs += result;
    }

    cout << "ops/ms      = " << totalOpsPerMs << endl;
    hazelcastInstance.shutdown();

    return 0;
}