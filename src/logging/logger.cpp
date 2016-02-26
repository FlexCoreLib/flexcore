#define BOOST_ALL_DYN_LINK
#define BOOST_LOG_USE_NATIVE_SYSLOG
#include <logging/logger.hpp>
#include <threading/parallelregion.hpp>
#include <boost/utility/empty_deleter.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <ios>
#include <utility>

/**
 * Boost log has the following design:
 *
 * sink_backend - sink_frontend --\          /-- logging source
 *                                |-- core --|
 * sink_backend - sink_frontend --/          \-- logging source
 *
 * The logging source creates a *record* which contains *attributes* (lineID, Timestamp, Channel,
 * Severity, ThreadID, Message, etc.). This logging record is passed to the core which passes it on
 * to the sink_frontends.
 *
 * The sink frontends can *filter* the records and select the ones which get passed on to the
 * backends for outputting (global filtering is also possible in the core). The sink frontends also
 * do the *formatting* of the record.
 *
 * The logging sources are responsible for filling the records and their attributes. Global
 * attributes (filled automatically) can be registered in the core.
 *
 * Sinks need to be registered in the core, sources can be directly created.
 *
 * With this information anyone working on this file should know where to look to change the
 * flexcore logger's behaviour.
 */
namespace fc
{

stream_handle::stream_handle(std::function<void()> deleter) : deleter(deleter)
{
}

stream_handle::~stream_handle()
{
	deleter();
}

logger& logger::get()
{
	static logger instance;
	return instance;
}

/// Print the severity level to the ostream, required for boost.log. For now this just casts the
/// value to an integer value, but it could also be converted to some string representation.
std::ostream& operator<<(std::ostream& out, fc::level severity)
{
	out << static_cast<int>(severity);
	return out;
}

using namespace boost::log;

template <class backend>
using sync_sink = sinks::synchronous_sink<backend>;

namespace
{
// keywords for accessing the specified attributes of the log record.
BOOST_LOG_ATTRIBUTE_KEYWORD(region, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", level)

/// format the log record with severity level, region information and a message.
auto get_format()
{
	namespace expr = boost::log::expressions;
	return expr::stream << "<" << severity << ">"
	                    << "[" << region << "] " << expr::smessage;
}

/// format the log record the with a timestamp and the same information as get_format().
//TODO: find out how to reuse the output of get_format().
auto get_format_with_timestamp()
{
	namespace expr = boost::log::expressions;
	return expr::stream << expr::format_date_time<attributes::utc_clock::value_type>(
	                           "TimeStamp", "%Y-%m-%d %H:%M:%S") << ": "
	                    << "<" << severity << ">"
	                    << "[" << region << "] " << expr::smessage;
}
} // anonymous namespace

void logger::add_file_log(std::string filename)
{
	// append to file, never truncate.
	auto file_sink = boost::make_shared<sinks::text_file_backend>(
	    keywords::file_name = filename,
	    keywords::open_mode = std::ios_base::app | std::ios_base::out);
	auto sink_front = boost::make_shared<sync_sink<sinks::text_file_backend>>(file_sink);
	sink_front->set_formatter(get_format_with_timestamp());
	core::get()->add_sink(sink_front);
}

void logger::add_syslog_log(std::string progname)
{
	// use the native api implementation does not use a local network connection.
	auto syslog_sink = boost::make_shared<sinks::syslog_backend>(
	    keywords::use_impl = sinks::syslog::impl_types::native,
	    keywords::ident = progname);
	auto sink_front = boost::make_shared<sync_sink<sinks::syslog_backend>>(syslog_sink);
	sink_front->set_formatter(get_format());
	core::get()->add_sink(sink_front);
}

stream_handle logger::add_stream_log(std::ostream& stream, logger::flush flush,
                                     logger::cleanup cleanup)
{
	auto stream_sink = boost::make_shared<sinks::text_ostream_backend>();
	auto shared_stream = boost::shared_ptr<std::ostream>(&stream, boost::empty_deleter());
	stream_sink->add_stream(shared_stream);
	stream_sink->auto_flush(static_cast<bool>(flush));
	auto sink_front = boost::make_shared<sync_sink<sinks::text_ostream_backend>>(stream_sink);
	sink_front->set_formatter(get_format_with_timestamp());
	core::get()->add_sink(sink_front);

	// Prepare a function that will remove the stream from the sink if cleanup is needed.
	std::function<void()> cleanup_fun;
	if (static_cast<bool>(cleanup))
		cleanup_fun = [=]() { stream_sink->remove_stream(shared_stream); };
	else
		cleanup_fun = []() {};

	return stream_handle(cleanup_fun);
}

logger::logger()
{
	// add the time stamp to every record (this does not mean that it gets output automatically).
	core::get()->add_global_attribute("TimeStamp", attributes::utc_clock{});
}

class log_client::log_client_impl
{
public:
	log_client_impl(const region_info* region)
	    : lg(keywords::channel = (region ? region->get_id().key : "(null)"))
	{
	}
	void write(const std::string& msg, level severity)
	{
		BOOST_LOG_SEV(lg, severity) << msg;
	}
private:
	sources::severity_channel_logger<level, std::string> lg;
};

void log_client::write(const std::string& msg, level severity)
{
	log_client_pimpl->write(msg, severity);
}

log_client::log_client() : log_client_pimpl(std::make_unique<log_client::log_client_impl>(nullptr))
{
}

log_client::log_client(const region_info* region)
    : log_client_pimpl(std::make_unique<log_client::log_client_impl>(region))
{
}

log_client::log_client(const log_client& other)
    : log_client_pimpl(std::make_unique<log_client::log_client_impl>(*other.log_client_pimpl))
{
}

// copy/move and swap idiom. For rvalues this results in two moves instead of one but unique_ptr
// moves are cheap.
log_client& log_client::operator=(log_client other)
{
	std::swap(log_client_pimpl, other.log_client_pimpl);
	return *this;
}

log_client::log_client(log_client&&) = default;
log_client::~log_client() = default;

} // namespace fc
