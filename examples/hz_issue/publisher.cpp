#include <iostream>

#include <boost/thread.hpp>

#include <hazelcast/client/hazelcast_client.h>

std::shared_ptr<hazelcast::client::hazelcast_client> hazelcast_client;

void publish_string_rtopic( const int& stamp )
{
	std::shared_ptr<hazelcast::client::reliable_topic> rtopic = hazelcast_client.get()->get_reliable_topic( "rtopic_string" ).get();

	std::string str = "foo" + std::to_string( stamp );
	rtopic->publish( str ).get();
	std::cout << "Published:" << str << std::endl;
}

void publish_string( const int& stamp )
{
	auto topic = hazelcast_client.get()->get_topic( "topic_string" ).get();

	std::string str = "foo" + std::to_string( stamp );
	topic->publish( str ).get();
	std::cout << "Published:" << str << std::endl;
}

int main( int argc, char** argv )
{
	hazelcast::client::hazelcast_client tmp = hazelcast::new_client().get();
	hazelcast_client = std::make_shared<hazelcast::client::hazelcast_client>( tmp );

	int stamp( 0 );

	if( argc > 1 )
	{
		boost::chrono::milliseconds loop_period_ms( 1000 );

		if( strcmp( "topic", argv[1] ) == 0 )
		{
			while( 1 )
			{
				publish_string( ++stamp );
				boost::this_thread::sleep_for( loop_period_ms );
			}
		}
		else if( strcmp( "rtopic", argv[1] ) == 0 )
		{
			while( 1 )
			{
				publish_string_rtopic( ++stamp );
				boost::this_thread::sleep_for( loop_period_ms );
			}
		}
	}

	return EXIT_SUCCESS;
}
