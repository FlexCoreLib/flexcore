#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <core/named.hpp>
#include <threading/parallelregion.hpp>
#include <3rdparty/adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>
#include <set>

namespace fc
{

/**
 * \brief base class for nodes.
 *
 * Nodes must be either of root_node type or are owned by a parent node.
 * It is possible to create nodes without setting a parent, but this is
 * strongly discouraged.
 *
 * Use make_child/make_child_named to create a node already inserted into
 * the ownership tree.
 *
 * Nodes are not copyable.
 * Deleting a node will delete any children.
 *
 * Node creation examples:
 * <code>
 * root_node root;
 * // simple creation
 * root.make_child<node>();
 * root.make_child<node_tmpl<int>>();
 * root.make_child_named<node_tmpl<int>>("name");
 *
 * // template with type deduction
 * root.make_child<node_tmpl>(5);
 * root.make_child_named<node_tmpl>("name", 5);
 * </code>
 *
 * TODO: - how to deal with parentless nodes that are not root?
 *         are they valid or invalid?
 *         if valid, what is their region?
 *       - Is it possible to disallow the creation of parentless nodes
 */
class base_node : public named
{
public:
	template<class node_t, class ... args_t>
	node_t* make_child(args_t ... args)
	{
		return add_child(std::make_unique<node_t>(args...));
	}
	/**
	 * Creates a new child node of type node_t from args,
	 * sets name to n and inserts into tree.
	 * @return pointer to child node
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

	base_node(const base_node&) = delete;

	virtual ~base_node()
	{
		if (forest_)
		{
			while (adobe::child_begin(self_) != adobe::child_end(self_))
				delete *adobe::child_begin(self_);
			forest_->erase(self_);
		}
	}

	const region_info& region() const { return *region_; }
	base_node* region(std::shared_ptr<region_info> r) { region_ = r; return this; }

protected:
	base_node(std::string name)
		: named(name)
		, forest_()
		, self_()
		, region_(std::make_shared<parallel_region>("invalid"))
	{}

	/**
	 * Access to parent. Returns 0 if parent does not exist.
	 */
	base_node* parent() const
	{
		if (not forest_)
			return 0;
		auto parent = adobe::find_parent(self_);
		if (parent != forest_->end())
			return *parent;
		return 0;
	}

private:
	// -- construction --
	/**
	 * Takes ownership of child node and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child(std::unique_ptr<node_t> child)
	{
		child->region_ = this->region_;
		child->forest_ = this->forest_;
		child->self_ = adobe::trailing_of(forest_->insert(self_, child.get()));
		return child.release();
	}
	/**
	 * Takes ownership of child node, sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child_named(std::string n, std::unique_ptr<node_t> child)
	{
		child->name(n);
		return add_child(std::move(child));
	}
	/**
	 * Creates a new child node of type node_t from args and inserts into tree.
	 * @return pointer to child node
	 */

	// -- constructors --
	/**
	 * Constructor taking no parent - used for
	 * creating a root_node
	 */
	base_node( std::string n, std::shared_ptr<region_info> r )
		: named(n)
		, forest_( std::make_shared<forest_t>() )
		, self_(adobe::trailing_of(forest_->insert(forest_->begin(), this)))
		, region_(r)
	{}

	// -- typedefs --
	typedef adobe::forest<base_node*> forest_t;

	// -- friends --
	friend class root_node;

	// -- members --
	std::shared_ptr<forest_t> forest_;
	adobe::forest<base_node*>::iterator self_;
	std::shared_ptr<region_info> region_;
};

/**
 * Root node for building node trees.
 * The only type of node that is allowed to exist without parent
 */
class root_node : public base_node
{
public:
	root_node(	std::string n = "",
				std::shared_ptr<region_info> r = std::make_shared<parallel_region>("root")	)
		: base_node(n, r)
	{}

	virtual ~root_node() {}
};

/**
 * Node without properties or purpose.
 * Maily for testing purposes
 */
struct null : public base_node
{
	null() : base_node("null") {}
};

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
