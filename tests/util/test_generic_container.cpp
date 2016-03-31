#include <boost/test/unit_test.hpp>

#include <util/settings_container.hpp>
#include <ports/ports.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_container)

BOOST_AUTO_TEST_CASE(test_storage)
{
	generic_container gc;
	pure::event_source<int> source;

	source
		>> gc.add<pure::event_sink<int>>([](int in){BOOST_CHECK_EQUAL(in, 3);});

	source.fire(3);
}

BOOST_AUTO_TEST_SUITE_END()
