/*
 * triggered_node.hpp
 *
 *  Created on: Feb 24, 2016
 *      Author: jschwan
 */

#ifndef SRC_NODES_REGION_WORKER_NODE_HPP_
#define SRC_NODES_REGION_WORKER_NODE_HPP_

#include <flexcore/scheduler/parallelregion.hpp>
#include <flexcore/extended/base_node.hpp>
#include <flexcore/core/connection.hpp>

namespace fc
{

class region_worker_node : public tree_base_node
{
public:
	template <class action_t>
	region_worker_node(action_t&& action, const detail::node_args& node)
	    : tree_base_node(node)
	{
		region()->work_tick() >> std::forward<action_t>(action);
	}

};


} //namespace fc
#endif /* SRC_NODES_REGION_WORKER_NODE_HPP_ */
