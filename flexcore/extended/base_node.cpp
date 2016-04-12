#include <flexcore/extended/base_node.hpp>

#include <stack>

namespace fc
{
using forest_t = tree_base_node::forest_t;

static forest_t::iterator find_self(forest_t* forest, const tree_base_node* node)
{
	auto self = std::find_if(forest->begin(), forest->end(), [=](auto& other_uniq_ptr)
	                         {
		                         return node == other_uniq_ptr.get();
	                         });
	assert(self != forest->end());
	return adobe::trailing_of(self);
}

static constexpr auto name_seperator = "/";

std::string full_name(forest_t& forest,
                      const tree_base_node* node)
{
	auto position = find_self(&forest, node);
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
	: region_(r)
	, graph_info_(name)
{
	assert(region_);
}

forest_t::iterator owning_base_node::self() const
{
	return find_self(this->forest_, this);
}

fc::tree_base_node* owning_base_node::add_child(std::unique_ptr<tree_base_node> child)
{
	assert(forest_);
	assert(child);
	auto child_it = adobe::trailing_of(forest_->insert(self(), std::move(child)));
	assert(adobe::find_parent(child_it) == self());
	assert(adobe::find_parent(child_it) != forest_->end());
	return child_it->get();
}


forest_owner::forest_owner(std::string n, std::shared_ptr<parallel_region> r)
		: forest_(std::make_unique<forest_t>()),
		  tree_root(nullptr)
{
	auto temp_it = adobe::trailing_of(
	        forest_->insert(forest_->begin(),
	                std::make_unique<owning_base_node>(r, n, forest_.get())));
	tree_root = static_cast<owning_base_node*>(temp_it->get());
	assert(forest_);
	assert(tree_root);
}

}
