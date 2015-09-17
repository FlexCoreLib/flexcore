#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_

// std
#include <memory>

namespace fc
{

/**
 * Simple StreamSource implementation that holds a stream_state.
 * Can be connected to any number of SinkConnections.
 * Is a SourceConnection.
 */
template<class data_t>
class stream_state
{
public:
	stream_state(data_t d_) : d(new data_t(d_)){}

	/// pull data
	data_t operator()() { return *d; }

	/// set current value
	void set(data_t d_) { *d = d_; }

private:
	std::shared_ptr<data_t> d;
};


template<class data_t>
struct is_source_port<stream_state<data_t>> : std::true_type
{
};
} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_ */
