#include <flexcore/pure/pure_ports.hpp>
#include <flexcore/scheduler/parallelregion.hpp>
#include <flexcore/scheduler/cyclecontrol.hpp>

#include <boost/test/unit_test.hpp>

// Little hack to get access to infrastructure internals
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"       // tell gcc to ignore the unknown warning below
#pragma GCC diagnostic ignored "-Wkeyword-macro" // tell clang to ignore this warning
#define private public
#include <flexcore/infrastructure.hpp>
#undef private
#pragma GCC diagnostic pop

BOOST_AUTO_TEST_SUITE(test_parallel_region)

namespace
{
class parallel_tester
{
public:
	static void switch_tick(std::shared_ptr<fc::parallel_region> region)
	{
		region->ticks.switch_buffers();
	}
	static void work_tick(std::shared_ptr<fc::parallel_region> region)
	{
		region->ticks.in_work()();
	}
};

constexpr auto fast_tick = fc::thread::cycle_control::fast_tick;

}

using fc::operator>>;

BOOST_AUTO_TEST_CASE(test_region_ticks)
{
	auto region = std::make_shared<fc::parallel_region>("r1", fast_tick);

	bool work_ticked{false};
	bool switch_ticked{false};

	region->switch_tick() >> [&](){switch_ticked = !switch_ticked;};
	region->work_tick() >> [&](){work_ticked = !work_ticked;};
	BOOST_CHECK(!switch_ticked);
	BOOST_CHECK(!work_ticked);

	parallel_tester::switch_tick(region);
	BOOST_CHECK(switch_ticked);
	BOOST_CHECK(!work_ticked);

	parallel_tester::work_tick(region);
	BOOST_CHECK(switch_ticked);
	BOOST_CHECK(work_ticked);
}


BOOST_AUTO_TEST_CASE(test_region_cloning)
{
	fc::infrastructure infra{};
	auto region_1 = infra.add_region("region-1", fast_tick);
	auto region_2 = region_1->new_region("region-2", fast_tick);
	bool region_1_worked{false};
	bool region_2_worked{false};
	fc::pure::event_sink<void> region_1_sink{[&] { region_1_worked = true; }};
	fc::pure::event_sink<void> region_2_sink{[&] { region_2_worked = true; }};
	region_1->work_tick() >> region_1_sink;
	region_2->work_tick() >> region_2_sink;
	infra.scheduler.work();
	while (infra.scheduler.nr_of_tasks() != 0);
	infra.stop_scheduler();
	BOOST_CHECK(region_1_worked);
	BOOST_CHECK(region_2_worked);
}

BOOST_AUTO_TEST_SUITE_END()

