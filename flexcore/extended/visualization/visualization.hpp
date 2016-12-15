#ifndef SRC_VISUALIZATION_HPP_
#define SRC_VISUALIZATION_HPP_

#include <ostream>

#include "flexcore/extended/base_node.hpp"

namespace fc
{

namespace graph { class connection_graph; }

class visualization
{
public:
	void Visualize(std::ostream& stream, const graph::connection_graph& graph,
				   const forest_t& forest) const;

private:
	void printSubgraph(typename forest_t::const_iterator node, std::ostream& stream,
					   const graph::connection_graph& graph) const;
};


}

#endif /* SRC_VISUALIZATION_HPP_ */
