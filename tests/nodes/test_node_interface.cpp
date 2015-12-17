#include <boost/test/unit_test.hpp>

#include <nodes/node_interface.hpp>

// std
#include <memory>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_node_interface )

/*
 * Confirm region propagation to child
 */
BOOST_AUTO_TEST_CASE( test_region_propagation )
{
	std::shared_ptr<node_interface::forest_t> forest = std::make_shared<node_interface::forest_t>();
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	node_interface child(&root, "child");

	BOOST_CHECK(child.region().get_id() == region->get_id());
}

/*
 * Confirm no parent and no region failes
 */
BOOST_AUTO_TEST_CASE( test_no_parent_no_region_fails )
{
	BOOST_CHECK_THROW
	(
		node_interface root(0),
		std::invalid_argument
	)
}

/*
 * Confirm full names
 */
BOOST_AUTO_TEST_CASE( test_name_chaining )
{
	std::shared_ptr<node_interface::forest_t> forest = std::make_shared<node_interface::forest_t>();
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	node_interface child1(&root, "1");
	node_interface child2(&root, "2");
	node_interface child1a(&child1, "a");

//	std::cout << "child1: " <<
	BOOST_CHECK_EQUAL(child1.full_name(), "root.1");
	BOOST_CHECK_EQUAL(child2.full_name(), "root.2");
	BOOST_CHECK_EQUAL(child1a.full_name(), "root.1.a");
}

BOOST_AUTO_TEST_SUITE_END()


