#ifndef SRC_PORTS_EVENT_BUFFER_HPP_
#define SRC_PORTS_EVENT_BUFFER_HPP_

#include <functional>
#include <memory>

#include "event_ports.hpp"

namespace fc
{

/**
 * \brief buffer for events using double buffering
 *
 * Stores events of event_t in two buffers.
 * Switches Buffers on receiving switch_tick.
 * This moves events from internal to external buffer.
 * New events are added to to the internal buffer.
 * Events from the external buffer are fired on receiving send tick.
 */
template<class event_t>
class event_buffer
{
public:

	event_buffer()
		: in_switch_tick( [this](){ switch_buffers(); } )
		, in_send_tick( [this](){ send_events(); } )
		, in_events( [this](event_t in_event) { intern_buffer->push_back(in_event);})
		, intern_buffer(std::make_shared<buffer_t>())
		, extern_buffer(std::make_shared<buffer_t>())
		{}

	// event in port of type void, switches buffers
	auto switch_tick() { return in_switch_tick; };
	// event in port of type void, fires external buffer
	auto send_tick() { return in_send_tick; };

protected:
	void switch_buffers()
	{
		// move content of intern buffer to extern, leaving content of extern buffer
		// since the buffers might be switched several times, before extern buffer is emptied.
		// otherwise we would potentially lose events on switch.
		extern_buffer->insert(
				end(*extern_buffer), begin(*intern_buffer), end(*intern_buffer));
		intern_buffer->clear();
	}

	void send_events()
	{
		for (event_t e : *extern_buffer)
			out_events.fire(e);

		// delete content of extern buffer, do not change capacity,
		// since we want to avoid allocations in next cycle.
		extern_buffer->clear();
	}

	event_in_port<void> in_switch_tick;
	event_in_port<void> in_send_tick;
	event_in_port<event_t> in_events;
	event_out_port<event_t> out_events;

	typedef std::vector<event_t> buffer_t;
	std::shared_ptr<buffer_t> intern_buffer;
	std::shared_ptr<buffer_t> extern_buffer;
};

/**
 * \brief Event buffer for events leaving the region.
 *
 * Fulfills is_passive_sink;
 *
 * include this in your region for outgoing events
 * and connect outputs of inner nodes to it.
 */
template<class event_t>
class exit_event : public event_buffer<event_t>
{
public:
	exit_event() = default;
	/**
	 * \brief receives events and default port
	 * \post intern_buffer is not empty.
	 */
	void operator()(event_t in_event)
	{
		this->in_events(in_event);
	}

	event_out_port<event_t> region_port() { return this->out_events ; }
};

/**
 * \brief Event buffer for events entering the region.
 *
 * Fulfills is_active_source;
 *
 * include this in your region for incoming events
 * and forward these to inner nodes.
 */
template<class event_t>
class enter_event : public event_buffer<event_t>
{
public:
	enter_event() = default;
	/**
	 * \brief receives events and default port
	 * \post intern_buffer is not empty.
	 */
	void operator()(event_t in_event)
	{
		this->in_events(in_event);
	}

	void connect(std::function<void(event_t)> new_handler)
	{
		this->out_events.connect(new_handler);
	}

	event_in_port<event_t> region_port() { return this->in_events ; }
};

// trait
template<class T> struct is_passive_sink<exit_event<T>> : public std::true_type {};
template<class T> struct is_active_source<enter_event<T>> : public std::true_type {};

// region connect
template<class source_t, class sink_t>
auto region_connect(source_t source, sink_t sink)
{
	return detail::connect_impl<
			decltype(sink.region_port()),
			decltype(source.region_port()) >
			()(source.region_port(), sink.region_port());
}

template<class source_t, class sink_t>
auto operator >>=(const source_t& source, const sink_t& sink)
{
	return region_connect(source, sink);
}

} // namespace fc

#endif /* SRC_PORTS_EVENT_BUFFER_HPP_ */
