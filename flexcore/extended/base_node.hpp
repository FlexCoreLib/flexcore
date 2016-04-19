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
class graph_node : public node
{
public:
	graph_node(graph::connection_graph& graph, const std::string& name)
	    : graph_node(graph, {}, name)
	{
	}
	graph_node(graph::connection_graph& graph, std::shared_ptr<parallel_region> r,
	           const std::string& name)
	    : region_(std::move(r)), props_(name), graph_(&graph)
	{
		assert(graph_);
	}
	graph::graph_node_properties graph_info() const override
	{
		return props_;
	}
	graph::connection_graph& get_graph() override { return *graph_; }
	std::shared_ptr<parallel_region> region() override { return region_; }
private:
	std::shared_ptr<parallel_region> region_;
	graph::graph_node_properties props_;
	graph::connection_graph* graph_;
};

class tree_node : public node
{
public:
	virtual std::string name() const = 0;
};

using forest_t = adobe::forest<std::unique_ptr<tree_node>>;

struct forest_graph
{
	forest_graph(graph::connection_graph& graph) : graph(graph) {}
	forest_t forest;
	graph::connection_graph& graph;
};

/**
 * \brief base class for nodes contained in forest
 *
 *  Nodes are neither copy_constructyble nor copy_assignable.
 *
 *
 * \invariant region_ != null_ptr
 */
class tree_base_node : public tree_node
{
public:
	template<class data_t> using event_source = ::fc::event_source<data_t>;
	template<class data_t> using event_sink = ::fc::event_sink<data_t>;
	template<class data_t> using state_source = ::fc::state_source<data_t>;
	template<class data_t> using state_sink = ::fc::state_sink<data_t>;
	template<class port_t> using mixin = ::fc::default_mixin<port_t>;

	tree_base_node(forest_graph* fg, std::shared_ptr<parallel_region> r, std::string name);
	std::shared_ptr<parallel_region> region() override { return region_; }
	std::string name() const override;

	graph::graph_node_properties graph_info() const override;
	graph::connection_graph& get_graph() override;

protected:
	forest_graph* fg_;
private:
	/* Information about which region the node belongs to */
	std::shared_ptr<parallel_region> region_;
	/* Information for abstract graph */
	//stores the metainformation of the node used by the abstract graph
	graph::graph_node_properties graph_info_;

};

class owning_base_node;

class owner_holder : public tree_node
{
public:
	owning_base_node* set_owner(std::unique_ptr<owning_base_node> node)
	{
		owner_ = std::move(node);
		return owner_.get();
	}
	std::shared_ptr<parallel_region> region() override;
	graph::graph_node_properties graph_info() const override;
	graph::connection_graph& get_graph() override;
	std::string name() const override;

private:
	std::unique_ptr<owning_base_node> owner_;
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
	owning_base_node(forest_t::iterator self, forest_graph* fg, std::shared_ptr<parallel_region> r,
	                 std::string name)
	    : tree_base_node(fg, r, name), self_(self)
	{
	}
	owning_base_node(forest_t::iterator self, const tree_base_node& node)
		: tree_base_node(node), self_(self)
	{
	}

	template <class node_t, class... Args>
	node_t* make_owner(std::shared_ptr<parallel_region> r, std::string name, Args&&... args)
	{
		auto iter = adobe::trailing_of(fg_->forest.insert(self_, std::make_unique<owner_holder>()));
		auto& holder = static_cast<owner_holder&>(*iter->get());
		return static_cast<node_t*>(holder.set_owner(std::make_unique<node_t>(
		    std::forward<Args>(args)..., iter, tree_base_node{fg_, r, name})));
	}

	tree_base_node* new_node(std::string name)
	{
		return static_cast<tree_base_node*>(
		    add_child(std::make_unique<tree_base_node>(fg_, region(), name)));
	}
	tree_base_node* new_node(std::shared_ptr<parallel_region> r, std::string name)
	{
		return static_cast<tree_base_node*>(
		    add_child(std::make_unique<tree_base_node>(fg_, r, name)));
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
	node_t* make_child(args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				tree_base_node{fg_, region(), node_t::default_name})));
	}

	/**
	 * \brief  Overload of make_child with region.
	 *
	 * @param r Child is attached to this region
	 * @param args onstructor arguments passed to node_t
	 * @return pointer to child node
	 */
	template<class node_t, class ... args_t>
	node_t* make_child(std::shared_ptr<parallel_region> r, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				tree_base_node{fg_, r, node_t::default_name})));
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
	node_t* make_child_named(std::string name, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				tree_base_node{fg_, region(), name})));
	}

	template<class node_t, class ... args_t>
	node_t* make_child_named(std::shared_ptr<parallel_region> r, std::string name, args_t&&... args)
	{
		return static_cast<node_t*>(add_child(std::make_unique<node_t>(
				std::forward<args_t>(args)...,
				tree_base_node{fg_, std::move(r), std::move(name)})));
	}

	forest_t::iterator self() const;
private:
	forest_t::iterator self_;
	/**
	 * Takes ownership of child node and inserts into tree.
	 * \return pointer to child node
	 * \pre child != nullptr
	 */
	tree_node* add_child(std::unique_ptr<tree_node> child);

};

/**
 * \brief Root node for building node trees.
 *
 * Has ownership of the forest and thus serves
 * as the root node for all other nodes in the forest.
 */
class forest_owner
{
public:
	forest_owner(graph::connection_graph& graph, std::string n, std::shared_ptr<parallel_region> r);
	owning_base_node& nodes() { return *tree_root; }
	void print_forest(std::ostream& out) const;

private:
	std::unique_ptr<forest_graph> fg_;
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
std::string full_name(forest_t& forest, const tree_node* node);

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
