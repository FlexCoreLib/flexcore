#ifndef SRC_PORTS_EVENTS_EVENT_SINKS_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINKS_HPP_

#include <cassert>
#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>


namespace fc
{

/**
 * \brief minimal input port for events
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_in_port
{
	typedef typename detail::handle_type<event_t>::type handler_t;
	explicit event_in_port(const handler_t& handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()(const event_t& in_event) // universal ref here?
	{
		assert(event_handler);
		event_handler(in_event);
	}

	event_in_port() = delete;

private:
	handler_t event_handler;

};

/// specialisation of event_in_port with void , necessary since operator() has no parameter.
template<>
struct event_in_port<void>
{
	typedef typename detail::handle_type<void>::type handler_t;
	explicit event_in_port(handler_t handler) :
			event_handler(handler)
	{
		assert(event_handler);
	}

	void operator()()
	{
		assert(event_handler);
		event_handler();
	}
	event_in_port() = delete;
private:
	handler_t event_handler;
};

/**
 */
struct default_port_tag {};

/**
 * \brief minimal input port for events with templated operator()
 *
 * fulfills passive_sink
 *
 * \tparam node_t type of owning node
 * \tparam tag_t is used as a template parameter for calling detail_in
 *         so the implementation of the node can distinguish between different ports.
 *         The value is not used.
 */
template<class node_t, class tag_t = default_port_tag>
class event_in_tmpl
{
public:
	explicit event_in_tmpl(node_t& n) :
			node(n)
	{}
	event_in_tmpl() = delete;

	/**
	 * \tparam event_t type of event expected
	 */
	template<class event_t>
	void operator()(const event_t& in_event) // universal ref here?
	{
		node.detail_in(tag_t(), in_event);
	}

private:
	node_t& node;
};

// traits
template<class T> struct is_port<event_in_port<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_port<T>> : std::true_type {};
template<class T, class U> struct is_passive_sink<event_in_tmpl<T, U>> : std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINKS_HPP_ */
