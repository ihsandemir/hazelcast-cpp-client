#include <hazelcast/client/hazelcast_client.h>

using namespace hazelcast::client;

hazelcast_client client = hazelcast::new_client().get();

void signal_handler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    // shutdown the client;
    client.shutdown();

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

int main( int argc, char** argv )
{
    // add signal handler
    signal(SIGTERM, signal_handler);
    signal(SIGTERM, signal_handler);

    auto map = client.get_map("issue_900_map").get();
    auto q = client.get_queue("issue_900_queue").get();
    
    map->add_entry_listener(entry_listener().on_added([](entry_event &&event) {
        std::cout << "Map entry added for key " << *event.get_key().get<int>() << std::endl;
    }).on_updated([](entry_event &&event) {
        std::cout << "Map entry updated for key " << *event.get_key().get<int>() << std::endl;
    }), true).get();

    q->add_item_listener(item_listener().on_added([] (item_event &&event) {
        std::cout << "Queue entry added: " << *event.get_item().get<int>() << std::endl;
    }), true);

    std::this_thread::sleep_for(std::chrono::hours(5));

	return EXIT_SUCCESS;
}
