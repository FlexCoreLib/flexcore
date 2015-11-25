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
		: in(*this)
		, out()
	{}
	event_in_tmpl<range_size> in;
	event_out_port<int> out;

	/**
	 * to be used by ports, should be considered private
	 */
	template<class tag_t, class event_t>
	void detail_in(const event_t& event)
	{
		size_t elems = std::distance(std::begin(event), std::end(event));
		out.fire(static_cast<int>(elems));
	}
};

} // namespace fc

#endif /* SRC_NODES_LIST_PROCESSING_HPP_ */
