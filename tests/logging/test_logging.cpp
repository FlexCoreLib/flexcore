
#if !defined(__clang__)
#define BOOST_ALL_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <flexcore/utils/logging/logger.hpp>
#include <tests/nodes/owning_node.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(test_logging)


using fc::logger;

namespace
{
struct log_test
{
	log_test()
	    : handle(
	          logger::get().add_stream_log(stream, logger::flush::true_, logger::cleanup::true_))
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
}

BOOST_FIXTURE_TEST_CASE( stream_logging, log_test )
{
	fc::log_client client;
	expected_in_output = "test log entry.";
	client.write(expected_in_output);
}

BOOST_FIXTURE_TEST_CASE( node_logging, log_test )
{
	fc::graph::connection_graph dummy_graph;
	auto node_name = std::string("test node");
	fc::tests::owning_node node(node_name);
	BOOST_CHECK_EQUAL(node.node().name(), node_name);
	fc::log_client client{node.node()};
	expected_in_output = node_name;
	client.write("another log message.");
}

BOOST_AUTO_TEST_CASE( stream_log_msg )
{
	log_test t1, t2, t3;
	fc::stream_log_client stream({}, fc::level::debug);
	auto msg1 = std::string("log message");
	auto msg2 = std::string("and another");
	stream << msg1 << msg2;
	t1.expected_in_output = msg1;
	t2.expected_in_output = msg2;
	t3.expected_in_output = "<" + std::to_string(static_cast<int>(fc::level::debug)) + ">";
}

namespace
{
std::ostringstream global_stream;
} // namespace

BOOST_AUTO_TEST_CASE( stream_no_cleanup )
{
	global_stream.str(std::string{});
	logger::get().add_stream_log(global_stream, logger::flush::true_, logger::cleanup::false_);
	fc::log_client client;
	auto msg = "message!";
	client.write(msg);
	auto str = global_stream.str();
	BOOST_CHECK_NE(str.find(msg), std::string::npos);
	BOOST_TEST_MESSAGE(str);
}

BOOST_AUTO_TEST_CASE( syslog_logging_works )
{
	logger::get().add_syslog_log("my program");
	// no way to check the syslog
	fc::log_client client;
	client.write("writing to syslog.");
	BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE( file_logging_works )
{
	logger::get().add_file_log("./localfile.txt");
	fc::log_client client;
	client.write("writing to localfile.");
	BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE( log_client_copy_and_move )
{
	fc::log_client client;
	{
		auto client2 = client;
		client = client2;
		auto client3 = std::move(client2);
	}
	client.write("second");
	BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()

#endif // !defined(__clang__)
