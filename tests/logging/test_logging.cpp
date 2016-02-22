#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <logging/logger.hpp>
#include <sstream>

BOOST_AUTO_TEST_CASE( stream_logging )
{
	std::ostringstream stream;
	auto handle = fc::logger::get()->add_stream_log(stream, fc::logger::flush::true_,
	                                                fc::logger::cleanup::true_);
	fc::log_client client;
	auto msg = "test log entry.";
	client.write(msg);
	auto str = stream.str();
	BOOST_CHECK_NE(str.find(msg), std::string::npos);
}
