#include <flexcore/extended/base_node.hpp>

#include <stack>

namespace fc
{

static constexpr auto name_seperator = "/";

std::string full_name(
        const tree_base_node::forest_t& forest,
        tree_base_node::forest_t::const_iterator position)
{
	assert(position != forest.end());
	//push names of parent / grandparent ... to stack to later reverse order.
	std::stack<std::string> name_stack;
	for (auto parent = adobe::find_parent(position); parent != forest.end();
	        parent = adobe::find_parent(parent))
	{
		name_stack.emplace((*parent)->name());
	}
	std::string full_name;
	while (!name_stack.empty())
	{
		full_name += (name_stack.top() + name_seperator);
		name_stack.pop();
	}
	full_name += (*position)->name();
	return full_name;
}

tree_base_node::tree_base_node(
		std::shared_ptr<parallel_region> r,
		std::string name)
	: self_() // todo this currently allows construction of node with invalid iterator self_
	, region_(r)
	, graph_info_(name)
{
assert(region_);
}

fc::tree_base_node* owning_base_node::add_child(
		std::unique_ptr<tree_base_node> child)
{
	assert(forest_);
	assert(child);
	//we need to store an iterator and then cast back to node_t*
	//to avoid use after move on child.
	typename forest_t::iterator child_it = adobe::trailing_of(
	        forest_->insert(self_, std::move(child)));
	(*child_it)->self_ = child_it;
	assert(adobe::find_parent(child_it) == self_);
	assert(adobe::find_parent(child_it) != forest_->end());
	return child_it->get();
}


root_node::root_node(std::string n, std::shared_ptr<parallel_region> r)
		: forest_(std::make_unique<forest_t>()),
		  tree_root(nullptr)
{
	auto temp_it = adobe::trailing_of(
	        forest_->insert(forest_->begin(),
	                std::make_unique<owning_base_node>(r, n, forest_.get())));
	tree_root = static_cast<owning_base_node*>(temp_it->get());
	tree_root->self_ = temp_it;
	assert(forest_);
	assert(tree_root);
}

}
