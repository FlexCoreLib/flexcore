#ifndef SRC_VISUALIZATION_HPP_
#define SRC_VISUALIZATION_HPP_

#include <ostream>

#include <flexcore/extended/base_node.hpp>
#include <flexcore/extended/graph/graph.hpp>
#include <map>

namespace fc
{

class visualization
{
public:
	visualization(const graph::connection_graph& graph, const forest_t& forest);

	void Visualize(std::ostream& stream);

private:
	void printSubgraph(typename forest_t::const_iterator node, std::ostream& stream);
	const std::string& getColor(const parallel_region* region);
	std::vector<graph::graph_properties> extractNodePorts(
			graph::graph_port_properties::unique_id nodeID);

	std::map<std::string, unsigned int> colorMap_;
	unsigned int currentColorIndex_ = 0U;
	std::set<graph::graph_properties> ports_;
	const graph::connection_graph& graph_;
	const forest_t& forest_;
};


}

#endif /* SRC_VISUALIZATION_HPP_ */
