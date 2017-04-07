#ifndef SRC_VISUALIZATION_HPP_
#define SRC_VISUALIZATION_HPP_

#include <flexcore/extended/base_node.hpp>

#include <iosfwd>
#include <memory>

namespace fc
{

namespace graph{ class connection_graph; }

/**
 * \brief Prints the flexcore graph in graphviz format to a stream.
 *
 * The printed graph consists of the connection_graph and the ownership forest.
 * nodes are colored by the region they belong to and ports are modeled as graphviz ports.
 */
class visualization
{
public:
	///Constructs the visualizer with referenzes to graph and forest
	visualization(const graph::connection_graph& graph, const forest_t& forest);
	~visualization();

	///Prints graphviz format to a given stream
	void visualize(std::ostream& stream);
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
}

#endif /* SRC_VISUALIZATION_HPP_ */
