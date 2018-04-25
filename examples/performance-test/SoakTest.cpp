#include <iostream>
#include <vector>
#include <memory>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <hazelcast/client/HazelcastAll.h>
#include <hazelcast/client/query/BetweenPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <hazelcast/client/serialization/ObjectDataOutput.h>
#include <hazelcast/client/serialization/ObjectDataInput.h>
#include <hazelcast/client/exception/ProtocolExceptions.h>

using namespace hazelcast;
using namespace hazelcast::client;
using namespace std;

class UpdateEntryProcessor : public client::serialization::IdentifiedDataSerializable {
public:
    UpdateEntryProcessor(const string &value) : value(new string(value)) {}

    virtual int getFactoryId() const {
        return 66;
    }

    virtual int getClassId() const {
        return 1;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(value.get());
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        throw client::exception::IllegalStateException(
                "UpdateEntryProcessor readData should not be called at client side");
    }

private:
    auto_ptr<string> value;
};

util::AtomicBoolean isCancelled;
util::Mutex threadLock;

void testMap(util::ThreadArgs &args) {
    IMap<string, string> *map = (IMap<string, string> *) args.arg0;

    {
        util::LockGuard guard(threadLock);
        cout << "Thread " << util::getThreadId() << " started." << endl;

    }

    int64_t getCount = 0;
    int64_t putCount = 0;
    int64_t valuesCount = 0;
    int64_t executeOnKeyCount = 0;
    int entryCount = 10000;

    while (!isCancelled) {
        std::ostringstream out;
        int operation = rand() % 100;
        int randomKey = rand() % entryCount;
        out << randomKey;
        string key(out.str());
        try {
            if (operation < 30) {
                map->get(key);
                ++getCount;
            } else if (operation < 60) {
                out.clear();
                out << rand();
                map->put(key, out.str());
                ++putCount;
            } else if (operation < 80) {
                map->values(query::BetweenPredicate<int>(query::QueryConstants::getValueAttributeName(), 1, 10));
                ++valuesCount;
            } else {
                UpdateEntryProcessor processor(out.str());
                map->executeOnKey<string, UpdateEntryProcessor>(key, processor);
                ++executeOnKeyCount;
            }

            int64_t totalCount = putCount + getCount + valuesCount + executeOnKeyCount;
            if (totalCount % 10000 == 0) {
                util::LockGuard guard(threadLock);
                cout << "Thread " << util::getThreadId() << " --> Total:" << totalCount << ":{getCount:" << getCount
                     << ", putCount:" << putCount << ", valuesCount:" << valuesCount << ", executeOnKeyCount:"
                     << executeOnKeyCount << "}" << endl;
            }
        } catch (std::exception &e) {
            util::LockGuard guard(threadLock);
            cout << "Thread " << util::getThreadId() << " --> Exception occured: " << e.what() << endl;
        }
    }

    {
        int64_t totalCount = putCount + getCount + valuesCount + executeOnKeyCount;
        util::LockGuard guard(threadLock);
        cout << "Thread " << util::getThreadId() << " is ending." << " --> Total:" << totalCount << ":{getCount:"
             << getCount << ", putCount:" << putCount << ", valuesCount:" << valuesCount << ", executeOnKeyCount:"
             << executeOnKeyCount << "}" << endl;
    }
}

void signalHandler(int s) {
    cout << "Caught signal: " << s << endl;
    isCancelled = true;
}

void registerSignalHandler() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char *args[]) {

    if (argc != 3) {
        cerr << "USAGE: SoakTest threadCount address" << endl;
        return -1;
    }

    const int threadCount = atoi(args[1]);
    cout << "threadCount = " << threadCount << endl;
    const string address = args[2];
    cout << "address = " << address << endl;

    ClientConfig config;
    config.addAddress(Address(address, 5701));

    HazelcastClient hazelcastInstance(config);
    IMap<string, string> map = hazelcastInstance.getMap<string, string>("test");

    registerSignalHandler();

    vector<util::Thread *> threads(threadCount);

    for (int i = 0; i < threadCount; i++) {
        threads[i] = new util::Thread(testMap, &map);
    }

    for (int i = 0; i < threadCount; i++) {
        threads[i]->join();
    }
    hazelcastInstance.shutdown();

    return 0;
}