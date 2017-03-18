#include <boost/test/unit_test.hpp>
#include <flexcore/scheduler/parallelregion.hpp>
#include <flexcore/extended/nodes/region_worker_node.hpp>
#include <flexcore/ports.hpp>

#include "nodes/owning_node.hpp"
#include <pure/sink_fixture.hpp>

BOOST_AUTO_TEST_SUITE(test_region_worker)

struct triggered_counter : public fc::region_worker_node
{
public:
	explicit triggered_counter(const fc::node_args& node)
		: region_worker_node([this](){out_event_source.fire(++work_counter);},
		                     node)
		, out_event_source(this)
		, work_counter(0)
	{
	}

	event_source<int> out_event_source;
	int work_counter;
};

BOOST_AUTO_TEST_CASE(test_worker)
{
	using fc::operator>>;
	auto region = std::make_shared<fc::parallel_region>("MyRegion");
	fc::tests::owning_node owner(region);

	triggered_counter& function_gen = owner.make_child_named<triggered_counter>("Counter");

	fc::pure::sink_fixture<int> sink{{1,2,3,4,5}};

	function_gen.out_event_source >> sink;

	for (int i = 0; i<5; ++i)
	{
		region->ticks.work.fire();
	}
}
BOOST_AUTO_TEST_SUITE_END()
