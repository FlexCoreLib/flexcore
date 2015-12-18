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

/**
 * TODO
 */
class node_interface
{
public:
	typedef adobe::forest<node_interface*> forest_t;

	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 * TODO set region by region() member
	 */
	node_interface(	node_interface* p,
					std::string n = "" )
		: forest_(p ? p->forest_ : throw std::invalid_argument("Parent is 0") )
		, self(forest_->insert(boost::next(p->self), this))
		, own_name_(n)
		, region_()
	{
		assert(p);
		assert(p->self != forest_->end());
		region_ = p->region_;
		update_name();
	}

	virtual ~node_interface()
	{
		if (adobe::child_begin(self) != adobe::child_end(self))
			std::cout << "#### ORPHAN PANIC!!!" << std::endl;
		forest_->erase(self);
	}

	node_interface& name(const std::string& n)
	{
		own_name_ = n;
		update_name();
		return *this;
	}
	std::string own_name() const { return own_name_; }
	std::string full_name() const { return full_name_; }

	/*
	 * TODO expose access to parent, children and forest as needed
	 */
	const region_info& region() const { return *region_; }
	node_interface& region(std::shared_ptr<region_info> r) { region_ = r; return *this; }

	const forest_t& forest() { return *forest_; }

private:
	friend class root_node;
	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 * TODO set region by region() member
	 */
	node_interface(	std::string n, std::shared_ptr<region_info> r )
		: forest_( std::make_shared<forest_t>() )
		, self(forest_->insert(forest_->end(), this))
		, own_name_(n)
		, region_(r)
	{}

	void update_name()
	{
		auto parent = adobe::find_parent(self);
		if (parent != forest_->end())
			full_name_ = (*parent)->full_name() + "." + own_name_;
		else
			full_name_ = own_name_;

		for (auto child = adobe::child_begin(self); child != adobe::child_end(self); ++child)
			(*child)->update_name();
	}

	std::shared_ptr<forest_t> forest_;
	adobe::forest<node_interface*>::iterator self;
	std::string own_name_;
	std::string full_name_;
	std::shared_ptr<region_info> region_;
};

class root_node : public node_interface
{
public:
	root_node(	std::string n = "root",
				std::shared_ptr<region_info> r = std::make_shared<parallel_region>("root")	)
		: node_interface(n, r)
	{
	}
};

} // namespace fc

#endif /* SRC_NODES_NODE_INTERFACE_HPP_ */
