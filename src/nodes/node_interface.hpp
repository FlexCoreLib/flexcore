#ifndef SRC_NODES_NODE_INTERFACE_HPP_
#define SRC_NODES_NODE_INTERFACE_HPP_

#include <threading/parallelregion.hpp>

#include <3rdparty/adobe/forest.hpp>

#include <cassert>
#include <string>
#include <memory>
#include <set>

namespace fc
{

class named
{
public:
	explicit named(std::string n = "")
		: own_name_(n)
	{}

	virtual std::string own_name() const { return own_name_; }
	virtual std::string full_name() const { return own_name(); }

	virtual named* name(const std::string& n)
	{
		own_name_ = n;
		return this;
	}

private:
	std::string own_name_;
};

/**
 * TODO
 */
class node_interface : public named
{
public:
	typedef adobe::forest<node_interface*> forest_t;

	template<class node_t>
	node_t* add_child(node_t* child)
	{
		child->region_ = this->region_;
		child->forest_ = this->forest_;
		child->self_ = forest_->insert(boost::next(self_), child);
		return child;
	}
	template<class node_t>
	node_t* add_child(std::string n, node_t* child)
	{
		child->name(n);
		add_child(child);
		return child;
	}

	// TODO TEST!!!
//	template<class node_t, class ... args_t>
//	node_t make_child(args_t ... args)
//	{
//		auto child = new node_t(args...);
//		add_child(child);
//		return child;
//	}
//	template<class node_t, class ... args_t>
//	node_t make_child_n(std::string n, args_t ... args)
//	{
//		auto child = new node_t(args...);
//		add_child(n, child);
//		return child;
//	}

	/**
	 * Constructor taking a parent node
	 */
	node_interface(std::string n)
		: named(n)
		, forest_()
		, self_()
		, region_()
	{}

	node_interface(const node_interface&) = delete;

	virtual ~node_interface()
	{
		for (auto child = adobe::child_begin(self_); child != adobe::child_end(self_); ++child)
			delete *child;
		forest_->erase(self_);
	}

	std::string full_name() const
	{
		auto parent = adobe::find_parent(self_);
		if (parent != forest_->end())
			return (*parent)->full_name() + "/" + own_name();
		else
			return this->own_name();
	}

	const region_info& region() const { return *region_; }

	node_interface* region(std::shared_ptr<region_info> r) { region_ = r; return this; }

//	const forest_t& forest() { return *forest_; }

private:
	friend class root_node;
	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 */
	node_interface(	std::string n, std::shared_ptr<region_info> r )
		: named(n)
		, forest_( std::make_shared<forest_t>() )
		, self_(forest_->insert(forest_->end(), this))
		, region_(r)
	{}

	std::shared_ptr<forest_t> forest_;
	adobe::forest<node_interface*>::iterator self_;
	std::shared_ptr<region_info> region_;
};

class root_node : public node_interface
{
public:
	root_node(	std::string n = "",
				std::shared_ptr<region_info> r = std::make_shared<parallel_region>("root")	)
		: node_interface(n, r)
	{}
};

struct null : public node_interface
{
	null(std::string n = "null") : node_interface(n) {}
};

} // namespace fc

#endif /* SRC_NODES_NODE_INTERFACE_HPP_ */
