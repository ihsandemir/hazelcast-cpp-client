#include <hazelcast/client/hazelcast_client.h>

int main( int argc, char** argv )
{
    using namespace hazelcast::client;

    hazelcast_client client = hazelcast::new_client().get();

    auto map = client.get_map("issue_900_map").get();
    auto q = client.get_queue("issue_900_queue").get();

    while (true) {
        auto key = rand() % 1000;
        map->put(key, 2 * key).get();
        q->offer(key).get();
        std::cout << "Put key " << key << " into the map and the queue\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return EXIT_SUCCESS;
}
