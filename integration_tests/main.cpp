
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>

#include <flexcore/extended/base_node.hpp>
#include <flexcore/ports.hpp>
#include <flexcore/infrastructure.hpp>

#include <boost/scope_exit.hpp>

using namespace fc;

struct null : tree_base_node
{
	null(std::string name, std::shared_ptr<fc::parallel_region> r)
			: tree_base_node(r, name) {}
};

int main()
{

	std::cout << "Starting Dummy Solution\n";
	std::cout << "build up infrastructure \n";
	fc::infrastructure infrastructure;

	auto first_region = infrastructure.add_region(
			"first_region",
			fc::thread::cycle_control::medium_tick);

	std::cout << "start building connections\n";

	using clock = fc::virtual_clock::system;
	using time_point = clock::time_point;

	fc::pure::event_source<time_point> source;
	first_region->ticks.work_tick()
			>> [&source](){ source.fire(clock::now()); };

	source >> [](time_point t){ return clock::to_time_t(t); }
		   >> [](time_t t) { std::cout << std::localtime(&t)->tm_sec << "\n"; };

	first_region->ticks.work_tick()
			>> [count = 0]() mutable {return count++;}
			>> [](int i) { std::cout << "counted ticks: " << i << "\n"; };


	//create a connection with region transition
	auto second_region = infrastructure.add_region(
			"region two",
			fc::thread::cycle_control::slow_tick);

	second_region->ticks.work_tick() >> [](){ std::cout << "Zonk!\n"; };

	auto child_a = infrastructure.node_owner().
			make_child<null>(first_region, "source_a");
	auto child_b = infrastructure.node_owner().
			make_child<null>(second_region, "sink_b");
	auto child_c = infrastructure.node_owner().
			make_child<null>(second_region, "source_c");

	event_source<std::string> string_source(child_a);
	fc::event_sink<std::string> string_sink(child_b,
			[second_region](std::string in){std::cout << second_region->get_id().key << " received: " << in << "\n";});

	string_source >> string_sink;
	first_region->ticks.work_tick()
			>>	[&string_source, id = first_region->get_id().key]() mutable
				{
					string_source.fire("a magic string from " + id);
				};

	event_source<std::string> string_source_2(child_c);
	string_source_2 >> string_sink;

	graph::connection_graph::access().print(std::cout);

	infrastructure.start_scheduler();
	BOOST_SCOPE_EXIT(&infrastructure) {
		infrastructure.stop_scheduler();
	} BOOST_SCOPE_EXIT_END

	using namespace std::chrono_literals;
	int iterations = 7;
	while (iterations--)
	{
		infrastructure.iterate_main_loop();
	}

	return 0;
}
