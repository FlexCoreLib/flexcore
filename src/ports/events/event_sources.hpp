#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
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
	typedef event_t result_type;
	typedef typename detail::handle_type<event_t>::type handler_t;

	event_out_port() = default;
	event_out_port(const event_out_port&) = default;

	template<class... T>
	void fire(T... event)
	{
		std::cout << "event_out_port nr targets: " << event_handlers->size() << "\n";
		for (auto& target : (*event_handlers))
		{
			assert(target);
			target(event...);
		}
	}

	size_t nr_connected_handlers() const
	{
		return event_handlers->size();
	}

	auto connect(handler_t new_handler)
	{
		assert(new_handler); //connecting empty functions is illegal
		event_handlers->push_back(new_handler);
		std::cout << "event_out_port nr targets connect: " << event_handlers->size() << "\n";
		return port_connection<decltype(*this), handler_t>();
	}

private:

	// stores event_handlers in shared vector, since the port is stored in a node
	// but will be copied, when it is connected. The node needs to send
	// to all connected event_handlers, when an event is fired.
	typedef std::vector<handler_t> handler_vector;
	std::shared_ptr<handler_vector> event_handlers = std::make_shared<handler_vector>(0);
};

// traits
// TODO prefer to test this algorithmically
template<class T> struct is_port<event_out_port<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
