#include <flexcore/infrastructure.hpp>
#include <flexcore/scheduler/parallelscheduler.hpp>

namespace fc
{

std::shared_ptr<parallel_region> infrastructure::add_region(const std::string& name,
		const virtual_clock::steady::duration& tick_rate)
{
	auto region = std::make_shared<parallel_region>(name);

	auto work_tick = region->ticks.in_work();
	// tick_cycle captures region to prevent dangling reference after return
	// (if no one stores the region in a variable).
	// TODO: evaluate storing the region in a vector inside infrastructure.
	auto tick_cycle = fc::thread::periodic_task(
			[work_tick,region](){ work_tick(); });

	tick_cycle.out_switch_tick() >> region->ticks.in_switch_buffers();
	scheduler.add_task(std::move(tick_cycle),tick_rate);

	return region;
}

infrastructure::infrastructure() :
	// abstract_graph(),
		scheduler(std::make_unique<fc::thread::parallel_scheduler>()),
		forest_root("root",
				add_region("root_region", thread::cycle_control::medium_tick))
{
}

infrastructure::~infrastructure()
{
	stop_scheduler();
}

void infrastructure::iterate_main_loop()
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(0.5s);
	if (auto ex = scheduler.last_exception())
	{
		std::rethrow_exception(ex);
	}
}

void infrastructure::infinite_main_loop()
{
	while( true )
	{
		iterate_main_loop();
	}
}

} /* namespace fc */
