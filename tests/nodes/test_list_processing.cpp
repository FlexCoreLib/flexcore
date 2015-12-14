#ifndef TESTS_NODES_TEST_LIST_PROCESSING_CPP_
#define TESTS_NODES_TEST_LIST_PROCESSING_CPP_

#include <boost/test/unit_test.hpp>

#include "nodes/list_processing.hpp"
#include "ports/event_ports.hpp"
#include "core/connection.hpp"

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_list_processing )

BOOST_AUTO_TEST_CASE( event_in_port_experimental )
{
	range_size get_size;
	int storage = 0;
	get_size.out >> [&](int i) { storage = i; };

	get_size.in()(std::list<float>{1., 2., .3});
	BOOST_CHECK_EQUAL(storage, 3);

	get_size.in()(std::vector<int>{0, 1});
	BOOST_CHECK_EQUAL(storage, 2);
}

BOOST_AUTO_TEST_SUITE_END()

#endif /* TESTS_NODES_TEST_LIST_PROCESSING_CPP_ */
