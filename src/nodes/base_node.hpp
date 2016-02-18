#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <threading/parallelregion.hpp>
#include <graph/graph.hpp>
#include <3rdparty/adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>
#include <stack>

namespace fc
{

static constexpr auto name_seperator = "/";

/**
 * \brief base class for nodes.
 *
 * Nodes must be either of root_node type or are owned by a parent node.
 * It is possible to create nodes without setting a parent, but this is
 * strongly discouraged.
 *
 * TODO: - how to deal with parentless nodes that are not root?
 *         are they valid or invalid?
 *         if valid, what is their region?
 *       - Is it possible to disallow the creation of parentless nodes
 *
 * \invariant forest != null_ptr
 * \invariant self_ is always valid
 */
class tree_base_node
{
public:
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;

	virtual ~tree_base_node() = default;

	const std::shared_ptr<region_info>& region() const { return region_; }
	std::shared_ptr<region_info>& region() { return region_; }

	std::shared_ptr<forest_t>& forest() { return forest_; }
	const std::shared_ptr<forest_t>& forest() const { return forest_; }

	tree_base_node* region(std::shared_ptr<region_info> r)
			{ region_ = r; return this; }

	std::string own_name() const { return graph_info_.name(); }
	void set_name(std::string name) { graph_info_.name() = name; }

	graph::graph_node_properties graph_info() { return graph_info_; }

	explicit tree_base_node(std::string name)
		: graph_info_(name)
		, forest_( std::make_shared<forest_t>() )
		, self_(forest_->end())
		, region_(std::make_shared<parallel_region>("root"))
	{
		assert(forest_); //check invariant
	}
protected:
	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 */
	tree_base_node(	std::string name, std::shared_ptr<region_info> r )
		: graph_info_(name)
		, forest_( std::make_shared<forest_t>() )
		, self_(forest_->end())
		, region_(r)
	{
		assert(forest_); //check invariant
	}

	/* Information for abstract graph */
	//stores the metainformation of the node used by the abstract graph
	graph::graph_node_properties graph_info_;

	/* Access to forest */
	// stores the access to the forest this node is contained in.
	std::shared_ptr<forest_t> forest_;
	/*
	 * iterator self_ allows access to the node inside the forest.
	 * Unfortunately this creates a circular reference of the node to itself.
	 */
	public: forest_t::iterator self_;

	/* Information about which region the node belongs to */
	protected: std::shared_ptr<region_info> region_;
};

/**
 * \brief extension of tree_base_node which creates an aggregate node.
 *
 * Nodes of this type may have children nodes.
 * Adds several methods which allow adding new nodes as children.
 * Does not add additional state.
 *
 * Use make_child/make_child_named to create a node already inserted into
 * the ownership tree.
 *
 * Nodes are not copyable.
 * Deleting a node will delete any children.
 *
 * Node creation examples:
 * \code{cpp}
 * root_node root;
 * // simple creation
 * root.make_child<node>();
 * root.make_child<node_tmpl<int>>();
 * root.make_child_named<node_tmpl<int>>("name");
 *
 * // template with type deduction
 * root.make_child<node_tmpl>(5);
 * root.make_child_named<node_tmpl>("name", 5);
 * \endcode
 */
template<class base_t>
struct node_owner : base_t
{
	using forest_t = typename base_t::forest_t;

	template<class... arg_types>
	node_owner(const arg_types&... args)
			: base_t(args...)
	{
	}

	/**
	 * \brief creates child node of type node_t with constructor arguments args.
	 *
	 * Inserts new child into tree.
	 * \tparam node_t type of node to be created
	 * \param args constructor arguments passed to node_t
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	node_t* make_child(args_t ... args)
	{
		return add_child(std::make_unique<node_t>(args...));
	}
	/**
	 * Creates a new child node of type node_t from args,
	 * sets name of child to n and inserts child into tree.
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	node_t* make_child_named(std::string name, args_t ... args)
	{
		return add_child_named(name, std::make_unique<node_t>(args...));
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args
	 * and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child(args_t ... args)
	{
		return make_child<node_t<args_t...>>(args...);
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args,
	 * sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child_named(std::string name, args_t ... args)
	{
		return make_child_named<node_t<args_t...>>(name, args...);
	}

private:
	/**
	 * Takes ownership of child node and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child(std::unique_ptr<node_t> child)
	{
		assert(this->forest_); //check invariant

		child->region() = this->region_;
		child->forest() = this->forest_;

		//we need to store an iterator and then cast back to node_t*
		//to avoid use after move on child.
		typename forest_t::iterator child_it = adobe::trailing_of(
				this->forest_->insert(this->self_, std::move(child)));
		(*child_it)->self_ = child_it;

		assert(adobe::find_parent(child_it) == this->self_);
		assert(adobe::find_parent(child_it) != this->forest_->end());

		return static_cast<node_t*>(child_it->get());
	}
	/**
	 * Takes ownership of child node, sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child_named(const std::string& n, std::unique_ptr<node_t> child)
	{
		child->set_name(n);
		return add_child(std::move(child));
	}
};

typedef node_owner<tree_base_node> base_node;

/**
 * Root node for building node trees.
 * The only type of node that is allowed to exist without parent
 */
class root_node : public base_node
{
public:
	root_node(	std::string n = "",
				std::shared_ptr<region_info> r =
						std::make_shared<parallel_region>("root")	)
		: base_node(n, r)
	{
		self_ = adobe::trailing_of(this->forest_->insert(
				this->forest_->begin(), std::make_unique<base_node>(n)));
	}

};

/**
 * \brief Erases node and recursively erases all children.
 *
 * \param forest, forest to delete node from.
 * \position iterator of forest pointing to node.
 * \pre position must be in forest.
 * \returns todo
 *
 * invalidates iterators pointing to deleted node.
 */
inline tree_base_node::forest_t::iterator
erase_with_subtree(
		tree_base_node::forest_t& forest,
		tree_base_node::forest_t::iterator position)
{
	return forest.erase(
			adobe::child_begin(position).base(),
			adobe::child_end(position).base());
}

/**
 * \brief Returns the full name of a node.
 *
 * The full name consists of the chained name of the nodes parent, grandparent etc.
 * and the name of the node itself.
 * The names are separated by a separation token.
 */
inline std::string
full_name(
		const tree_base_node::forest_t& forest,
		tree_base_node::forest_t::const_iterator position)
{

	if (position == forest.end())
		return (*position)->own_name();

	//push names of parent / grandparent ... to stack to later reverse order.
	std::stack<std::string> name_stack;
	for (auto parent =  adobe::find_parent(position);
			parent != forest.end();
			parent =  adobe::find_parent(parent))
	{
		name_stack.emplace((*parent)->own_name());
	}

	std::string full_name;
	while (!name_stack.empty())
	{
		full_name = full_name + name_stack.top() + name_seperator;
		name_stack.pop();
	}

	full_name += (*position)->own_name();
	return full_name;
}

/// Returns the full name of a node.
inline std::string full_name(const tree_base_node& node)
{
	return full_name(*(node.forest()), node.self_);
}

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
