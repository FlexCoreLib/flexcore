#ifndef SRC_NODES_LIST_PROCESSING_HPP_
#define SRC_NODES_LIST_PROCESSING_HPP_

#include "ports/event_ports.hpp"
#include "core/connection.hpp"

namespace fc
{

/**
 * calculates the number of elements in a range
 */
struct range_size
{
public:
	range_size()
		: out()
	{}
	auto in()
	{
		return make_event_in_port2( [this](auto event){ this->detail_in(event); } );
	}
	event_out_port<int> out;

	/**
	 * to be used by ports, should be considered private
	 */
	template<class event_t>
	void detail_in(const event_t& event)
	{
		size_t elems = std::distance(std::begin(event), std::end(event));
		out.fire(static_cast<int>(elems));
	}
};

} // namespace fc

#endif /* SRC_NODES_LIST_PROCESSING_HPP_ */
