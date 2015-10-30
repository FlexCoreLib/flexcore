#ifndef SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_
#define SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_

// std
#include <memory>

#include <core/traits.hpp>

namespace fc
{

/**
 * Simple StreamSource implementation that holds a state_source_with_setter.
 * Can be connected to any number of SinkConnections.
 * Is a SourceConnection.
 */
template<class data_t>
class state_source_with_setter
{
public:
	explicit state_source_with_setter(data_t d) : d(std::make_shared<data_t>(d)){}

	/// pull data
	data_t operator()() { return *d; }

	/// set current value
	void set(data_t d_) { *d = d_; }

private:
	std::shared_ptr<data_t> d;
};

// traits
template<class data_t> struct is_passive_source<state_source_with_setter<data_t>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_STREAM_SOURCES_STREAM_STATE_HPP_ */
