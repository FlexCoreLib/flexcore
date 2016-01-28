#ifndef SRC_NODES_STATE_NODES_HPP_
#define SRC_NODES_STATE_NODES_HPP_

#include <core/traits.hpp>
#include <ports/states/state_sink.hpp>
#include <ports/states/state_sources.hpp>

#include <ports/events/event_sinks.hpp>

#include <utility>

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
			"Tried to create merge_node with a function taking no arguments");

	explicit merge_node(operation op) :
			in_ports(),
			op(op)
	{
	}

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
template<class operation>
auto merge(operation op)
{
	return merge_node<
			operation,
			typename utils::function_traits<operation>::function_type
			>(op);
}


/*****************************************************************************/
/*                                   Caches                                  */
/*****************************************************************************/

/// pulls inputs on incoming pull tick and makes it available to state otuput out().
template<class data_t>
class current_state
{
public:
	explicit current_state(const data_t& initial_value = data_t()) :
			pull_tick([this]()
			{
				out_port.set(in_port.get());
			}),
			in_port(),
			out_port(initial_value)
	{
	}

	///events to this port trigger the node to pull its inputs. expects void
	auto& pull() noexcept { return pull_tick; }
	///State Input Port of type data_t
	auto& in() noexcept { return in_port; }
	///State Output Port of type data_t
	auto& out() noexcept { return out_port; }

private:
	event_in_port<void> pull_tick;
	state_sink<data_t> in_port;
	state_source_with_setter<data_t> out_port;
};

/**
 * \brief Caches state and only pulls new state when cache is marked as dirty.
 *
 * switch_tick port needs to be connected,
 * as events to this port mark the cache as dirty.
 */
template<class data_t>
class state_cache
{
public:
	state_cache() :
		cache(std::make_shared<data_t>()),
		load_new(true)
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

	///events to this port mark the cache as dirty. Expects events of type void.
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

}  // namespace fc

#endif /* SRC_NODES_STATE_NODES_HPP_ */
