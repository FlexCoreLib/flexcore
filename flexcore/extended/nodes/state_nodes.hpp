#ifndef SRC_NODES_STATE_NODES_HPP_
#define SRC_NODES_STATE_NODES_HPP_

#include <core/traits.hpp>
#include <core/tuple_meta.hpp>
#include <extended/ports/node_aware.hpp>
#include <pure/mux_ports.hpp>
#include <extended/base_node.hpp>
#include <pure/pure_node.hpp>
#include <extended/nodes/region_worker_node.hpp>

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
template<class operation, class signature, class base_t>
struct merge_node;

namespace detail
{
auto as_ref = [](auto& sink)
{
	return std::ref(sink);
};
}

template<class operation, class result, class... args, class base_t>
struct merge_node<operation, result (args...), base_t> : public base_t
{
	using arguments = std::tuple<args...>;
	template <typename arg>
	using base_sink_t = typename base_t::template state_sink<arg>;
	using result_type = result ;

	using in_ports_t = std::tuple<base_sink_t<args>...>;

	static constexpr auto nr_of_arguments = sizeof...(args);

	static_assert(nr_of_arguments > 0,
			"Tried to create merge_node with a function taking no arguments");

	template<class... ctr_args_t>
	explicit merge_node(operation o, ctr_args_t&&... ctr_args)
		: base_t(std::forward<ctr_args_t>(ctr_args)...)
		, in_ports(base_sink_t<args>(this)...)
		, op(o)
	{}

	///calls all in ports, converts their results from tuple to varargs and calls operation
	result_type operator()()
	{
		auto op = this->op;
		auto get_and_apply = [op](auto&&... sink)
		{
			return op(std::forward<decltype(sink)>(sink).get()...);
		};
		return tuple::invoke_function(get_and_apply, in_ports,
		                              std::make_index_sequence<nr_of_arguments>{});
	}

	/// State Sink corresponding to i-th argument of merge operation.
	template<size_t i>
	auto& in() noexcept { return std::get<i>(in_ports); }

	mux_port<base_sink_t<args>&...> mux() noexcept
	{
		return {tuple::transform(in_ports, detail::as_ref)};
	}

protected:
	in_ports_t in_ports;
	operation op;
};

/**
 * \brief creates a merge node which applies the operation to all inputs and returns single state.
 * @param parent nodes the created merge_node is attached to.
 * @param op operation to apply to inputs of merge_node
 * @return pointer to created merge_node
 */
template<class parent_t, class operation>
auto make_merge(parent_t& parent, operation op, std::string name = "merger")
{
	typedef merge_node
			<	operation,
				typename utils::function_traits<operation>::function_type,
				tree_base_node
			> node_t;
	return parent.template make_child<node_t>(op, name);
}

///creats a merge node which applies the operation to all inputs and returns single state.
template<class operation>
auto make_merge(operation op)
{
	typedef merge_node
			<	operation,
				typename utils::function_traits<operation>::function_type,
				pure::pure_node
			> node_t;
	return node_t{op};
}

/*****************************************************************************/
/*                                   Caches                                  */
/*****************************************************************************/

/// Pulls inputs on incoming pull tick and makes it available to state output out().
template<class data_t>
class current_state : public region_worker_node
{
public:
	explicit current_state(std::shared_ptr<parallel_region> r, const data_t& initial_value = data_t()) :
			region_worker_node(
			[this]()
			{
				stored_state = in_port.get();
			}, r, "cache"),
			in_port(this),
			out_port(this, [this](){ return stored_state;}),
			stored_state(initial_value)
	{
	}

	/// State Input Port of type data_t.
	auto& in() noexcept { return in_port; }
	/// State Output Port of type data_t.
	auto& out() noexcept { return out_port; }

private:
	state_sink<data_t> in_port;
	state_source<data_t> out_port;
	data_t stored_state;
};

/**
 * \brief Caches state and only pulls new state when cache is marked as dirty.
 *
 * event_sink update needs to be connected,
 * as events to this port mark the cache as dirty.
 */
template<class data_t, class base_t>
class state_cache : public base_t
{
public:
	template<class... args_t>
	explicit state_cache(args_t&&... args) :
	base_t(std::forward<args_t>(args)...),
		cache(std::make_unique<data_t>()),
		load_new(true),
		in_port(this)
	{
	}

	/// State Output Port of type data_t
	auto out() noexcept // todo: make lambda member and only return reference
	{
		return [this]()
		{
			if (load_new)
				refresh_cache();
			return *cache;
		};
	}

	/// State Input Port of type data_t
	auto& in() noexcept { return in_port; }

	/// Events to this port mark the cache as dirty. Expects events of type void.
	auto update() noexcept { return [this](){ load_new = true; }; } // todo: make lambda member and only return reference

private:
	void refresh_cache()
	{
		*cache = in_port.get();
		load_new = false;
	}
	std::unique_ptr<data_t> cache;
	bool load_new;
	typename base_t::template state_sink<data_t> in_port;
};

} // namespace fc

#endif /* SRC_NODES_STATE_NODES_HPP_ */
