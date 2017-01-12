#ifndef SRC_VISUALIZATION_HPP_
#define SRC_VISUALIZATION_HPP_

#include <flexcore/extended/base_node.hpp>
#include <flexcore/extended/graph/graph.hpp>

#include <map>
#include <ostream>

namespace fc
{

class visualization
{
public:
	visualization(const graph::connection_graph& graph, const forest_t& forest);

	void visualize(std::ostream& stream);

private:
	const std::string& get_color(const parallel_region* region);
	void print_subgraph(typename forest_t::const_iterator node, std::ostream& stream);
	std::vector<graph::graph_properties> extract_node_ports(
			graph::graph_port_properties::unique_id nodeID);
	void print_ports(const std::vector<graph::graph_properties>& ports, unsigned long owner_hash,
			std::ostream& stream);

	std::map<std::string, unsigned int> color_map_;
	unsigned int current_color_index_ = 0U;
	std::set<graph::graph_properties> ports_;
	const graph::connection_graph& graph_;
	const forest_t& forest_;
};
}

#endif /* SRC_VISUALIZATION_HPP_ */
