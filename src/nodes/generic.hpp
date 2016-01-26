#ifndef SRC_NODES_GENERIC_HPP_
#define SRC_NODES_GENERIC_HPP_

#include <core/traits.hpp>
#include <ports/token_tags.hpp>

#include <ports/ports.hpp>
#include <ports/pure_ports.hpp>

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
struct transform_node// : public node_interface
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
template<class data_t, class tag, class key_t = size_t> class n_ary_switch;

template<class data_t, class key_t>
class n_ary_switch<data_t, state_tag, key_t> : public tree_base_node
{
public:
	n_ary_switch()
		: tree_base_node("switch")
		, switch_state(this)
		, in_ports()
		, out_port(this, [this](){return in_ports.at(switch_state.get()).get();} )
	{}

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
			it = in_ports.emplace(std::make_pair(port, state_sink<data_t>(this))).first;
		return it->second;
	}
	/// parameter port controlling the switch, expects state of key_t
	auto& control() noexcept { return switch_state; }
	auto& out() noexcept { return out_port; }
private:
	/// provides the current state of the switch.
	state_sink<key_t> switch_state;
	std::map<key_t, state_sink<data_t>> in_ports;
	state_source<data_t> out_port;
};

template<class data_t, class key_t>
class n_ary_switch<data_t, event_tag, key_t> : public tree_base_node
{
public:
	n_ary_switch()
		: tree_base_node("switch")
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
					event_sink<data_t>( this,
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
	state_sink<key_t> switch_state;
	event_source<data_t> out_port;
	std::map<key_t, event_sink<data_t>> in_ports;
	/// fires incoming event if and only if it is from the currently chosen port.
	void forward_call(data_t event, key_t port)
	{
		assert(!in_ports.empty());
		assert(in_ports.find(port) != end(in_ports));

		if (port == switch_state.get())
			out().fire(event);
	}
};

}  // namespace fc

#endif /* SRC_NODES_GENERIC_HPP_ */
