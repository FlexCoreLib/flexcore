
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

#include <threading/cyclecontrol.hpp>
#include <threading/parallelregion.hpp>
#include <unistd.h>

int main()
{

	std::cout << "Starting Dummy Solution\n";

	std::cout << "build up infrastructure \n";
	fc::thread::cycle_control thread_manager;
	auto first_region = std::make_shared<fc::parallel_region>();

	thread_manager.add_task(fc::thread::periodic_task(
			[&first_region]()
			{
				first_region->ticks.in_switch_buffers()();
				first_region->ticks.in_work()();
			},
			fc::thread::cycle_control::fast_tick));

	std::cout << "start building connections\n";

	using time_point = fc::wall_clock::system::time_point;
	fc::event_out_port<time_point> source;
	first_region->ticks.work_tick()
			>> [&source](){ source.fire(fc::wall_clock::system::now()); };

	source >> [](time_point t){ return fc::wall_clock::system::to_time_t(t); }
		   >> [](time_t t) { std::cout << std::localtime(&t)->tm_sec << "\n"; };

	unsigned int count = 0;

	first_region->ticks.work_tick()
			>> [&count](){return count++;}
			>> [](int i) { std::cout << "counted ticks: " << i << "\n"; };

	thread_manager.start();

	while (true)
		sleep(1);

	return 0;
}
