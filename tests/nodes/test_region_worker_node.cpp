#include <boost/test/unit_test.hpp>

#include <core/connectables.hpp>
#include <nodes/region_worker_node.hpp>
#include <ports/ports.hpp>
#include <threading/parallelregion.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_region_worker)

struct triggered_counter : public region_worker_node
{
public:
	triggered_counter(std::string name, std::shared_ptr<parallel_region> parent_region)
		: region_worker_node([this](){out_event_source.fire(++work_counter);},
				parent_region, name)
		, out_event_source(this)
		, work_counter(0)
	{
	}

	event_source<int> out_event_source;
	int work_counter;
};

struct container_sink
{
	void operator()(int in) { *storage = in; }
	std::shared_ptr<int> storage = std::make_shared<int>();
};

BOOST_AUTO_TEST_CASE(test_worker)
{
	auto region = std::make_shared<parallel_region>("MyRegion");

	triggered_counter function_gen("Counter", region);

	container_sink sink;

	function_gen.out_event_source >> sink;

	int my_counter = 0;

	for (int i = 0; i<5; ++i)
	{
		region->ticks.work.fire();
		BOOST_CHECK_EQUAL(*(sink.storage), ++my_counter);
	}
}
BOOST_AUTO_TEST_SUITE_END()
