#include <boost/test/unit_test.hpp>


#include <core/connection.hpp>
#include <serialisation/configfile.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_serialisation)

BOOST_AUTO_TEST_CASE(test_configfile)
{
	std::stringstream ss;
	ss << "{ "
			"\"test_int\": 1,"
			"\"test_float\": 0.5 "
			"}";

	config_file test_config{ss};


	auto out_port = test_config.out_value<int>("test_int");
	auto out_float = test_config.out_value<double>("test_float");
	test_config.load();

	BOOST_CHECK_EQUAL(out_port(), 1);
	BOOST_CHECK_EQUAL(out_float(), 0.5);


}

BOOST_AUTO_TEST_SUITE_END()
