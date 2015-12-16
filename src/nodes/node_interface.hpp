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
	/**
	 * Constructor taking a forest that must be empty
	 * This node is set as root node
	 *
	 * @param f empty forest
	 * @param r region_info object. Will be constructed from name if not given
	 */
	typedef adobe::forest<node_interface*> forest_t;
	node_interface(	std::shared_ptr<forest_t> f,
					std::string n = "",
					std::shared_ptr<region_info> r = std::shared_ptr<region_info>() )
		: forest(f->empty() ? f : throw std::invalid_argument("Forest must be emtpy"))
		, parent(f->end())
		, self(forest->insert(parent, this))
		, own_name_(n)
		, region_(r)
	{
		// TODO enforce region uniqueness?
		if (not region_)
			region_ = std::make_shared<parallel_region>(n);

		update_name();
	}

	/**
	 * Constructor taking a parent node
	 *
	 * @param p parent node (not 0)
	 * @param r region_info object. Will be taken from parent if not given
	 */
	node_interface(	node_interface* p,
					std::string n = "",
					std::shared_ptr<region_info> r = std::shared_ptr<region_info>() )
		: forest(p ? p->forest : throw std::invalid_argument("Parent may not be 0"))
		, parent(p->self)
		, self(forest->insert(boost::next(parent), this))
		, own_name_(n)
		, region_(r)
	{
		if (not region_)
			region_ = p->region_;

		update_name();
	}

	virtual ~node_interface()
	{
		forest->erase(self);
	}

	node_interface& set_name(const std::string& n)
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

private:
	void update_name()
	{
		if (parent != forest->end())
			full_name_ = (*parent)->full_name() + "." + own_name_;
		else
			full_name_ = own_name_;

		for (auto child = adobe::child_begin(self); child != adobe::child_end(self); ++child)
			(*child)->update_name();
	}

	std::shared_ptr<forest_t> forest;
	adobe::forest<node_interface*>::iterator parent;
	adobe::forest<node_interface*>::iterator self;
	std::string own_name_;
	std::string full_name_;
	std::shared_ptr<region_info> region_;
};

} // namespace fc

#endif /* SRC_NODES_NODE_INTERFACE_HPP_ */
