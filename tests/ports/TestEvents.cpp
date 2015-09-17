/*
 * TestEvents.h
 *
 *  Created on: Sep 15, 2015
 *      Author: ckielwein
 */

#ifndef TESTS_CORE_TESTEVENTS_CPP_
#define TESTS_CORE_TESTEVENTS_CPP_

#include <boost/test/unit_test.hpp>

#include "ports/event_ports.hpp"
#include "core/connection.hpp"

using namespace fc;

template<class T>
struct event_sink
{
	void operator()(T in)
	{
		*storage = in;
	}
	std::shared_ptr<T> storage = std::shared_ptr<T>(new T);
};

namespace fc{

template<class T>
struct is_event_sink<event_sink<T>> : public std::true_type
{
};
}

BOOST_AUTO_TEST_CASE(test_event_connections)
{
	event_out_port<int> test_event;
	event_sink<int> test_handler;

	connect(test_event, test_handler);
	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 1);


	auto tmp_connection = test_event >> [](int i){return ++i;};
	tmp_connection >> test_handler;

	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 2);

	auto incr = [](int i){return ++i;};
	test_event >> incr >> incr >> incr >> test_handler;
	test_event.fire(1);
	BOOST_CHECK_EQUAL(*(test_handler.storage), 4);
}

#endif /* TESTS_CORE_TESTEVENTS_CPP_ */
