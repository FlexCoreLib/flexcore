#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <flexcore/extended/node_fwd.hpp>
#include <flexcore/ports.hpp>
#include <adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>


namespace fc
{

/// any class implementing node interface can be stored in forest
using tree_node = node;
/// the ownership tree of all nodes
using forest_t = adobe::forest<std::unique_ptr<tree_node>>;

struct forest_graph
{
	forest_graph(graph::connection_graph& graph) : graph(graph) {}
	forest_t forest;
	graph::connection_graph& graph;
};

class tree_base_node;
class owning_base_node;
class forest_owner;

static constexpr auto name_seperator = ".";

class node_args
{
	node_args(forest_graph& fg, const std::shared_ptr<parallel_region>& r, const std::string& name,
	          forest_t::iterator self = forest_t::iterator{})
	    : fg(fg), r(r), graph_info(name, r.get()), self(self)
	{
	}
	forest_graph& fg;
	std::shared_ptr<parallel_region> r;
	graph::graph_node_properties graph_info;
	forest_t::iterator self;

	friend class fc::tree_base_node;
	friend class fc::owning_base_node;
	friend class fc::forest_owner;
};


/** \brief Base class for nodes contained in forest.
 *
 * These should only be constructed through an owning_base_node's
 * make_child()/make_child_named()/new_node() methods.
 *
 * \invariant region_ != nullptr
 * \ingroup nodes
 */
class tree_base_node : public tree_node
{
public:
	template<class data_t> using event_source = ::fc::event_source<data_t>;
	template<class data_t> using event_sink = ::fc::event_sink<data_t>;
	template<class data_t> using state_source = ::fc::state_source<data_t>;
	template<class data_t> using state_sink = ::fc::state_sink<data_t>;
	template<class port_t> using mixin = ::fc::default_mixin<port_t>;

	explicit tree_base_node(const node_args& args);

	tree_base_node(const tree_base_node&) = delete;
	tree_base_node& operator=(const tree_base_node&) = delete;

	std::shared_ptr<parallel_region> region() override { assert(region_); return region_; }
	std::string name() const override;

	graph::graph_node_properties graph_info() const override;
	graph::connection_graph& get_graph() final override;

protected:
	forest_graph& fg_;
private:
	/// Information about which region the node belongs to
	std::shared_ptr<parallel_region> region_;
	/// Stores the metainformation of the node used by the abstract graph
	graph::graph_node_properties graph_info_;

};

/**
 * \brief Base class for nodes which own other nodes, aka compound nodes.
 *
 * \invariant self_ points to self in forest.
 *
 * Nodes of this type may have children nodes.
 * Use make_child/make_child_named to create a node already inserted into
 * the ownership tree.
 * Use new_node to create a tree_base_node that carries metadata (about its
 * position in tree) but which can be passed onto a node for use with ports.
 *
 * \ingroup nodes
 *
 * Node creation examples:
 * \code{cpp}
 * owning_base_node root;
 * // simple creation
 * root.make_child<node>();
 * root.make_child<node_tmpl<int>>();
 * root.make_child_named<node_tmpl<int>>("name");
 * \endcode
 */
class owning_base_node : public tree_base_node
{
public:
	explicit owning_base_node(const node_args& node)
		: tree_base_node(node), self_(node.self)
	{
	}

