#include <iostream>
#include <memory>
#include <string>

#include <boost/thread.hpp>

#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/topic/reliable_listener.h>

class HazelcastReliableEventConnectorImpl
{
public:
	HazelcastReliableEventConnectorImpl( std::shared_ptr<hazelcast::client::hazelcast_client> hazelcast_client )
	{
		hazelcast_client_ = hazelcast_client;
	}

	~HazelcastReliableEventConnectorImpl()
	{
		std::cout << "~HazelcastReliableEventConnectorImpl" << std::endl;
	};

	void subscribe_reliable_topic( const std::string& topicName )
	{
		std::shared_ptr<hazelcast::client::reliable_topic> rtopic = hazelcast_client_.get()->get_reliable_topic( topicName ).get();

		rtopic->add_message_listener(
			hazelcast::client::topic::reliable_listener( false ).on_received( []( hazelcast::client::topic::message&& message )
			{
			    	std::cout << "Received: " << message.get_message_object().get<std::string>() << std::endl;
			} )
		);
	}

	void subscribe_topic( const std::string& topicName )
	{
		auto topic = hazelcast_client_.get()->get_topic( topicName ).get();

		topic->add_message_listener(
			hazelcast::client::topic::listener().on_received( []( hazelcast::client::topic::message&& message ) 
			{
				std::cout    << "Received: " << message.get_message_object().get<std::string>() << std::endl;
			} )
		).get();
	}

private:
	std::shared_ptr<hazelcast::client::hazelcast_client> hazelcast_client_;
};

std::shared_ptr<hazelcast::client::hazelcast_client> hazelcast_client;

HazelcastReliableEventConnectorImpl* reliable_event_connector;

void ListenReliableTopicV1() /* quickly ends with deadlock, after some events are received */
{
	boost::chrono::milliseconds loop_period_ms( 1000 );

	reliable_event_connector->subscribe_reliable_topic( "rtopic_string" );

	std::cout << "Subscribed" << std::endl;

	while( 1 )
	{
		boost::this_thread::sleep_for( loop_period_ms );
	}
}

void ListenReliableTopicV2() /* no deadlock */
{
	boost::chrono::milliseconds loop_period_ms( 1000 );

	std::shared_ptr<hazelcast::client::reliable_topic> rtopic = hazelcast_client.get()->get_reliable_topic( "rtopic_string" ).get();

	rtopic->add_message_listener(
		hazelcast::client::topic::reliable_listener( false ).on_received( []( hazelcast::client::topic::message&& message )
		{
			std::cout << "Received: " << message.get_message_object().get<std::string>() << std::endl;
		} )
	);

	std::cout << "Subscribed" << std::endl;

	while( 1 )
	{
		boost::this_thread::sleep_for( loop_period_ms );
	}
}

void ListenReliableTopicV3() /* same as V1 but with topic instead of reliable_topic => no deadlock */
{
	boost::chrono::milliseconds loop_period_ms( 1000 );

	reliable_event_connector->subscribe_topic( "topic_string" );

	std::cout << "Subscribed" << std::endl;

	while( 1 )
	{
		boost::this_thread::sleep_for( loop_period_ms );
	}
}

int main( int argc, char** argv )
{
	if( argc < 2 )
	{
		std::cout << "Usage: app <version> (where version is either V1, V2 or V3)" << std::endl;
		return EXIT_FAILURE;
	}

	hazelcast::client::hazelcast_client tmp = hazelcast::new_client().get();
	hazelcast_client = std::make_shared<hazelcast::client::hazelcast_client>( tmp );

	std::cout << "Connected to IMDG" << std::endl;

	reliable_event_connector = new HazelcastReliableEventConnectorImpl( hazelcast_client );

	// Start threads
	boost::thread_group thread_group;

	if( strcmp( "V1", argv[1] ) == 0 )
	{
		thread_group.create_thread( ListenReliableTopicV1 );
	}
	else if( strcmp( "V2", argv[1] ) == 0 )
	{
		thread_group.create_thread( ListenReliableTopicV2 );
	}
	else if( strcmp( "V3", argv[1] ) == 0 )
	{
		thread_group.create_thread( ListenReliableTopicV3 );
	}
	else
	{
		std::cout << "Usage: app <version> (where version is either V1, V2 or V3)" << std::endl;
		return EXIT_FAILURE;
	}

	thread_group.join_all();

	return EXIT_SUCCESS;
}
