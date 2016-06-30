#ifndef SRC_NODES_STATE_NODES_HPP_
#define SRC_NODES_STATE_NODES_HPP_

#include <flexcore/core/traits.hpp>
#include <flexcore/core/tuple_meta.hpp>
#include <flexcore/extended/ports/node_aware.hpp>
#include <flexcore/pure/mux_ports.hpp>
#include <flexcore/extended/base_node.hpp>
#include <flexcore/pure/pure_node.hpp>
#include <flexcore/extended/nodes/region_worker_node.hpp>

#include <utility>
#include <tuple>
#include <memory>
#include <cstddef>

namespace fc
{

/** \addtogroup nodes
 *  @{
 */

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
	using result_t = result ;

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
	result_t operator()()
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
 * @return reference to created merge_node
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

///creates a merge node which applies the operation to all inputs and returns single state.
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

/**
 * \brief Merges inputs combining incoming elements to a range of elements.
 *
 * Incoming ranges will thus be converted to a range of ranges.
 *
 * \tparam data_t type of data flowing through node.
 * \tparam out_container_t type of range used as output. Default is std::vector
 *
 */
template<class data_t, class base_t, class out_container_t = std::vector<data_t>>
class dynamic_merger final : public base_t
{
public:
	using in_port_t = typename base_t::template state_sink<data_t>;
	using out_port_t = typename base_t::template state_source<out_container_t>;

	static constexpr auto default_name = "merger";

	template<class... args_t>
	explicit dynamic_merger(args_t&&... args) :
		base_t(std::forward<args_t>(args)...),
		in_ports(),
		out_port(this,[this](){return merge_inputs();})
	{
	}

	/// state_sink of type data_t, creates a new port for each call.
	in_port_t& in()
	{
		in_ports.emplace_back(std::make_unique<in_port_t>(this));
		return *(in_ports.back());
	}

	/// State Output Port of type out_container_t<data_t>.
	out_port_t& out() { return out_port; }

private:
	out_container_t merge_inputs()
	{
		out_container_t out_buffer;
		for(auto& port : in_ports)
		{
			out_buffer.push_back(port->get());
		}

		return out_buffer;
	}

	std::vector<std::unique_ptr<in_port_t>> in_ports;
	out_port_t out_port;
};

/*****************************************************************************/
/*                                   Caches                                  */
/*****************************************************************************/

/// Pulls inputs on incoming pull tick and makes it available to state output out().
template<class data_t>
class current_state : public region_worker_node
{
public:
	static constexpr auto default_name = "cache";

	explicit current_state(const detail::node_args& node)
		: current_state(data_t{}, node)
	{
	}
	explicit current_state(const data_t& initial_value, const detail::node_args& node)
		: region_worker_node(
			[this]()
			{
				stored_state = in_port.get();
			}, node),
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
		in_port(this),
		out_port(this, [this]()
		{
			if (load_new)
				refresh_cache();
			return *cache;
		}),
		update_port(this,  [this](){ load_new = true; })
	{
	}

	/// State Output Port of type data_t
	auto& out() noexcept { return out_port; }

	/// State Input Port of type data_t
	auto& in() noexcept { return in_port; }

	/// Events to this port mark the cache as dirty. Expects events of type void.
	auto& update() noexcept { return update_port; }

private:
	void refresh_cache()
	{
		*cache = in_port.get();
		load_new = false;
	}
	std::unique_ptr<data_t> cache;
	bool load_new;
	typename base_t::template state_sink<data_t> in_port;
	typename base_t::template state_source<data_t> out_port;
	typename base_t::template event_sink<void> update_port;
};

/** @} doxygen group nodes */

} // namespace fc

#endif /* SRC_NODES_STATE_NODES_HPP_ */
