#include <flexcore/infrastructure.hpp>
#include <flexcore/scheduler/parallelscheduler.hpp>
#include <memory>
#include <stdexcept>

namespace fc
{
namespace detail {
class scheduled_region : public fc::parallel_region
{
public:
	scheduled_region(std::string name, std::weak_ptr<region_factory> region_maker)
	    : parallel_region(std::move(name)), region_maker(region_maker)
	{
	}
	std::shared_ptr<parallel_region>
	new_region(std::string name, virtual_clock::steady::duration tick_rate) const override;

private:
	std::weak_ptr<region_factory> region_maker;
};

/** \brief Factory for creating regions and connecting them to the scheduler
 *
 * \pre region_factory needs to be held as shared_ptr.
 */
class region_factory : public std::enable_shared_from_this<region_factory>
{
public:
	region_factory(thread::cycle_control& scheduler) : scheduler(scheduler) {}

	/// Creates a new region and connects it to the scheduler with a periodic task.
	std::shared_ptr<parallel_region> new_region(const std::string& name,
	                                            const virtual_clock::steady::duration& tick_rate);

private:
	thread::cycle_control& scheduler;
};

std::shared_ptr<parallel_region>
scheduled_region::new_region(std::string name, virtual_clock::steady::duration tick_rate) const
{
	if (auto factory = region_maker.lock())
		return factory->new_region(std::move(name), tick_rate);
	else
		throw std::runtime_error{"Region factory has been destroyed already"};
}

std::shared_ptr<parallel_region>
region_factory::new_region(const std::string& name,
                           const virtual_clock::steady::duration& tick_rate)
{
	auto region = std::make_shared<scheduled_region>(name, shared_from_this());
	auto tick_cycle = fc::thread::periodic_task(region);
	scheduler.add_task(std::move(tick_cycle),tick_rate);
	return region;
}
} // namespace detail

std::shared_ptr<parallel_region>
infrastructure::add_region(const std::string& name,
                           const virtual_clock::steady::duration& tick_rate)
{
	return region_maker->new_region(name, tick_rate);
}

infrastructure::infrastructure()
    : scheduler(std::make_unique<fc::thread::parallel_scheduler>())
    , region_maker(std::make_shared<detail::region_factory>(scheduler))
    , graph()
    , forest_root(graph, "root", add_region("root_region", thread::cycle_control::medium_tick))
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
