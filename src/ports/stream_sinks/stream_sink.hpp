#ifndef SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_
#define SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <core/traits.hpp>

namespace fc
{

/**
 * Simple implementation for StreamSink.
 * Will pull data when get is called.
 * Is not a Connectable.
 */
template<class data_t>
class stream_sink
{
public:
	stream_sink()
		: con(new std::function<data_t()>())
	{ }
	stream_sink(const stream_sink& other) : con(other.con) {  }

	data_t get() { return (*con)(); }

	template<class con_t>
	void connect(con_t c)
	{
		(*con) = c;
	}

private:
	std::shared_ptr<std::function<data_t()>> con;
};

template<>
struct is_sink_port<stream_sink<int> > : public std::true_type
{
};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_ */
