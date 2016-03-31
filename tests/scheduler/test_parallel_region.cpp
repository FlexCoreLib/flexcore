#include <scheduler/parallelregion.hpp>
#include <boost/test/unit_test.hpp>

#include <pure/pure_ports.hpp>

using namespace fc;

namespace unit_test
{
class parallel_tester
{
public:
	static void switch_tick(std::shared_ptr<parallel_region> region)
	{
		region->ticks.in_switch_buffers()();
	}
	static void work_tick(std::shared_ptr<parallel_region> region)
	{
		region->ticks.in_work()();
	}
};
}

BOOST_AUTO_TEST_CASE(test_region_ticks)
{
	auto region = std::make_shared<parallel_region>();

	bool work_ticked = false;
	bool switch_ticked = false;

	region->switch_tick() >> [&](){switch_ticked = !switch_ticked;};
	region->work_tick() >> [&](){work_ticked = !work_ticked;};
	BOOST_CHECK(!switch_ticked);
	BOOST_CHECK(!work_ticked);

	::unit_test::parallel_tester::switch_tick(region);
	BOOST_CHECK(switch_ticked);
	BOOST_CHECK(!work_ticked);

	::unit_test::parallel_tester::work_tick(region);
	BOOST_CHECK(switch_ticked);
	BOOST_CHECK(work_ticked);
}
