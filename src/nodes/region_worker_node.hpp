/*
 * triggered_node.hpp
 *
 *  Created on: Feb 24, 2016
 *      Author: jschwan
 */

#ifndef SRC_NODES_REGION_WORKER_NODE_HPP_
#define SRC_NODES_REGION_WORKER_NODE_HPP_

#include <nodes/base_node.hpp>
#include <threading/parallelregion.hpp>

namespace fc
{

class region_worker_node : public tree_base_node
{
public:
	region_worker_node(std::string name, parallel_region& parent_region) :
		tree_base_node(name)
	{
		parent_region.work_tick() >>  std::function<void(void)>(std::bind(&region_worker_node::DoWork, this));
	}

	virtual ~region_worker_node()
	{
	}

protected:
	virtual void DoWork() = 0;
};


} //namespace fc
#endif /* SRC_NODES_REGION_WORKER_NODE_HPP_ */
