#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <functional>

#include <core/traits.hpp>
#include <core/connection.hpp>

namespace fc
{

template<class event_t>
using handler_t = std::function<void(event_t)>;

template<class event_t>
struct event_out_port
{
	void fire(event_t event)
	{
		for (auto target : *event_handlers)
			target(event);
	}

	void connect(std::function<void(event_t)> new_handler)
	{
		event_handlers->push_back(new_handler);
	}

private:

	// stores event_handlers in shared vector, since the port is stored in a node
	// but will be copied, when it is connected. The node needs to send
	// to all connected event_handlers, when an event is fired.
	typedef std::vector<handler_t<event_t>> handler_vector;
	std::shared_ptr<handler_vector> event_handlers = std::make_shared<handler_vector>();
};

//static_assert(not is_callable<event_out_port<int>>::value, "CALLABLE");

// traits
template<class T> struct is_callable<event_out_port<T>> : public std::false_type {};
template<class T> struct is_active_source<event_out_port<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
