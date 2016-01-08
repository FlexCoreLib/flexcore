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
 * Use add_child and make_child to insert a new node into the ownership tree.
 *
 * Nodes are not copyable.
 * Deleting a node will delete any children.
 *
 * Node creation examples:
 * <code>
 * root_node root;
 * // factory function
 * root.add_child(new node_making_fun(a, b));
 * root.add_child("name", new node_making_fun(a, b));
 *
 * // simple creation
 * root.make_child<node>();
 * root.make_child<node_tmpl<int>();
 * root.make_child_n<"name", node_tmpl<int>();
 *
 * // template with type deduction
 * root.make_child<node_tmpl>(5);
 * root.make_child_n<node_tmpl>("name", 5);
 * </code>
 *
 * base_node is not copyable.
 *
 * TODO: how to deal with parentless nodes that are not root?
 *       are they valid or invalid?
 *       if valid, what is their region?
 */
class base_node : public named
{
public:
	/**
	 * Takes ownership of child node and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child(node_t* child)
	{
		child->region_ = this->region_;
		child->forest_ = this->forest_;
		child->self_ = adobe::trailing_of(forest_->insert(self_, child));
		return child;
	}
	/**
	 * Takes ownership of child node, sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t>
	node_t* add_child(std::string n, node_t* child)
	{
		child->name(n);
		add_child(child);
		return child;
	}
	/**
	 * Creates a new child node of type node_t from args and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	node_t* make_child(args_t ... args)
	{
		auto child = new node_t(args...);
		add_child(child);
		return child;
	}
	/**
	 * Creates a new child node of type node_t from args,
	 * sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	node_t* make_child_n(std::string n, args_t ... args)
	{
		auto child = new node_t(args...);
		add_child(n, child);
		return child;
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args
	 * and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child(args_t ... args)
	{
		auto child = new node_t<args_t ...>(args...);
		add_child(child);
		return child;
	}
	/**
	 * Creates a new child node of type node_t<args_t> from args,
	 * sets name to n and inserts into tree.
	 * @return pointer to child node
	 */
	template<template <typename ...> class node_t, class ... args_t>
	node_t<args_t ...>* make_child_n(std::string n, args_t ... args)
	{
		auto child = new node_t<args_t ...>(args...);
		add_child(n, child);
		return child;
	}

	base_node(std::string name)
		: named(name)
		, forest_()
		, self_()
		, region_(std::make_shared<parallel_region>("invalid"))
	{}

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
	typedef adobe::forest<base_node*> forest_t;

	friend class root_node;
	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 */
	base_node(	std::string n, std::shared_ptr<region_info> r )
		: named(n)
		, forest_( std::make_shared<forest_t>() )
		, self_(adobe::trailing_of(forest_->insert(forest_->begin(), this)))
		, region_(r)
	{}

	std::shared_ptr<forest_t> forest_;
	adobe::forest<base_node*>::iterator self_;
	std::shared_ptr<region_info> region_;
};

class root_node : public base_node
{
public:
	root_node(	std::string n = "",
				std::shared_ptr<region_info> r = std::make_shared<parallel_region>("root")	)
		: base_node(n, r)
	{}

	virtual ~root_node() {}
};

struct null : public base_node
{
	null() : base_node("null") {}
};

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