	/**
	 * \brief creates a new element in the forest which serves as a proxy for a node
	 *
	 * Use this if you want to manually control ownership of a node.
	 * \param name name of the node.
	 * \return node_args corresponding to the proxy node.
	 */
	node_args new_node(std::string name)
	{
		return new_node({fg_, region(), std::move(name)});
	}
	/**
	 * \brief creates a new element in the forest which serves as a proxy for a node
	 *
	 * Use this if you want to manually control ownership of a node.
	 * \param name name of the node.
	 * \param r region to be used by new node.
	 * \return node_args corresponding to the proxy node.
	 */
	node_args new_node(std::shared_ptr<parallel_region> r, std::string name)
	{
		return new_node({fg_, std::move(r), std::move(name)});
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
	node_t& make_child(args_t&&... args)
	{
		static_assert(std::is_base_of<tree_base_node, node_t>(),
				"make_child can only be used with classes inheriting from fc::tree_base_node");
		return make_child_impl<node_t>(node_args{fg_, region(), node_t::default_name},
		                               std::forward<args_t>(args)...);
	}

	/**
	 * \brief  Overload of make_child with region.
	 *
	 * @param r Child is attached to this region
	 * @param args onstructor arguments passed to node_t
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	node_t& make_child(std::shared_ptr<parallel_region> r, args_t&&... args)
	{
		static_assert(std::is_base_of<tree_base_node, node_t>(),
				"make_child can only be used with classes inheriting from fc::tree_base_node");
		return make_child_impl<node_t>(node_args{fg_, std::move(r), node_t::default_name},
		                               std::forward<args_t>(args)...);
	}

	/**
	 * \brief Creates a new child node of type node_t from args
	 *
	 * Sets name of child to n and inserts child into tree.
	 * Sets forest of of child to forest of parent.
	 * \return pointer to child node
	 * \post nr of children > 0
	 */
	template<class node_t, class ... args_t>
	node_t& make_child_named(std::string name, args_t&&... args)
	{
		static_assert(std::is_base_of<tree_base_node, node_t>(),
				"make_child can only be used with classes inheriting from fc::tree_base_node");
		return make_child_impl<node_t>(node_args{fg_, region(), std::move(name)},
		                               std::forward<args_t>(args)...);
	}

	template<class node_t, class ... args_t>
	node_t& make_child_named(std::shared_ptr<parallel_region> r, std::string name, args_t&&... args)
	{
		static_assert(std::is_base_of<tree_base_node, node_t>(),
				"make_child can only be used with classes inheriting from fc::tree_base_node");
		return make_child_impl<node_t>(node_args{fg_,  std::move(r),  std::move(name)},
		                               std::forward<args_t>(args)...);
	}


protected:
	forest_t::iterator self() const;
private:
	template <class node_t, class... Args>
	node_t& make_child_impl(node_args nargs, Args&&... args)
	{
		//first create a proxy node to get the node_args with a correct iterator
		node_args n = new_node(std::move(nargs));
		//then replace proxy with proper node
		*n.self = std::make_unique<node_t>(std::forward<Args>(args)..., n);
		return dynamic_cast<node_t&>(**n.self);
	}

	forest_t::iterator self_;
	/**
	 * Takes ownership of child node and inserts into tree.
	 * \return iterator to child node
	 * \pre child != nullptr
	 */
	forest_t::iterator add_child(std::unique_ptr<tree_node> child);

	/// Helper: create a new tree_base_node in tree from node_args.
	node_args new_node(node_args args);
};

class visualization;

/**
 * \brief Root node for building node trees.
 *
 * Has ownership of the forest and thus serves
 * as the root node for all other nodes in the forest.
 *
 *\invariant tree_root != nullptr
 *\invariant viz_ != nullptr
 *\invariant fg_ != nullptr
 */
class forest_owner
{
public:
	/**
	 * \brief constructs root node with access to graph infrastructure
	 * \param graph access to the abstract connectopn graph
	 * \param n Human readable name of the root node
	 * \param r parallel_region the root node belongs to
	 * \pre r != nullptr
	 */
	forest_owner(graph::connection_graph& graph, std::string n, std::shared_ptr<parallel_region> r);
	~forest_owner();
	owning_base_node& nodes() { assert(tree_root); return *tree_root; }
	void visualize(std::ostream& out) const;

private:
	std::unique_ptr<forest_graph> fg_;
	/// non_owning access to first node in tree, ownership is in forest.
	owning_base_node* tree_root;
	std::unique_ptr<visualization> viz_; // ptr to avoid cyclic reference
};

/**
 * \brief Erases node and recursively erases all children.
 *
 * \param forest forest container to delete node from.
 * \param position iterator of forest pointing to node.
 * \pre position must be in forest.
 * \returns trailing iterator pointing to parent of position.
 *
 * invalidates iterators pointing to deleted node.
 */
inline forest_t::iterator
erase_with_subtree(
		forest_t& forest,
		forest_t::iterator position)
{
	return forest.erase(
			adobe::leading_of(position),
			++adobe::trailing_of(position));
}

/**
 * \brief Returns the full name of a node.
 *
 * The full name consists of the chained name of the nodes parent, grandparent etc.
 * and the name of the node itself.
 * The names are separated by a separation token.
 */
std::string full_name(forest_t& forest, const tree_node& node);

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
