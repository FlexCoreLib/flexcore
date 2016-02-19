#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <logging/logger.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <sstream>

BOOST_AUTO_TEST_CASE( stream_logging )
{
	std::ostringstream stream;
	auto* logger = fc::logger::get();
	auto handle =
	    logger->add_stream_log(stream, fc::logger::flush::true_, fc::logger::cleanup::true_);
	boost::log::sources::logger lg;
	auto msg = "test log entry.";
	BOOST_LOG(lg) << msg;
	auto str = stream.str();
	BOOST_CHECK_NE(str.find(msg), std::string::npos);
}
