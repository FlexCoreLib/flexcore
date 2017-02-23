#ifndef SRC_NODES_EVENT_NODES_HPP_
#define SRC_NODES_EVENT_NODES_HPP_

#include <flexcore/pure/state_sink.hpp>
#include <flexcore/pure/event_sinks.hpp>
#include <flexcore/pure/event_sources.hpp>
#include <flexcore/pure/pure_node.hpp>

#include <map>
#include <utility>

namespace fc
{

/**
 * \brief Generic unary node which only handles events.
 *
 * Use this as base-class to simply build nodes which handle events.
 *
 * \tparam input_t type of event accepted as inputs by the node
 * \tparam output_t type of event sent by this node
 * \ingroup nodes
 */
template<class input_t, class output_t, class base>
class generic_event_node : public base
{
public:
	/**
	 * \brief constructs generic_event_node with action to be executed
	 * when event is received.
	 *
	 * \param action action to execute when an event is received at in().
	 */
	template<class action_t, class... base_args>
	explicit generic_event_node(action_t&& action, base_args&&... args) :
		base(std::forward<base_args>(args)...),
		in_port(this, action), out_port(this)
	{
	}

	/// Event Sink expecting event_t.
	auto& in() noexcept
	{
		return in_port;
	}
	/// Event Source firing event_t.
	auto& out() noexcept
	{
		return out_port;
	}
protected:
	typename base::template event_sink<input_t> in_port;
	typename base::template event_source<output_t> out_port;
};

/**
 * \brief forwards events if and only if a predicate returns true.
 *
 * \tparam event_t type of event expected and forwarded
 * \tparam predicate type of predicate evaluated on event
 * needs to be callable with type convertible from event_t and return bool.
 * \ingroup nodes
 */
template<class event_t, class predicate, class base_t = pure::pure_node>
class gate_with_predicate: public generic_event_node<event_t, event_t, base_t>
{
public:
	template<class... base_args>
	explicit gate_with_predicate(const predicate& p, base_args&&... args) :
		generic_event_node<event_t, event_t, base_t>(
				[this](const event_t& in)
				{
					if (pred(in))
						this->out_port.fire(in);
				},
				std::forward<base_args>(args)...),
			pred(p)
	{
	}

private:
	predicate pred;
};

/**
 * \brief Forwards events if and only if the state at in_control is true.
 *
 * State sink in_control must be connected when events are received!
 *
 * \tparam event_t type of event expected and forwarded
 * \ingroup nodes
 */
template<class event_t, class base_t = pure::pure_node>
class gate_with_control: public generic_event_node<event_t, event_t, base_t>
{
public:
	template<class... base_args>
	explicit gate_with_control(base_args&&... args) :
		generic_event_node<event_t, event_t, base_t>(
				[this](auto&&... in)
				{
					if (control.get())
						this->out_port.fire(std::forward<decltype(in)>(in)...);
				},
				std::forward<base_args>(args)...),
			control(this)
	{
	}
	/// State sink expecting bool. Events are forwarded if this state is true.
	auto& in_control() noexcept { return control; }

private:
	typename base_t::template state_sink<bool> control;
};

/// Creates gate_with_predicate with predicate p of type event_t.
template<class event_t, class predicate>
auto gate(const predicate& p)
{
	return gate_with_predicate<event_t, predicate>{p};
}

/// Creates gate_with_control of type event_t.
template<class event_t>
auto gate()
{
	return gate_with_control<event_t>();
}

/**
 * \brief Takes a pair of \p key_t and \p data_t as inputs splits them to separate ports.
 *
 * pair_splitter has one output port per key.
 * On incoming std::pair<key_t, data_t> it sends the second element of the pair
 * out on the output port corresponding to the first element (the key).
 * \tparam data_t type of event expected and forwarded
 * \tparam key_t type of key used in pair, needs to provide operator <
 * \ingroup nodes
 * \see pair_joiner
 */
template<class key_t, class data_t, class base = pure::pure_node>
class pair_splitter : public base
{
public:
	using in_port_t = typename base::template event_sink<std::pair<key_t, data_t>>;
	using out_port_t = typename base::template event_source<data_t>;

	template<class... base_args>
	explicit pair_splitter(base_args&&... args) :
		base(std::forward<base_args>(args)...),
		in_port{this,
			[this](const std::pair<key_t, data_t>& in)
			{
				this->out(in.first).fire(in.second);
			}},
		out_ports{}
	{
	}

	/// Event sink expecting std::pair<key_t, data_t>
	auto& in() { return in_port; }

	/// event_source sending data_t
	out_port_t& out(const key_t& key)
	{
		auto it = out_ports.find(key);
		if (it == out_ports.end())
			it = out_ports.insert(std::make_pair(key, out_port_t{this})).first;
		return it->second;
	}
private:
	in_port_t in_port;
	std::map<key_t, out_port_t> out_ports;
};

/**
 * \brief Has a map of input ports and sends pairs of key and value as output.
 *
 * On incoming data on port key it sends a std::pair<key_t, data_t> as output.
 * \tparam data_t type of event expected and forwarded
 * \tparam key_t type of key used in pair, needs to provide operator <
 * \ingroup nodes
 * \see pair_splitter
 */
template<class key_t, class data_t, class base = pure::pure_node>
class pair_joiner : public base
{
public:
	template<class... base_args>
	explicit pair_joiner(base_args&&... args) :
		base(std::forward<base_args>(args)...),
		in_ports{},
		out_port{this}
	{
	}

	///event_sink expecing data_t
	auto& in(const key_t& id)
	{
		auto fire_pair = [this, id](data_t input){
			out_port.fire(std::make_pair(id, input));
		};

		auto it = in_ports.find(id);
		if (it == in_ports.end())
			it = in_ports.insert(
					std::make_pair(id, in_port_t{this,fire_pair})
					).first;
		return it->second;
	}

	///event_source sending std::pair<key_t, data_t>
	auto& out() { return out_port; }

private:
	using in_port_t = typename base::template event_sink<data_t>;
	using out_port_t = typename base::template event_source<std::pair<key_t, data_t>>;

	std::map<key_t, in_port_t> in_ports;
	out_port_t out_port;
};

} // namespace fc

#endif /* SRC_NODES_EVENT_NODES_HPP_ */
