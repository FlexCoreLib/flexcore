#ifndef SRC_VISUALIZATION_HPP_
#define SRC_VISUALIZATION_HPP_

#include <flexcore/extended/base_node.hpp>

#include <iosfwd>
#include <memory>

namespace fc
{

namespace graph{ class connection_graph; }

class visualization
{
public:
	visualization(const graph::connection_graph& graph, const forest_t& forest);
	~visualization();

	void visualize(std::ostream& stream);
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
}

#endif /* SRC_VISUALIZATION_HPP_ */
