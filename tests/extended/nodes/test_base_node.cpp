#include <boost/test/unit_test.hpp>

#include <flexcore/extended/base_node.hpp>
#include <nodes/owning_node.hpp>

// std
#include <memory>

using namespace fc;

namespace // unnamed
{
template<class data_t>
struct node_class : tree_base_node
{
	node_class(data_t a, std::shared_ptr<parallel_region> r)
		: tree_base_node(r, "test_node")
		, value(a)
	{}

	node_class(data_t a, std::shared_ptr<parallel_region> r, std::string name)
		: tree_base_node(r, name)
		, value(a)
	{}


	data_t get_value() { return value; }

	auto port()
	{
		return [this](){ return this->get_value(); };
	}

	data_t value;
};

struct null : tree_base_node
{
	explicit null(std::shared_ptr<parallel_region> r,
			std::string name = "null")
	: tree_base_node(r, name) {}
};
} // unnamed namespace

BOOST_AUTO_TEST_SUITE( test_base_node )

/*
 * Confirm region propagation to child
 */
BOOST_AUTO_TEST_CASE( test_region_propagation )
{
	auto region = std::make_shared<parallel_region>("foo");
	tests::owning_node root(region);
	auto child = root.make_child_named<null>("child");

	BOOST_CHECK(child->region()->get_id() == region->get_id());
}

namespace
{
class test_owning_node : public owning_base_node
{
public:
	explicit test_owning_node(std::shared_ptr<parallel_region> r, forest_t* f ) :
			owning_base_node(r, "test_owning_node", f) {}
	tree_base_node::forest_t::iterator add_child()
	{
		make_child<test_owning_node>();
		return ++adobe::trailing_of(adobe::child_begin(self_).base());
	}

	explicit test_owning_node(std::shared_ptr<parallel_region> r, std::string name, forest_t* f ) :
			owning_base_node(r, name, f) {}

	size_t nr_of_children()
	{
		return this->forest_->size() -2; //-1 for this. -1 for root node
	}
};
}

/*
 * Confirm full names
 */
BOOST_AUTO_TEST_CASE( test_name_chaining )
{
	tests::owning_node root("root");
	auto child1 = root.make_child_named<test_owning_node>("test_owning_node");
	auto child2 = root.make_child_named<null>("2");
	auto child1a = child1->make_child_named<null>("a");

	BOOST_CHECK_EQUAL(full_name(*(root.forest()),child1->self_), "root/test_owning_node");
	BOOST_CHECK_EQUAL(full_name(*(root.forest()),child2->self_), "root/2");
	BOOST_CHECK_EQUAL(full_name(*(root.forest()),child1a->self_), "root/test_owning_node/a");
}

BOOST_AUTO_TEST_CASE( test_make_child )
{
	tests::owning_node root("root");
	auto child1 = root.make_child<node_class<int>>(5);
	auto child2 = root.make_child_named<node_class<int>>("name", 5);

	BOOST_CHECK_EQUAL(full_name(*(root.forest()), child1->self_), "root/test_node");
	BOOST_CHECK_EQUAL(full_name(*(root.forest()),child2->self_), "root/name");
}



BOOST_AUTO_TEST_CASE( test_deletion )
{
	tests::owning_node root;

	auto test_node = root.make_child<test_owning_node>();

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);

	auto temp_it = test_node->add_child();
	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 1);

	erase_with_subtree(*(root.forest()), temp_it);
	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);

	auto temp_it_2 = test_node->add_child();
	static_cast<test_owning_node*>(temp_it_2->get())->add_child();

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 2);
	erase_with_subtree(*(root.forest()), temp_it);

	BOOST_CHECK_EQUAL(test_node->nr_of_children(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
