#ifndef TESTS_NODES_OWNING_NODE_HPP_
#define TESTS_NODES_OWNING_NODE_HPP_

#include <flexcore/extended/base_node.hpp>

namespace fc
{

namespace tests
{


/**
 * \brief Provides a simple node to attach ports to for testing.
 *
 * mockup of a forest and nodeowner to minimize boilerplate in unit-tests.
 * contains its own forest.
 */
class owning_node
{
public:
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;
	explicit owning_node(const std::shared_ptr<parallel_region>&  r
			= std::make_shared<parallel_region>("test_root_region"),
			std::string name = "owner")
		: owned_forest(std::make_unique<forest_t>())
		, owner(nullptr)
	{
		assert(r);
		assert(owned_forest);
		auto node = std::make_unique<owning_base_node>(r, name, owned_forest.get());
		owner = static_cast<owning_base_node*>(
		    owned_forest->insert(owned_forest->begin(), std::move(node))->get());
		assert(owner);
	}

	explicit owning_node(std::string name)
		: owning_node(std::make_shared<parallel_region>(name), name)
	{
	}

	template <class node_t, class ... args_t>
	node_t* make_child(args_t&& ... args)
	{
		return owner->make_child<node_t>(std::forward<args_t>(args)...);
	}

	template<class node_t, class ... args_t>
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child_named(std::string name, args_t&& ... args)
	{
		return owner->make_child_named<node_t>(name, std::forward<args_t>(args)...);
	}

	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child_named(std::string name, args_t&& ... args)
	{
		return owner->make_child_named<node_t>(name, std::forward<args_t>(args)...);
	}

	forest_t* forest() { return owned_forest.get(); }
	const forest_t* forest() const { return owned_forest.get(); }

	auto region() const { return owner->region(); }

	auto& node() { return *owner; };

private:
	std::unique_ptr<forest_t> owned_forest;
	owning_base_node* owner;

};

}  // namespace tests
}  // namespace fc

#endif /* TESTS_NODES_OWNING_NODE_HPP_ */
