#ifndef SRC_LOGGING_LOGGER_HPP_
#define SRC_LOGGING_LOGGER_HPP_

#include <functional>
#include <ostream>
#include <string>

namespace fc
{

class stream_handle
{
public:
	stream_handle(std::function<void()> deleter);
	~stream_handle();
private:
	std::function<void()> deleter;
};

/**
 * \brief A class for writing messages to multiple logs.
 */
class logger
{
public:
	/// get the singleton instance of logger.
	static logger* get();

	/// add log files of the specified type
	void add_file_log(std::string filename);
	void add_syslog_log(std::string progname);

	enum class flush { true_ = true, false_ = false };
	enum class cleanup { true_ = true, false_ = false };
	stream_handle add_stream_log(std::ostream& stream, logger::flush flush,
	                             logger::cleanup cleanup);

private:
	logger();
};

} // namespace fc
#endif // SRC_LOGGING_LOGGER_HPP_

