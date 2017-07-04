#ifndef SRC_GRAPH_TRAITS_HPP_
#define SRC_GRAPH_TRAITS_HPP_

namespace fc
{
namespace graph
{

///checks if a type has a member called graph_info
template<class T>
constexpr auto has_graph_info(int) -> decltype(std::declval<T>().graph_info, bool())
{
	return true;
}

template<class T>
constexpr bool has_graph_info(...)
{
	return false;
}


}  // namespace graph
}  // namespace fc

#endif /* SRC_GRAPH_TRAITS_HPP_ */
