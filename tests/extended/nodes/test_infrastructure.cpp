#include <boost/test/unit_test.hpp>

#include <flexcore/extended/base_node.hpp>
#include <flexcore/infrastructure.hpp>

// std
#include <memory>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_infrastructure )

struct null : tree_base_node
{
	static constexpr auto default_name = "null";
	explicit null(const tree_base_node& node)
	: tree_base_node(node) {}
};


/*
 * check if infrastructure correctly builds and assigns node and regions.
 */
BOOST_AUTO_TEST_CASE(test_buildup)
{
	infrastructure test_is;

	auto region = test_is.add_region("test_region",thread::cycle_control::fast_tick);
	BOOST_CHECK(region != nullptr);

	auto& test_node = test_is.node_owner().make_child<null>(region);
	BOOST_CHECK(test_node.region()->get_id() == region->get_id());
	BOOST_CHECK_EQUAL(test_node.name(), "null");
}

BOOST_AUTO_TEST_SUITE_END()
