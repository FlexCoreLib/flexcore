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
template<class action>
struct checked_destruction : public base_node
{
	checked_destruction(action a)
		: base_node("checked_destruction")
		, action_(a)
	{}

	virtual ~checked_destruction()
	{
		action_();
	}

	action action_;
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
	auto child = root.make_child_named<null>("child");

	BOOST_CHECK(child->region().get_id() == region->get_id());
}

/*
 * Confirm full names
 */
BOOST_AUTO_TEST_CASE( test_name_chaining )
{
	std::shared_ptr<region_info> region = std::make_shared<parallel_region>("foo");
	root_node root("root", region);
	auto child1 = root.make_child_named<null>("1");
	auto child2 = root.make_child_named<null>("2");
	auto child1a = child1->make_child_named<null>("a");

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

void set_flag(bool& f)
{
	// every node shall only be destroyed once!
	BOOST_CHECK_EQUAL(f, false);
	f = true;
}
template<class T> using node_t = checked_destruction<T>;

/*
 * confirm that erase_child and erase_children_by_name do
 * what they are supposed to do.
 */
BOOST_AUTO_TEST_CASE( test_explicit_erase )
{
	root_node root("root");
	bool flag_a    = false;
	bool flag_b1   = false;
	bool flag_b2   = false;
	bool flag_b2_d = false;
	bool flag_b2_b = false;
	bool flag_c    = false;
	bool flag_c_f  = false;
	auto child_a    = root.     make_child_named<node_t>( "a", [&](){ set_flag(flag_a   ); } );
					  root.     make_child_named<node_t>( "b", [&](){ set_flag(flag_b1  ); } );
	auto child_b2   = root.     make_child_named<node_t>( "b", [&](){ set_flag(flag_b2  ); } );
					  child_b2->make_child_named<node_t>( "d", [&](){ set_flag(flag_b2_d); } );
					  child_b2->make_child_named<node_t>( "b", [&](){ set_flag(flag_b2_b); } );
	auto child_c    = root.     make_child_named<node_t>( "c", [&](){ set_flag(flag_c   ); } );
		    		  child_c-> make_child_named<node_t>( "f", [&](){ set_flag(flag_c_f ); } );

	/*
	 * confirm single child erasing works
	 */
	root.erase_child(child_a);
	BOOST_CHECK_EQUAL(flag_a, true);

	/*
	 * confirm erasing by name works and grandchildren are also erased
	 */
	root.erase_children_by_name("b");
	BOOST_CHECK_EQUAL(flag_b1, true);
	BOOST_CHECK_EQUAL(flag_b2, true);
	BOOST_CHECK_EQUAL(flag_b2_d, true);
	BOOST_CHECK_EQUAL(flag_b2_b, true);

	/*
	 * confirm grandchildren with the mathing name are not erased
	 */
	root.erase_children_by_name("f");
	BOOST_CHECK_EQUAL(flag_c_f, false);

	/*
	 * confirm erase_child also erased grandchildren
	 */
	root.erase_child(child_c);
	BOOST_CHECK_EQUAL(flag_c, true);
	BOOST_CHECK_EQUAL(flag_c_f, true);
}

BOOST_AUTO_TEST_SUITE_END()



