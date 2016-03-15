#ifndef SRC_INFRASTRUCTURE_HPP_
#define SRC_INFRASTRUCTURE_HPP_

#include <nodes/base_node.hpp>
#include <threading/cyclecontrol.hpp>

namespace fc
{

class infrastructure
{
public:
	infrastructure();

	std::shared_ptr<parallel_region> add_region(const std::string& name,
			const virtual_clock::steady::duration& tick);

	root_node& node_owner() { return forest_root; }
	void infinite_main_loop();
	void state_scheduler() { scheduler.start(); }
	void iterate_main_loop();

private:
//	graph::connection_graph abstract_graph;
	thread::cycle_control scheduler;
	root_node forest_root;
};

} /* namespace fc */

#endif /* SRC_INFRASTRUCTURE_HPP_ */
