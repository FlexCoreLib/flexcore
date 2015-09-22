#ifndef SRC_PORTS_STREAMS_STREAM_SINK_HPP_
#define SRC_PORTS_STREAMS_STREAM_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <core/traits.hpp>
#include <core/connection.hpp>

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
		: con(std::make_shared<std::function<data_t()>>())
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

// traits
template<class T> struct is_active_sink<stream_sink<T> > : public std::true_type {};

}  // namespace fc

#endif /* SRC_PORTS_STREAMS_STREAM_SINK_HPP_ */
