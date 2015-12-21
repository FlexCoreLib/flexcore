#include <boost/test/unit_test.hpp>

#include <nodes/node_interface.hpp>

// std
#include <memory>

using namespace fc;

namespace // unnamed
{
template<class data_t>
struct node_class : public node_interface
{
	node_class(data_t a)
		: node_interface("test_node")
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

BOOST_AUTO_TEST_SUITE( test_node_interface )

/*
 * Confirm region propagation to child
 */
BOOST_AUTO_TEST_CASE( test_region_propagation )
{
	std::shared_ptr<node_interface::forest_t> forest = std::make_shared<node_interface::forest_t>();
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	auto child = root.add_child("child", new null);

	BOOST_CHECK(child->region().get_id() == region->get_id());
}

/*
 * Confirm full names
 */
BOOST_AUTO_TEST_CASE( test_name_chaining )
{
	std::shared_ptr<node_interface::forest_t> forest = std::make_shared<node_interface::forest_t>();
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	auto child1 = root.add_child("1", new null);
	auto child2 = root.add_child("2", new null);
	auto child1a = child1->add_child("a", new null);

	BOOST_CHECK_EQUAL(child1->full_name(), "root/1");
	BOOST_CHECK_EQUAL(child2->full_name(), "root/2");
	BOOST_CHECK_EQUAL(child1a->full_name(), "root/1/a");
}

BOOST_AUTO_TEST_CASE( test_make_child )
{
	root_node root("root");
	auto child1 = root.make_child<node_class<int>>(5);
	auto child2 = root.make_child_n<node_class<int>>("name", 5);

	BOOST_CHECK_EQUAL(child1->full_name(), "root/test_node");
	BOOST_CHECK_EQUAL(child2->full_name(), "root/name");

	auto child3 = root.make_child<node_class>(5);

	BOOST_CHECK_EQUAL(child3->full_name(), "root/name");
}

BOOST_AUTO_TEST_SUITE_END()



