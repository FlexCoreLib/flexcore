#ifndef SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_
#define SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_

// std
#include <functional>
#include <memory>

// flexcore
#include <core/traits.hpp>
#include <core/connection.hpp>
#include <ports/ports.hpp>

#include <ports/stream_sources/stream_state.hpp>


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

template<class T>
struct is_stream_sink<stream_sink<T> > : public std::true_type
{
};

namespace detail
{
/**
 * Specialization of connect_impl for the case of connecting a standard connectable
 * which is not a source_port to a sink_port.
 * \return a stream_proxy which contains the source and the sink.
 */
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t, typename std::enable_if<
		(!fc::is_stream_source<source_t>::value) &&
		fc::is_stream_sink<sink_t>::value
		>::type >
{
	fc::stream_proxy<source_t, sink_t> operator()(source_t source, sink_t sink)
	{
		return fc::stream_proxy<source_t, sink_t>(source, sink);
	}
};

/**
 * Specialization for the case of connecting a source_port to a sink_port.
 * \pre source_t needs to be a stream_source.
 * \pre sink_t needs to be a stream_sink.
 * \post source is now connected to sink
 * \return nothing, the connection is complete
 */
template<class sink_t, class source_t>
struct connect_impl<sink_t, source_t, typename std::enable_if<
		fc::is_stream_source<source_t>::value &&
		fc::is_stream_sink<sink_t>::value
		>::type >
{
	void operator()(source_t source, sink_t sink)
	{
		sink.connect(source);
		return;
	}
};

} // namespace detail
}  // namespace fc

#endif /* SRC_PORTS_STREAM_SINKS_STREAM_SINK_HPP_ */
