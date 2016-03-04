#include <boost/test/unit_test.hpp>

#include <nodes/base_node.hpp>

// std
#include <memory>

using namespace fc;

namespace // unnamed
{
template<class data_t>
struct node_class : base_node
{
	node_class(std::shared_ptr<parallel_region> r, data_t a)
		: base_node(r, "test_node")
		, value(a)
	{}

	node_class(std::shared_ptr<parallel_region> r, std::string name, data_t a)
		: base_node(r, name)
		, value(a)
	{}


	data_t get_value() { return value; }

	auto port()
	{
		return [this](){ return this->get_value(); };
	}

	data_t value;
};

struct null : base_node
{
	explicit null(std::shared_ptr<parallel_region> r,
			std::string name = "null")
	: base_node(r, name) {}
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

	BOOST_CHECK(child->region()->get_id() == region->get_id());
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

	BOOST_CHECK_EQUAL(full_name(*child1), "root/1");
	BOOST_CHECK_EQUAL(full_name(*child2), "root/2");
	BOOST_CHECK_EQUAL(full_name(*child1a), "root/1/a");
}

BOOST_AUTO_TEST_CASE( test_make_child )
{
	root_node root("root");
	auto child1 = root.make_child<node_class<int>>(5);
	auto child2 = root.make_child_named<node_class<int>>("name", 5);

	BOOST_CHECK_EQUAL(full_name(*child1), "root/test_node");
	BOOST_CHECK_EQUAL(full_name(*child2), "root/name");

	auto child3 = root.make_child<node_class>(5);
	auto child4 = root.make_child_named<node_class>("foo", 5);

	BOOST_CHECK_EQUAL(full_name(*child3), "root/test_node");
	BOOST_CHECK_EQUAL(full_name(*child4), "root/foo");
}

namespace
{
class test_owning_node : public base_node
{
public:
	explicit test_owning_node(std::shared_ptr<parallel_region> r) : base_node(r, "test") {}
	base_node::forest_t::iterator add_child()
	{
		make_child<test_owning_node>();
		return ++adobe::trailing_of(adobe::child_begin(self_).base());
	}

	size_t nr_of_children()
	{
		return forest_->size() -2; //-1 for this. -1 for root node
	}
};
}

BOOST_AUTO_TEST_CASE( test_deletion )
{
	root_node root("root");

	auto test_node = root.make_child<test_owning_node>();

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);

	auto temp_it = test_node->add_child();
	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 1);

	erase_with_subtree(*(test_node->forest()), temp_it);
	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);

	auto temp_it_2 = test_node->add_child();
	static_cast<test_owning_node*>(temp_it_2->get())->add_child();

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 2);
	erase_with_subtree(*(test_node->forest()), temp_it);

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
