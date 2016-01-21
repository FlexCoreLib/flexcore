#ifndef SRC_NODES_EVENT_NODES_HPP_
#define SRC_NODES_EVENT_NODES_HPP_

#include <ports/states/state_sink.hpp>
#include <ports/events/event_sinks.hpp>
#include <ports/events/event_sources.hpp>

namespace fc
{

/**
 * \brief generic unary node which only handles events
 *
 * \tparam event_t type of event handled by the node
 */
template<class event_t>
class generic_event_node
{
public:
	/**
	 * \brief constructs generic_event_node with action to be executed
	 * when event is received.
	 *
	 * \param action action to execute when an event is received at in().
	 */
	template<class action_t>
	explicit generic_event_node(action_t&& action) :
		in_port(action), out_port()
	{
	}

	/// Event Sink expecting event_t.
	auto in() noexcept
	{
		return in_port;
	}
	/// Event Source firing event_t.
	auto out() noexcept
	{
		return out_port;
	}
protected:
	event_in_port<event_t> in_port;
	event_out_port<event_t> out_port;
};

/**
 * \brief forwards events if and only if a predicate returns true.
 *
 * \tparam event_t type of event expected and forwarded
 * \pparam predicate type of predicate evaluated on event
 * needs to be callable with type convertible from event_t and return bool.
 */
template<class event_t, class predicate>
class gate_with_predicate: public generic_event_node<event_t>
{
public:
	explicit gate_with_predicate(const predicate& p) :
		generic_event_node<event_t>([this](const event_t& in)
		{
			if (pred(in))
			this->out_port.fire(in);
		}), pred(p)
	{
	}

private:
	predicate pred;
};

/**
 * \brief forwards events if and only if the state at in_control is true.
 *
 * \tparam event_t type of event expected and forwarded
 */
template<class event_t>
class gate_with_control: public generic_event_node<event_t>
{
public:
	gate_with_control() :
		generic_event_node<event_t>([this](const event_t& in)
		{
			if (control.get())
			this->out_port.fire(in);
		}), control()
	{
	}

	/// State sink expecting bool. Events are forwarded if this state is true.
	auto in_control() const noexcept { return control; }

private:
	state_sink<bool> control;
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

}  //fc

#endif /* SRC_NODES_EVENT_NODES_HPP_ */
