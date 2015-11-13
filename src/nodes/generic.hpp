#ifndef SRC_NODES_GENERIC_HPP_
#define SRC_NODES_GENERIC_HPP_

#include <core/traits.hpp>
#include <ports/ports.hpp>

#include <utility>
#include <map>

namespace fc
{

/**
 * \brief generic unary node which applys transform with parameter to all inputs
 *
 * \tparam bin_op binary operator, argument is input of node, second is parameter
 *
 * \pre bin_op needs to be callable with two argments
 */
template<class bin_op>
struct transform_node
{
	static_assert(utils::function_traits<bin_op>::arity == 2,
			"operator in transform node needs to take two parameters");
	typedef typename result_of<bin_op>::type result_type;
	typedef typename argtype_of<bin_op,1>::type param_type;
	typedef typename argtype_of<bin_op,0>::type data_t;

	explicit transform_node(bin_op op) : op(op) {}

	state_sink<param_type> param;

	decltype(auto) operator()(const data_t& in)
	{
		return op(in, param());
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
template<class data_t, class tag, class key_t = size_t> class n_ary_switch {};

template<class data_t, class key_t>
class n_ary_switch<data_t, state_tag, key_t>
{
public:
	typedef typename in_port<data_t, state_tag>::type port_t;

	n_ary_switch() :
		index() ,
		in_ports() ,
		out_port([this](){return  in_ports.at(index())();})
	{
	}

	/**
	 * \brief input port for state of type data_t corresponding to key port.
	 *
	 * \returns input port corresponding to key
	 * \param port, key by which port is identified.
	 * \post !in_ports.empty()
	 */
	auto in(key_t port) noexcept { return in_ports[port]; }
	/// parameter port controlling the switch, expects state of key_t
	auto control() const noexcept { return index; }
	auto out() const noexcept { return out_port; }
private:
	state_sink<key_t> index;
	std::map<key_t, port_t> in_ports;
	state_source_call_function<data_t> out_port;
};

template<class data_t, class key_t>
class n_ary_switch<data_t, event_tag, key_t>
{
public:
	typedef typename in_port<data_t, event_tag>::type port_t;

	/**
	 * \brief Get port by key. Creates port if none was found for key.
	 *
	 * \returns input port corresponding to key
	 * \param port, key by which port is identified.
	 * \post !in_ports.empty()
	 */
	auto in(key_t port)
	{
		if (in_ports.find(port) == end(in_ports))
		{
			in_ports.emplace(std::make_pair(
					port,
					port_t([this, port](const data_t& in)
							{ forward_call(in, port); }))
			);
		} //else the port already exists, we can just return it

		assert(!in_ports.empty());
		return in_ports.at(port);
	};

	/// output port of events of type data_t.
	auto out() const noexcept { return out_port; }
	/// parameter port controlling the switch, expects state of key_t
	auto control() const noexcept { return index; }


private:
	state_sink<size_t> index;
	event_out_port<data_t> out_port;
	std::map<key_t, port_t> in_ports;
	/// fires incoming event if and only if it is from the currently chosen port.
	void forward_call(data_t event, key_t port)
	{
		assert(!in_ports.empty());
		assert(in_ports.find(port) != end(in_ports));

		if (port == index())
			out().fire(event);
	}
};

}  // namespace fc

#endif /* SRC_NODES_GENERIC_HPP_ */
