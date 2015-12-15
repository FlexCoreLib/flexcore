#ifndef SRC_NODES_LIST_PROCESSING_HPP_
#define SRC_NODES_LIST_PROCESSING_HPP_

#include "ports/event_ports.hpp"
#include "core/connection.hpp"

namespace fc
{

/**
 * \brief Node for calculating the number of elements in a range
 */
struct range_size
{
public:
	range_size()
		: out()
	{}
	event_out_port<int> out;

	auto in()
	{
		return ::fc::make_event_in_port_tmpl( [this](auto event)
		{
			size_t elems = std::distance(std::begin(event), std::end(event));
			this->out.fire(static_cast<int>(elems));
		} );
	}
};

} // namespace fc

#endif /* SRC_NODES_LIST_PROCESSING_HPP_ */
