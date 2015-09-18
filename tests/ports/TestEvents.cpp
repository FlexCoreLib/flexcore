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
	std::shared_ptr<T> storage = std::make_shared<T>();
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

template<class T>
struct event_vector_sink
{
	void operator()(T in)
	{
		storage->push_back(in);
	}
	std::shared_ptr<std::vector<T>> storage =
			std::make_shared<std::vector<T>>();
};

namespace fc{

template<class T>
struct is_event_sink<event_vector_sink<T>> : public std::true_type
{
};
}

BOOST_AUTO_TEST_CASE(merge_events)
{
	event_out_port<int> test_event;
	event_out_port<int> test_event_2;
	event_vector_sink<int> test_handler;

	test_event >> test_handler;
	test_event_2 >> test_handler;

	test_event.fire(0);
	BOOST_CHECK_EQUAL(test_handler.storage->size(), 1);
	BOOST_CHECK_EQUAL(test_handler.storage->back(), 0);

	test_event_2.fire(1);

	BOOST_CHECK_EQUAL(test_handler.storage->size(), 2);
	BOOST_CHECK_EQUAL(test_handler.storage->front(), 0);
	BOOST_CHECK_EQUAL(test_handler.storage->back(), 1);

}

BOOST_AUTO_TEST_CASE(split_events)
{
	event_out_port<int> test_event;
	event_sink<int> test_handler_1;
	event_sink<int> test_handler_2;

	test_event >> test_handler_1;
	test_event >> test_handler_2;

	test_event.fire(2);
	BOOST_CHECK_EQUAL(*(test_handler_1.storage), 2);
	BOOST_CHECK_EQUAL(*(test_handler_2.storage), 2);
}

#endif /* TESTS_CORE_TESTEVENTS_CPP_ */
