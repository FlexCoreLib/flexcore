#include <flexcore/extended/base_node.hpp>

#include <stack>

namespace fc
{
static forest_t::iterator find_self(forest_t& forest, const tree_base_node* node)
{
	auto self = std::find_if(forest.begin(), forest.end(), [=](auto& other_uniq_ptr)
	                         {
		                         return node == other_uniq_ptr.get();
	                         });
	assert(self != forest.end());
	return adobe::trailing_of(self);
}

static constexpr auto name_seperator = "/";

std::string full_name(forest_t& forest,
                      const tree_base_node* node)
{
	auto position = find_self(forest, node);
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
		forest_graph* fg,
		std::shared_ptr<parallel_region> r,
		std::string name)
	: fg_(fg)
	, region_(r)
	, graph_info_(name)
{
	assert(fg_);
	assert(region_);
}

std::string tree_base_node::name() const
{
	return graph_info_.name();
}

graph::graph_node_properties tree_base_node::graph_info() const
{
	return graph_info_;
}

graph::connection_graph& tree_base_node::get_graph()
{
	return fg_->graph;
}

forest_t::iterator owning_base_node::self() const
{
	return find_self(fg_->forest, this);
}

fc::tree_base_node* owning_base_node::add_child(std::unique_ptr<tree_base_node> child)
{
	assert(fg_);
	assert(child);
	auto& forest = fg_->forest;
	auto child_it = adobe::trailing_of(forest.insert(self(), std::move(child)));
	assert(adobe::find_parent(child_it) == self());
	assert(adobe::find_parent(child_it) != forest.end());
	return child_it->get();
}


forest_owner::forest_owner(std::string n, std::shared_ptr<parallel_region> r)
		: fg_(std::make_unique<forest_graph>()),
		  tree_root(nullptr)
{
	assert(fg_);
	auto& forest = fg_->forest;
	auto temp_it = adobe::trailing_of(
	        forest.insert(forest.begin(),
	                std::make_unique<owning_base_node>(fg_.get(), r, n)));
	tree_root = static_cast<owning_base_node*>(temp_it->get());
	assert(tree_root);
}

}
