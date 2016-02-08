#ifndef SRC_NODES_STATE_NODES_HPP_
#define SRC_NODES_STATE_NODES_HPP_

#include <core/traits.hpp>
#include <ports/ports.hpp>
#include <nodes/base_node.hpp>
#include <ports/states/state_sources.hpp>

#include <ports/events/event_sinks.hpp>

#include <utility>
#include <tuple>
#include <memory>
#include <cstddef>

namespace fc
{

/**
 * \brief merges all input states to one output state by given operation
 *
 * \tparam operation operation to apply to all inputs, returns desired output.
 *
 * example:
 * \code{.cpp}
 *	auto multiply = merge([](int a, int b){return a*b;});
 *	[](){ return 3;} >> multiply.in<0>();
 *	[](){ return 2;} >> multiply.in<1>();
 *	BOOST_CHECK_EQUAL(multiply(), 6);
 * \endcode
 */
template<class operation, class signature>
struct merge_node;

template<class operation, class result, class... args>
struct merge_node<operation, result (args...)> : public tree_base_node
{
	typedef std::tuple<args...> arguments;
	typedef std::tuple<state_sink<args> ...> in_ports_t;
	typedef result result_type;
	static constexpr auto nr_of_arguments = sizeof...(args);

	static_assert(nr_of_arguments > 0,
			"Tried to create merge_node with a function taking no arguments");

    explicit merge_node(operation o)
		: tree_base_node("merger")
  		, in_ports(state_sink<args>(this)...)
		, op(o)
	{}

	///calls all in ports, converts their results from tuple to varargs and calls operation
	result_type operator()()
	{
		return invoke_helper(in_ports, std::make_index_sequence<nr_of_arguments>{});
	}

	template<size_t i>
	auto& in() noexcept { return std::get<i>(in_ports); }

protected:
	in_ports_t in_ports;
	operation op;

private:
	///Helper function to get varargs index from nr of arguments by type deduction.
	template<class tuple, std::size_t... index>
	decltype(auto) invoke_helper(tuple&& tup, std::index_sequence<index...>)
	{
		return op(std::get<index>(std::forward<tuple>(tup)).get()...);
	}
};

///creats a merge node which applies the operation to all inputs and returns single state.
template<class parent_t, class operation>
auto make_merge(parent_t& parent, operation op)
{
	typedef merge_node
			<	operation,
				typename utils::function_traits<operation>::function_type
			> node_t;
	return parent.template make_child<node_t>(op);
}

/*****************************************************************************/
/*                                   Caches                                  */
/*****************************************************************************/

/// Pulls inputs on incoming pull tick and makes it available to state output out().
template<class data_t>
class current_state : public tree_base_node
{
public:
	explicit current_state(const data_t& initial_value = data_t()) :
			tree_base_node("cache"),
			pull_tick([this]()
			{
				stored_state = in_port.get();
			}),
			in_port(this),
			out_port(this, [this](){ return stored_state;}),
			stored_state(initial_value)
	{
	}

	///events to this port trigger the node to pull its inputs. expects void
	auto& pull() noexcept { return pull_tick; }
	///State Input Port of type data_t
	auto& in() noexcept { return in_port; }
	///State Output Port of type data_t
	auto& out() noexcept { return out_port; }

private:
	pure::event_sink<void> pull_tick;
	state_sink<data_t> in_port;
	state_source<data_t> out_port;
	data_t stored_state;
};

/**
 * \brief Caches state and only pulls new state when cache is marked as dirty.
 *
 * switch_tick port needs to be connected,
 * as events to this port mark the cache as dirty.
 */
template<class data_t>
class state_cache : public tree_base_node
{
public:
	state_cache() :
		tree_base_node("cache"),
		cache(std::make_shared<data_t>()),
		load_new(true),
		in_port(this)
	{
	}

	///State Output Port of type data_t
	auto out() noexcept
	{
		return [this]()
		{
			if (load_new)
				refresh_cache();
			return *cache;
		};
	}

	///State Input Port of type data_t
	auto& in() noexcept { return in_port; }

	///Events to this port mark the cache as dirty. Expects events of type void.
	auto switch_tick() noexcept { return [this](){ load_new = true; }; }

private:
	void refresh_cache()
	{
		*cache = in_port.get();
		load_new = false;
	}
	std::shared_ptr<data_t> cache;
	bool load_new;
	state_sink<data_t> in_port;
};

} // namespace fc

#endif /* SRC_NODES_STATE_NODES_HPP_ */
