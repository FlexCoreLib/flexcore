#ifndef SRC_INFRASTRUCTURE_HPP_
#define SRC_INFRASTRUCTURE_HPP_

#include <flexcore/extended/base_node.hpp>
#include <flexcore/scheduler/cyclecontrol.hpp>

namespace fc
{

class infrastructure
{
public:
	infrastructure();
	~infrastructure();

	std::shared_ptr<parallel_region> add_region(const std::string& name,
			const virtual_clock::steady::duration& tick_rate);

	owning_base_node& node_owner() { return forest_root.nodes(); }
	graph::connection_graph& get_graph() { return graph; }
	void infinite_main_loop();
	void start_scheduler() { scheduler.start(); }
	void stop_scheduler() { scheduler.stop(); }
	void iterate_main_loop();

private:
	thread::cycle_control scheduler;
	graph::connection_graph graph;
	forest_owner forest_root;
};

} /* namespace fc */

#endif /* SRC_INFRASTRUCTURE_HPP_ */
