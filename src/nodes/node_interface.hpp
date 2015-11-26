#ifndef SRC_NODES_NODE_INTERFACE_HPP_
#define SRC_NODES_NODE_INTERFACE_HPP_

#include <threading/parallelregion.hpp>

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
	node_interface(	node_interface* p = 0,
					std::shared_ptr<region_info> r = std::shared_ptr<region_info>() )
		: parent()
		, region_(r)
	{
		set_parent(p);

		if (not region_)
			region_ = parent->region_;
	}
	virtual ~node_interface()
	{
		for (auto child : children)
			child->set_parent(0);
		set_parent(0);
		assert(children.empty());
	}

	virtual void set_name(const std::string& n)
	{
		own_name_ = n;
		update_name();
	}
	std::string own_name() const { return own_name_; }
	std::string full_name() const { return full_name_; }

	// call with 0 to unset parent
	virtual void set_parent(node_interface* p)
	{
		if (parent == p)
			return;
		if (parent)
			parent->unregister_child(this);
		parent = p;
		if (parent)
			parent->register_child(this);
		update_name();
	}
	const region_info& region() const { return *region_; }

private:
	void register_child(node_interface* child)
	{
		assert(child);
		auto res = children.insert(child);
		if (not res.second)
			throw std::runtime_error("child already registered.");
	}
	void unregister_child(node_interface* child)
	{
		assert(child);
		size_t num_erased = children.erase(child);
		if (num_erased != 1)
			throw std::runtime_error("child was not registered in the first place.");
	}
	void update_name()
	{
		if (parent)
			full_name_ = parent->full_name() + "." + own_name_;
		else
			full_name_ = own_name_;
		for (auto child : children)
			child->update_name();
	}

	node_interface* parent;
	std::set<node_interface*> children;
	std::string own_name_;
	std::string full_name_;
	std::shared_ptr<region_info> region_;
};

} // namespace fc

#endif /* SRC_NODES_NODE_INTERFACE_HPP_ */
