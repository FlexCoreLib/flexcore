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
	static graph::graph_port_properties::port_type merge_property_types(
			const graph::graph_properties& source_node, const graph::graph_properties& sink_node);
	std::vector<graph::graph_properties> find_node_ports(
			graph::graph_port_properties::unique_id node_id) const;
	std::vector<graph::graph_properties> find_connectables(
			graph::graph_port_properties::unique_id node_id) const;
	std::string get_color(const parallel_region* region);
	void print_subgraph(typename forest_t::const_iterator node, std::ostream& stream);
	void print_ports(const std::vector<graph::graph_properties>& ports, unsigned long owner_hash,
			std::ostream& stream);

	const graph::connection_graph& graph_;
	const forest_t& forest_;
	const std::set<graph::graph_properties>& ports_;
	std::map<std::string, unsigned int> color_map_;
	unsigned int current_color_index_ = 0U;
};
}

#endif /* SRC_VISUALIZATION_HPP_ */
