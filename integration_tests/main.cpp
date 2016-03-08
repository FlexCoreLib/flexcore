
#include <nodes/base_node.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>

#include <threading/cyclecontrol.hpp>
#include <threading/parallelregion.hpp>
#include <ports/ports.hpp>

using namespace fc;

struct null : base_node
{
	null() : base_node("null") {}
};

auto setup_parallel_region(const std::string& name,
		const virtual_clock::steady::duration& tick,
		fc::thread::cycle_control& thread_manager)
{
	auto region = std::make_shared<fc::parallel_region>(name);

	auto tick_cycle = fc::thread::periodic_task(
			[&region]()
			{
				region->ticks.in_work()();
			},
			tick);

	tick_cycle.out_switch_tick() >> region->ticks.in_switch_buffers();
	thread_manager.add_task(std::move(tick_cycle));

	return region;
}

int main()
{

	std::cout << "Starting Dummy Solution\n";

	std::cout << "build up infrastructure \n";
	fc::thread::cycle_control thread_manager;
	auto first_region = setup_parallel_region("first_region",
			fc::thread::cycle_control::medium_tick,
			thread_manager);

	std::cout << "start building connections\n";

	using time_point = fc::wall_clock::system::time_point;
	fc::pure::event_source<time_point> source;
	first_region->ticks.work_tick()
			>> [&source](){ source.fire(fc::wall_clock::system::now()); };

	source >> [](time_point t){ return fc::wall_clock::system::to_time_t(t); }
		   >> [](time_t t) { std::cout << std::localtime(&t)->tm_sec << "\n"; };

	first_region->ticks.work_tick()
			>> [count = 0]() mutable {return count++;}
			>> [](int i) { std::cout << "counted ticks: " << i << "\n"; };


	//create a connection with region transition
	auto second_region = setup_parallel_region("region two",
			fc::thread::cycle_control::slow_tick,
			thread_manager);

	second_region->ticks.work_tick() >> [](){ std::cout << "Zonk!\n"; };

	fc::root_node root;
	auto child_a = root.make_child_named<null>("source_a")->region(first_region);
	auto child_b = root.make_child_named<null>("sink_b")->region(second_region);
	auto child_c = root.make_child_named<null>("source_c")->region(second_region);

	event_source<std::string> string_source(child_a);
	fc::event_sink<std::string> string_sink(child_b,
			[second_region](std::string in){std::cout << second_region->get_id().key << " received: " << in << "\n";});

	string_source >> string_sink;
	first_region->ticks.work_tick()
			>>	[&string_source, first_region]() mutable
				{
					string_source.fire("a magic string from " + first_region->get_id().key);
				};

	event_source<std::string> string_source_2(child_c);
	string_source_2 >> string_sink;

	graph::print(std::cout);

	thread_manager.start();

	using namespace std::chrono_literals;
	int iterations = 7;
	while (iterations--)
		std::this_thread::sleep_for(0.5s);

	return 0;
}
