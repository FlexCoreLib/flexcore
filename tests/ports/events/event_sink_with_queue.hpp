#ifndef SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_
#define SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_

#include <cassert>
#include <functional>
#include <queue>

#include <core/traits.hpp>
#include <core/connection.hpp>
#include <ports/fancy_ports.hpp>

namespace fc
{
namespace pure
{

/**
 * \brief minimal input port for events
 *
 * Currently the port is only for use in the unittests.
 * There is no check for the buffer size. If you want to use this port in
 * production code, you need to add a size check!
 *
 * fulfills passive_sink
 * \tparam event_t type of event expected, must be copy_constructable
 */
template<class event_t>
struct event_sink_queue
{
	explicit event_sink_queue()
		: queue(new std::queue<event_t>)
	{}

	template<class T>
	void operator()(T&& in_event)
	{
		queue->push(std::forward<T>(in_event));
	}

	bool empty() { return queue->empty(); }

	auto get()
	{
		if (empty())
			throw std::runtime_error("queue emtpy.");
		auto result = std::move(queue->front());
		queue->pop();
		return result;
	}

private:
	std::shared_ptr<std::queue<event_t>> queue;
};

} // namespace pure

// traits
//template<class T> struct is_port<pure::event_sink_queue<T>> : public std::true_type {}; // FIXME
template<class T> struct is_passive_sink<pure::event_sink_queue<T>> : public std::true_type {};

template<class data_t>
using event_sink_queue = default_mixin<pure::event_sink_queue<data_t>>;

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_SINK_WITH_QUEUE_HPP_ */
