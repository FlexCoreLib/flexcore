#ifndef SRC_LOGGING_LOGGER_HPP_
#define SRC_LOGGING_LOGGER_HPP_

#include <functional>
#include <memory>
#include <ostream>
#include <string>

namespace fc
{

class stream_handle
{
public:
	stream_handle(std::function<void()> deleter);

	// moveable, but not copyable
	stream_handle(const stream_handle&) = delete;
	stream_handle(stream_handle&&) = default;
	stream_handle& operator=(const stream_handle&) = delete;
	stream_handle& operator=(stream_handle&&) = default;

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

class log_client
{
public:
	void write(const std::string& msg);
	log_client();
	log_client(const log_client&);
	log_client& operator=(log_client);
	log_client(log_client&&);
	~log_client();
private:
	class log_client_impl;
	std::unique_ptr<log_client_impl> log_client_pimpl;
};

} // namespace fc
#endif // SRC_LOGGING_LOGGER_HPP_

