#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <logging/logger.hpp>
#include <sstream>

using fc::logger;

struct log_test
{
	log_test()
	    : handle(
	          logger::get()->add_stream_log(stream, logger::flush::true_, logger::cleanup::true_))
	{
	}
	~log_test()
	{
		auto str = stream.str();
		BOOST_CHECK_NE(str.find(expected_in_output), std::string::npos);
		BOOST_TEST_MESSAGE(str);
	}
	std::ostringstream stream;
	std::string expected_in_output;
	fc::stream_handle handle;
};

BOOST_FIXTURE_TEST_CASE( stream_logging, log_test )
{
	fc::log_client client;
	expected_in_output = "test log entry.";
	client.write(expected_in_output);
}

BOOST_FIXTURE_TEST_CASE( region_logging, log_test )
{
	auto region_name = "test region";
	auto region = std::make_shared<fc::parallel_region>(region_name);
	BOOST_CHECK_EQUAL(region->get_id().key, region_name);
	fc::log_client client{region.get()};
	expected_in_output = region_name;
	client.write("another log message.");
}

