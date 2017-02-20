#ifndef TESTS_NODES_OWNING_NODE_HPP_
#define TESTS_NODES_OWNING_NODE_HPP_

#include <flexcore/extended/base_node.hpp>
#include <flexcore/scheduler/cyclecontrol.hpp>

namespace fc
{

namespace tests
{
class test_helper_node : public owning_base_node
{
public:
	explicit test_helper_node(const node_args& node) : owning_base_node(node) {}
	forest_t* get_forest() { return &this->fg_.forest; }
	forest_t::iterator self() { return owning_base_node::self(); }
};

/**
 * \brief Provides a simple node to attach ports to for testing.
 *
 * mockup of a forest and nodeowner to minimize boilerplate in unit-tests.
 * contains its own forest.
 */
class owning_node
{
public:
	explicit owning_node(const std::shared_ptr<parallel_region>& r
			= std::make_shared<parallel_region>("test_root_region", thread::cycle_control::slow_tick),
	                     std::string name = "owner")
	    : owner_(graph, std::move(name), r)
	    , forest_(nullptr)
	    , owner(&owner_.nodes())
	{
		auto& n = owner->make_child_named<test_helper_node>("");
		forest_ = n.get_forest();
		forest_->erase(n.self());
		assert(r);
		assert(owner);
	}

	explicit owning_node(const std::string& name, virtual_clock::steady::duration tick_rate = thread::cycle_control::slow_tick)
		: owning_node(std::make_shared<parallel_region>(name, tick_rate), name)
	{
	}

	node_args new_node(std::string name)
	{
		return owner->new_node(name);
	}

	node_args new_node(std::shared_ptr<parallel_region> r, std::string name)
	{
		return owner->new_node(r, name);
	}

	template <class node_t, class ... args_t>
	node_t& make_child(args_t&& ... args)
	{
		return owner->make_child<node_t>(std::forward<args_t>(args)...);
	}

	template<class node_t, class ... args_t>
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type&
	make_child_named(std::string name, args_t&& ... args)
	{
		return owner->make_child_named<node_t>(name, std::forward<args_t>(args)...);
	}

	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type&
	make_child_named(std::string name, args_t&& ... args)
	{
		return owner->make_child_named<node_t>(name, std::forward<args_t>(args)...);
	}

	forest_t* forest() { return forest_; }
	const forest_t* forest() const { return forest_; }

	auto region() const { return owner->region(); }
	auto& node() { return *owner; }

private:
	graph::connection_graph graph;
	forest_owner owner_;
	forest_t* forest_;
	owning_base_node* owner;

};

}  // namespace tests
}  // namespace fc

#endif /* TESTS_NODES_OWNING_NODE_HPP_ */
