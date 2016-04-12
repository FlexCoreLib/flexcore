#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <flexcore/extended/graph/graph.hpp>

#include <flexcore/scheduler/parallelregion.hpp>
#include <adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>

#include <flexcore/ports.hpp>

namespace fc
{

/**
 * \brief base class for nodes contained in forest
 *
 *  Nodes are neither copy_constructyble nor copy_assignable.
 *
 *
 * \invariant region_ != null_ptr
 * \invariant self_ is always valid
 */
class tree_base_node
{
public:
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;

	template<class data_t> using event_source = ::fc::event_source<data_t>;
	template<class data_t> using event_sink = ::fc::event_sink<data_t>;
	template<class data_t> using state_source = ::fc::state_source<data_t>;
	template<class data_t> using state_sink = ::fc::state_sink<data_t>;

	tree_base_node(std::shared_ptr<parallel_region> r, std::string name);
	virtual ~tree_base_node() = default;

	const std::shared_ptr<parallel_region>& region() const { return region_; }

	std::string name() const { return graph_info_.name(); }

	graph::graph_node_properties graph_info() const { return graph_info_; }

private:
	/* Information about which region the node belongs to */
	std::shared_ptr<parallel_region> region_;
	/* Information for abstract graph */
	//stores the metainformation of the node used by the abstract graph
	graph::graph_node_properties graph_info_;

};

/**
 * \brief base class for nodes which own other nodes, aka nested nodes.
 *
 * \invariant forest_ != nullptr
 *
 * Nodes of this type may have children nodes.
 *
 * Use make_child/make_child_named to create a node already inserted into
 * the ownership tree.
 *
 * Node creation examples:
 * \code{cpp}
 * root_node root;
 * // simple creation
 * root.make_child<node>();
 * root.make_child<node_tmpl<int>>();
 * root.make_child_named<node_tmpl<int>>("name");
 * \endcode
 */
class owning_base_node : public tree_base_node
{
public:
	owning_base_node(std::shared_ptr<parallel_region> r, std::string name, forest_t* f)
		: tree_base_node(r, name)
		, forest_(f)
	{
		assert(forest_); //check invariant
	}

	/**
	 * \brief creates child node of type node_t with constructor arguments args.
	 *
	 * Inserts new child into tree.
	 * Child node is attached to region of parent node.
	 *
	 * \tparam node_t type of node to be created
	 * \param args constructor arguments passed to node_t
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child(args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				region())));
	}

	/**
	 * \brief  Overload of make_child with region.
	 *
	 * @param r Child is attached to this region
	 * @param args onstructor arguments passed to node_t
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child(std::shared_ptr<parallel_region> r, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				r)));
	}
	/**
	 * \brief creates child node of type node_t with constructor arguments args.
	 *
	 * Inserts new child into tree.
	 * Sets forest of child to forest of parent.
	 * \tparam node_t type of node to be created, needs to inherit from owning_base_node.
	 * \param args constructor arguments passed to node_t
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child(args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				region(),
				forest_)));
	}

	/**
	 * \brief  Overload of make_child with region.
	 *
	 * @param r Child is attached to this region
	 * @param args onstructor arguments passed to node_t
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child(std::shared_ptr<parallel_region> r, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				r,
				forest_)));
	}
	/**
	 * \brief Creates a new child node of type node_t from args.
	 *
	 * sets name of child to n and inserts child into tree.
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child_named(std::string name, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				region(),
				name)));
	}
	/**
	 * \brief Creates a new child node of type node_t from args
	 * if node_t inherits from owning_base_node
	 *
	 * Sets name of child to n and inserts child into tree.
	 * Sets forest of of child to forest of parent.
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child_named(std::string name, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				region(),
				name,
				forest_)));
	}

	template<class node_t, class ... args_t>
	typename std::enable_if<std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child_named(std::shared_ptr<parallel_region> r, std::string name, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				std::move(r),
				std::move(name),
				forest_)));
	}

protected:
	forest_t::iterator self() const;
	// stores the access to the forest this node is contained in.
	forest_t* forest_;

private:
	/**
	 * Takes ownership of child node and inserts into tree.
	 * \return pointer to child node
	 * \pre child != nullptr
	 */
	tree_base_node* add_child(std::unique_ptr<tree_base_node> child);

};

/**
 * \brief Root node for building node trees.
 *
 * Has ownership of the forest and thus serves
 * as the root node for all other nodes in the forest.
 */
class root_node
{
public:
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;

	root_node(std::string n, std::shared_ptr<parallel_region> r);

	owning_base_node& nodes() { return *tree_root; }

private:
	std::unique_ptr<forest_t> forest_;
	/// non_owning access to first node in tree, ownership is in forest.
	owning_base_node* tree_root;
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
std::string full_name(tree_base_node::forest_t& forest,
                      const tree_base_node* node);

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
