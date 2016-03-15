#ifndef SRC_NODES_BASE_NODE_HPP_
#define SRC_NODES_BASE_NODE_HPP_

#include <threading/parallelregion.hpp>
#include <graph/graph.hpp>
#include <3rdparty/adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>
#include <stack>

#include <ports/ports.hpp>

namespace fc
{

static constexpr auto name_seperator = "/";

/**
 * \brief base class for nodes contained in forest
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

	virtual ~tree_base_node() = default;

	const std::shared_ptr<parallel_region>& region() const { return region_; }

	std::string own_name() const { return graph_info_.name(); }

	graph::graph_node_properties graph_info() const { return graph_info_; }

	explicit tree_base_node(std::shared_ptr<parallel_region> r, std::string name)
		: graph_info_(name)
		, self_() // todo zonk
		, region_(r)
	{
		assert(region_);
	}
protected:

	/* Information for abstract graph */
	//stores the metainformation of the node used by the abstract graph
	graph::graph_node_properties graph_info_;


	/*
	 * iterator self_ allows access to the node inside the forest.
	 * Unfortunately this creates a circular reference of the node to itself.
	 */
public: forest_t::iterator self_;

	/* Information about which region the node belongs to */
private: std::shared_ptr<parallel_region> region_;
};

/**
 * \brief base class for nodes which own other nodes, aka nested nodes.
 *
 * \invariant forest_ != nullptr
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

	/// returns access to forest containing this node
	forest_t* forest() { return forest_; }

protected:
	// stores the access to the forest this node is contained in.
	forest_t* forest_;
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
template<class base_t>
struct node_owner : base_t
{
	using forest_t = typename base_t::forest_t;

	template<class... arg_types>
	node_owner(arg_types&&... args)
			: base_t(std::forward<arg_types>(args)...)
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
	typename std::enable_if<!std::is_base_of<owning_base_node, node_t>{}, node_t>::type*
	make_child(args_t ... args)
	{
		return add_child(std::make_unique<node_t>(args..., this->region()));
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
	make_child(args_t ... args)
	{
		return add_child(std::make_unique<node_t>( args..., this->region(), this->forest()));
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
	make_child_named(std::string name, args_t ... args)
	{
		return add_child(std::make_unique<node_t>(args..., this->region(),name));
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
	make_child_named(std::string name, args_t ... args)
	{
		return add_child(std::make_unique<node_t>( args..., this->region(), name, this->forest()));
	}

private:
	/**
	 * Takes ownership of child node and inserts into tree.
	 * \return pointer to child node
	 * \pre child != nullptr
	 */
	template<class node_t>
	node_t* add_child(std::unique_ptr<node_t> child)
	{
		assert(this->forest_); //check invariant
		assert(child);

		//we need to store an iterator and then cast back to node_t*
		//to avoid use after move on child.
		typename forest_t::iterator child_it = adobe::trailing_of(
				this->forest_->insert(this->self_, std::move(child)));
		(*child_it)->self_ = child_it;

		assert(adobe::find_parent(child_it) == this->self_);
		assert(adobe::find_parent(child_it) != this->forest_->end());

		return static_cast<node_t*>(child_it->get());
	}

};

namespace detail
{
/// Contains the forest itself, used by root_node
class forest_owner
{
public:
	typedef adobe::forest<std::unique_ptr<tree_base_node>> forest_t;
	std::shared_ptr<parallel_region>& region() { return region_; }
	forest_t* forest() { return forest_.get(); }
protected:
	std::unique_ptr<forest_t> forest_;
	std::shared_ptr<parallel_region> region_;
	forest_t::iterator self_;

	forest_owner(std::unique_ptr<forest_t> f,
			const std::shared_ptr<parallel_region>& r)
	: forest_{std::move(f)}
	, region_(r)
	{
		assert(forest_);
		assert(region_);
	}
};
} //namespace detail
/**
 * Root node for building node trees.
 * Has Ownership of the forest and thus serves
 * as the base node for all other nodes.
 */
class root_node : public  node_owner<detail::forest_owner>
{
public:
	root_node(	std::string n = "",
				std::shared_ptr<parallel_region> r =
						std::make_shared<parallel_region>("root")	)
		:  node_owner<detail::forest_owner>(std::make_unique<forest_owner::forest_t>(), r)
		, tree_root(nullptr)
	{
		self_ = adobe::trailing_of(this->forest_->insert(
				this->forest_->begin(), std::make_unique<tree_base_node>(r, n)));

		tree_root = static_cast<node_owner<owning_base_node>*>(self_->get());
		tree_root->self_ = self_;
	}

	//Todo this is currently necessary since ports in tests are attached directly to the root, remove if that is fixed.
	graph::graph_node_properties graph_info() const
	{
		return (*self_)->graph_info();
	}

	auto& nodes() { return *tree_root; }

private:
	node_owner<owning_base_node>* tree_root;

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

inline std::string full_name(const root_node& node)
{
	return node.graph_info().name();
}

} // namespace fc

#endif /* SRC_NODES_BASE_NODE_HPP_ */
