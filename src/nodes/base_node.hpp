#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <threading/parallelregion.hpp>
#include <3rdparty/adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>
#include <stack>

namespace fc
{

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

	tree_base_node(const tree_base_node&) = delete;

	virtual ~tree_base_node()
	{
	}

	const region_info& region() const { return *region_; }
	tree_base_node* region(std::shared_ptr<region_info> r)
			{ region_ = r; return this; }


	std::string own_name() const { return name; }
	std::string full_name() const //todo, extract fully to non_member method
	{
		assert(forest_); //check invariant

		if (self_ == forest_->end())
			return own_name();

		//push names of parent / grandparent ... to stack to later reverse order.
		std::stack<std::string> name_stack;
		for (auto parent =  adobe::find_parent(self_);
				parent != forest_->end();
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

		full_name += own_name();
		return full_name;
	}

	tree_base_node(std::string name)
		: name(name)
		, forest_( std::make_shared<forest_t>() )
		, self_(forest_->end())
		, region_(std::make_shared<parallel_region>("invalid"))
	{
		assert(forest_); //check invariant
	}
protected:

	friend class root_node;
	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 */
	tree_base_node(	std::string n, std::shared_ptr<region_info> r )
		: name(n)
		, forest_( std::make_shared<forest_t>() )
		, self_(forest_->end())
		, region_(r)
	{
		assert(forest_); //check invariant
	}

	std::string name;
	std::shared_ptr<forest_t> forest_;
	public :forest_t::iterator self_;
	protected:
	std::shared_ptr<region_info> region_;

private:
	static constexpr auto name_seperator = "/";
};

inline std::string full_name(const tree_base_node& node)
{
	return node.full_name();
}

/**
 * \brief mixin to tree_base_node which creates an aggregate node.
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
struct node_owner : public base_t
{
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;

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
	node_t* make_child_named(std::string n, args_t ... args)
	{
		return add_child_named(n, std::make_unique<node_t>(args...));
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args
	 * and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child(args_t ... args)
	{
		return add_child(std::make_unique<node_t<args_t...>>(args...));
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args,
	 * sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child_named(std::string n, args_t ... args)
	{
		return add_child_named(n, std::make_unique<node_t<args_t...>>(args...));
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

		child->region_ = this->region_;
		child->forest_ = this->forest_;

		//we need to store an iterator and then cast back to node_t*
		//to avoid use after move on child.
		forest_t::iterator child_it = adobe::trailing_of(
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
		child->name = n;
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
		self_ = adobe::trailing_of(forest_->insert(
				forest_->begin(), std::make_unique<base_node>(n)));
	}

	virtual ~root_node() {}
};

/**
 * \brief Erases node and together with all children.
 *
 * \param forest, forest to delete node from.
 * \position iterator of forest pointing to node.
 * \pre position must be in forest.
 * \returns todo
 *
 * invalidates iterators pointing to deleted node.
 */
inline adobe::forest<std::unique_ptr<tree_base_node>>::iterator
erase_with_subtree(
		adobe::forest<std::unique_ptr<tree_base_node>>& forest,
		adobe::forest<std::unique_ptr<tree_base_node>>::iterator position)
{
	return forest.erase(
			adobe::child_begin(position).base(),
			adobe::child_end(position).base());
}

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
