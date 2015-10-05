#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <functional>
#include <memory>
#include <vector>

#include <core/traits.hpp>
#include <core/connection.hpp>

#include <ports/port_traits.hpp>

#include <iostream>

namespace fc
{


template<class event_t>
struct event_out_port
{
	typedef typename detail::handle_type<event_t>::type handler_t;

	template<class... T>
	void fire(T... event)
	{
		for (auto target : *event_handlers)
			target(event...);
	}

	auto connect(handler_t new_handler)
	{
		event_handlers->push_back(new_handler);
		return port_connection<decltype(this), handler_t>();
	}

private:

	// stores event_handlers in shared vector, since the port is stored in a node
	// but will be copied, when it is connected. The node needs to send
	// to all connected event_handlers, when an event is fired.
	typedef std::vector<handler_t> handler_vector;
	std::shared_ptr<handler_vector> event_handlers = std::make_shared<handler_vector>();
};

// traits
// TODO prefer to test this algorithmically
template<class T> struct is_port<event_out_port<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
