#include <hazelcast/client/hazelcast_client.h>

using namespace hazelcast::client;

hazelcast_client hz = hazelcast::new_client().get();

void signal_handler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    // shutdown the client;
    hz.shutdown().get();

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

int main( int argc, char** argv )
{
    // add signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSTOP, signal_handler);

    auto map = hz.get_map("issue_900_map").get();
    auto q = hz.get_queue("issue_900_queue").get();
    
    map->add_entry_listener(entry_listener().on_added([](entry_event &&event) {
        std::cout << "Map entry added for key " << *event.get_key().get<int>() << std::endl;
    }).on_updated([](entry_event &&event) {
        std::cout << "Map entry updated for key " << *event.get_key().get<int>() << std::endl;
    }), true).get();

    q->add_item_listener(item_listener().on_added([] (item_event &&event) {
        std::cout << "Queue entry added: " << *event.get_item().get<int>() << std::endl;
    }), true);

    while (true) {
        try {
            auto key = rand() % 1000;
            map->put(key, 2 * key).get();
            q->offer(key).get();
            std::cout << "Put key " << key << " into the map and the queue\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (std::exception &e) {
            std::cout << "Exception during map put or queue offer. " << e.what() <<std::endl;
            break;
        }
    }

	return EXIT_SUCCESS;
}
