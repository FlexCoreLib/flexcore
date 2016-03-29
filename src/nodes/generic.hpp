#ifndef SRC_NODES_GENERIC_HPP_
#define SRC_NODES_GENERIC_HPP_

#include <core/traits.hpp>
#include <ports/token_tags.hpp>

#include <ports/ports.hpp>
#include <ports/pure_ports.hpp>

#include <nodes/pure_node.hpp>
#include <nodes/base_node.hpp>

#include <utility>
#include <map>

namespace fc
{
/**
 * \brief generic unary node which applies transform with parameter to all inputs
 *
 * \tparam bin_op binary operator, argument is input of node, second is parameter
 *
 * \pre bin_op needs to be callable with two arguments
 */
template<class bin_op>
struct transform_node// : node_interface
{
	static_assert(utils::function_traits<bin_op>::arity == 2,
			"operator in transform node needs to take two parameters");
	typedef result_of_t<bin_op> result_type;
	typedef typename argtype_of<bin_op,1>::type param_type;
	typedef typename argtype_of<bin_op,0>::type data_t;

	explicit transform_node(bin_op op)
		: param()
		, op(op) {}

	pure::state_sink<param_type> param;

	decltype(auto) operator()(const data_t& in)
	{
		return op(in, param.get());
	}

private:
	bin_op op;
};

/// creates transform_node with op as operation.
template<class bin_op>
auto transform(bin_op op)
{
	return transform_node<bin_op>(op);
}

/**
 * \brief n_ary_switch forwards one of n inputs to output
 *
 * Simply connect new input ports to add them to the set for the switch.
 * The switch itself is controlled by the port "control" which needs to be connected
 * to a state source of a type convertible to key_t.
 * is specialized for state and events, as the implementations differ.
 *
 * \tparam data_t type of data flowing through the switch
 * \tparam tag, either event_tag or state_tag to set switch to event handling
 * or forwarding of state
 *
 * \key_t key for lookup of inputs in switch. needs to have operator < and ==
 */
template<class data_t,
		class tag,
		class key_t = size_t,
		class base_node = tree_base_node
		> class n_ary_switch;

template<class data_t, class key_t, class base_node>
class n_ary_switch<data_t, state_tag, key_t, base_node> : public base_node
{
public:
	template<class... base_args>
	explicit n_ary_switch(base_args&&... args)
		: base_node(std::forward<base_args>(args)...)
		, switch_state(this)
		, in_ports()
		, out_port(this, [this](){return in_ports.at(switch_state.get()).get();} )
	{}

	using data_sink_t = typename base_node::template state_sink<data_t>;
	using key_sink_t = typename base_node::template state_sink<key_t>;
	using state_source_t = typename base_node::template state_source<data_t>;

	/**
	 * \brief input port for state of type data_t corresponding to key port.
	 *
	 * \returns input port corresponding to key
	 * \param port, key by which port is identified.
	 * \post !in_ports.empty()
	 */
	auto& in(key_t port) noexcept
	{
		auto it = in_ports.find(port);
		if (it == in_ports.end())
			it = in_ports.emplace(std::make_pair(port,
					data_sink_t{this})).first;
		return it->second;
	}
	/// parameter port controlling the switch, expects state of key_t
	auto& control() noexcept { return switch_state; }
	auto& out() noexcept { return out_port; }
private:
	/// provides the current state of the switch.
	key_sink_t switch_state;
	std::map<key_t, data_sink_t> in_ports;
	state_source_t out_port;
};

template<class data_t, class key_t, class base_node>
class n_ary_switch<data_t, event_tag, key_t, base_node> : public base_node
{
public:
	using data_sink_t = typename base_node::template event_sink<data_t>;
	using key_sink_t = typename base_node::template state_sink<key_t>;
	using event_source_t = typename base_node::template event_source<data_t>;


	template<class... base_args>
	explicit n_ary_switch(base_args&&... args)
		: base_node(std::forward<base_args>(args)...)
		, switch_state(this)
		, out_port(this)
		, in_ports()
	{}

	/**
	 * \brief Get port by key. Creates port if none was found for key.
	 *
	 * \returns input port corresponding to key
	 * \param port, key by which port is identified.
	 * \post !in_ports.empty()
	 */
	auto& in(key_t port)
	{
		auto it = in_ports.find(port);
		if (it == end(in_ports))
		{
			it = in_ports.emplace(std::make_pair
				(	port,
					data_sink_t( this,
							[this, port](const data_t& in){ forward_call(in, port); })
				)
			).first;
		} //else the port already exists, we can just return it

		return it->second;
	};

	/// output port of events of type data_t.
	auto& out() noexcept { return out_port; }
	/// parameter port controlling the switch, expects state of key_t
	auto& control() noexcept { return switch_state; }


private:
	key_sink_t switch_state;
	event_source_t out_port;
	std::map<key_t, data_sink_t> in_ports;
	/// fires incoming event if and only if it is from the currently chosen port.
	void forward_call(data_t event, key_t port)
	{
		assert(!in_ports.empty());
		assert(in_ports.find(port) != end(in_ports));

		if (port == switch_state.get())
			out().fire(event);
	}
};

/**
 * \brief node which observes a state and fires an event if the state matches a predicate.
 *
 * Needs to be connected to a tick, which triggers the check of the predicate on the state.
 *
 * \tparam data_t type of data watched by the watch_node.
 * \tparam predicate predicate which is tested on the observed state
 * predicate needs to be a callable which takes objects convertible from data_t
 * and returns a bool.
 */
template<class data_t, class predicate, class base_node>
class watch_node : public base_node
{
public:
	template<class... base_args>
	explicit watch_node(predicate pred, base_args&&... args)
		: base_node(std::forward<base_args>(args)...)
		, pred{std::move(pred)}
		, in_port(this)
		, out_port(this)
	{
	}

	watch_node(watch_node&&) = default;

	/// State input port, expects data_t.
	auto& in() noexcept { return in_port; }
	/// Event Output port, fires data_t.
	auto& out() noexcept { return out_port; }

	/// Event input port expects event of type void. Usually connected to a work_tick.
	auto check_tick()
	{
		return [this]()
		{
			const auto tmp = in_port.get();
			if (pred(tmp))
				out_port.fire(tmp);
		};
	}

private:
	predicate pred;
	typename base_node::template state_sink<data_t> in_port;
	typename base_node::template event_source<data_t> out_port;
};

/// Creates a watch node with a predicate.
template<class data_t, class predicate>
auto watch(predicate&& pred, data_t)
{
	return watch_node<data_t, predicate, pure::pure_node>{std::forward<predicate>(pred)};
}

/**
 * \brief Creates a watch_node, which fires an event, if the state changes.
 *
 *  Does not fire the first time the state is querried.
 */
template<class data_t>
auto on_changed(data_t initial_value = data_t())
{
	return watch(
			[last = std::make_unique<data_t>()](const data_t& in) mutable
			{
				const bool is_same = last && (*last == in);
				last = std::make_unique<data_t>(in);
				return !is_same;
			},
			initial_value);
}

}  // namespace fc

#endif /* SRC_NODES_GENERIC_HPP_ */
