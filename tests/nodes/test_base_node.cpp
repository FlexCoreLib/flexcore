#include <boost/test/unit_test.hpp>

#include <nodes/base_node.hpp>

// std
#include <memory>

using namespace fc;

namespace // unnamed
{
template<class data_t>
struct node_class : public base_node
{
	node_class(data_t a)
		: base_node("test_node")
		, value(a)
	{}

	data_t get_value() { return value; }

	auto port()
	{
		return [this](){ return this->get_value(); };
	}

	data_t value;
};
} // unnamed namespace

BOOST_AUTO_TEST_SUITE( test_base_node )

/*
 * Confirm region propagation to child
 */
BOOST_AUTO_TEST_CASE( test_region_propagation )
{
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
//	auto child = root.add_child_named("child", std::make_unique<null>());
	auto child = root.add_child_named("child", std::unique_ptr<null>(new null));

	BOOST_CHECK(child->region().get_id() == region->get_id());
}

/*
 * Confirm full names
 */
BOOST_AUTO_TEST_CASE( test_name_chaining )
{
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	auto child1 = root.add_child_named("1", std::make_unique<null>());
	auto child2 = root.add_child_named("2", std::make_unique<null>());
	auto child1a = child1->add_child_named("a", std::make_unique<null>());

	BOOST_CHECK_EQUAL(child1->full_name(), "root/1");
	BOOST_CHECK_EQUAL(child2->full_name(), "root/2");
	BOOST_CHECK_EQUAL(child1a->full_name(), "root/1/a");
}

BOOST_AUTO_TEST_CASE( test_make_child )
{
	root_node root("root");
	auto child1 = root.make_child<node_class<int>>(5);
	auto child2 = root.make_child_named<node_class<int>>("name", 5);

	BOOST_CHECK_EQUAL(child1->full_name(), "root/test_node");
	BOOST_CHECK_EQUAL(child2->full_name(), "root/name");

	auto child3 = root.make_child<node_class>(5);
	auto child4 = root.make_child_named<node_class>("foo", 5);

	BOOST_CHECK_EQUAL(child3->full_name(), "root/test_node");
	BOOST_CHECK_EQUAL(child4->full_name(), "root/foo");
}

BOOST_AUTO_TEST_SUITE_END()



