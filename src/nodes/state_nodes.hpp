#ifndef SRC_NODES_STATE_NODES_HPP_
#define SRC_NODES_STATE_NODES_HPP_

#include <core/traits.hpp>
#include <ports/states/state_sink.hpp>

#include <utility>

namespace fc
{

/**
 * \brief merges all input states to one output state by given operation
 *
 * \tparam operation operation to apply to all inputs, returns desired output.
 */
template<class... operation>
struct merge_node
{
};

template<class operation, class result, class... args>
struct merge_node<operation, result (args...)>
{
	typedef std::tuple<args...> arguments;
	typedef std::tuple<state_sink<args> ...> in_ports_t;
	typedef result result_type;
	static constexpr auto nr_of_arguments = sizeof...(args);

	static_assert(nr_of_arguments > 0,
			"Tried to create merge_node wtih a function taking no arguments");

	merge_node(operation op) :
			op(op)
	{
	}

	///calls all in ports, converts their results from tuple to varargs and calls operation
	result_type operator()()
	{
		return invoke_helper(in_ports, std::make_index_sequence<nr_of_arguments>{});
	}

	template<size_t i>
	auto in() const { return std::get<i>(in_ports); }

	in_ports_t in_ports;
protected:
	operation op;

private:
	///Helper function to get varargs index from nr of arguments by type deduction.
	template<class tuple, std::size_t... index>
	decltype(auto) invoke_helper(tuple&& tup, std::index_sequence<index...>)
	{
		return op(std::get<index>(std::forward<tuple>(tup))()...);
	}
};

///creats a merge node which applies the operation to all inputs and returns single state.
template<class operation>
auto merge(operation op)
{
	return merge_node<
			operation,
			typename utils::function_traits<operation>::function_type
			>(op);
}


}  // namespace fc

#endif /* SRC_NODES_STATE_NODES_HPP_ */
