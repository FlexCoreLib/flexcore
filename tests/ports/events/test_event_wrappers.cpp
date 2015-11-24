/*
 * test_event_wrappers.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: jschwan
 */

#include <boost/test/unit_test.hpp>

#include "ports/event_ports.hpp"
#include "core/connection.hpp"
#include "ports/events/event_sources.hpp"
#include "ports/events/event_sinks.hpp"
#include "ports/events/event_wrappers.hpp"

#include <iostream>

using namespace fc;

template<class T>
struct wrapper_event_sink
{
	wrapper_event_sink(std::string name):
	mName(name)
	{
	}

	void operator()(T in)
	{
//		*storage = in;
		std::cout<<"Sink "<<mName<<" says "<<in<<"\n";
	}
//	std::shared_ptr<T> storage = std::make_shared<T>();
	std::string mName;
};


BOOST_AUTO_TEST_SUITE(test_event_wrappers)

BOOST_AUTO_TEST_CASE(test_wrapper_callback)
{
	event_source_wrapper<event_out_port<int>> test_source;
	event_sink_wrapper<wrapper_event_sink<int>> test_sink1("Sink 1");
	event_sink_wrapper<wrapper_event_sink<int>> test_sink4("Sink 4");
	test_source >> test_sink1;
	test_source.fire(5);

	{
		event_sink_wrapper<wrapper_event_sink<int>> test_sink2("Sink 2");
		event_sink_wrapper<wrapper_event_sink<int>> test_sink3("Sink 3");
		test_source >> test_sink2;
		test_source >> test_sink3;
		test_source.fire(6);
		test_source >> test_sink4;
		test_source.fire(7);
	}

	test_source.fire(8);

}

BOOST_AUTO_TEST_SUITE_END()



