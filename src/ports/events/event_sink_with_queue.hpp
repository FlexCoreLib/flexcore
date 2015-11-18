#ifndef SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_

#include <cassert>
#include <functional>
#include <queue>

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
struct event_in_queue
{
	explicit event_in_queue()
		: queue(new std::queue<event_t>)
	{}

	void operator()(const event_t& in_event)
	{
		queue->push(in_event);
	}

	bool empty() { return queue->empty(); }

	event_t get()
	{
		if (empty())
			throw std::runtime_error("queue emtpy.");
		event_t result { queue->front() };
		queue->pop();
		return result;
	}

private:
	std::shared_ptr<std::queue<event_t>> queue;
};

// traits
template<class T> struct is_port<event_in_queue<T>> : public std::true_type {};
template<class T> struct is_passive_sink<event_in_queue<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_ */
