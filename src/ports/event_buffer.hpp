#ifndef SRC_PORTS_EVENT_BUFFER_HPP_
#define SRC_PORTS_EVENT_BUFFER_HPP_

#include <functional>
#include <memory>

#include "event_ports.hpp"

namespace fc
{

template<class event_t>
struct buffer_interface
{
	buffer_interface() = default;
	virtual ~buffer_interface() = default;
	virtual event_in_port<event_t> in() = 0;
	virtual event_out_port<event_t> out() = 0;

	buffer_interface(const buffer_interface&) = delete;
	buffer_interface& operator= (const buffer_interface &) = delete;
};

template<class event_t>
class no_buffer : public buffer_interface<event_t>
{
public:
	no_buffer()
	: 	in_event_port( [this](event_t in_event) { out_event_port.fire(in_event);})
	{
	}

	event_in_port<event_t> in() override
	{
		return in_event_port;
	}
	event_out_port<event_t> out() override
	{
		return out_event_port;
	}

private:
	event_in_port<event_t> in_event_port;
	event_out_port<event_t> out_event_port;
};

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
class event_buffer : public buffer_interface<event_t>
{
public:

	event_buffer()
		: in_switch_tick( [this](){ switch_buffers(); } )
		, in_send_tick( [this](){ send_events(); } )
		, in_event_port( [this](event_t in_event) { intern_buffer.push_back(in_event);})
		, intern_buffer()
		, extern_buffer()
		{
		}

	// event in port of type void, switches buffers
	auto switch_tick() { return in_switch_tick; };
	// event in port of type void, fires external buffer
	auto send_tick() { return in_send_tick; };

	event_in_port<event_t> in() override
	{
		return in_event_port;
	}
	event_out_port<event_t> out() override
	{
		return out_event_port;
	}

	event_buffer(const event_buffer&) = delete;

protected:
	void switch_buffers()
	{

		// move content of intern buffer to extern, leaving content of extern buffer
		// since the buffers might be switched several times, before extern buffer is emptied.
		// otherwise we would potentially lose events on switch.
		extern_buffer.insert(end(extern_buffer), begin(intern_buffer), end(intern_buffer));
		intern_buffer.clear();
	}

	/**
	 * \sends all events stored in outgoing buffer to targets
	 *
	 * \post extern_buffer is empty
	 */
	void send_events()
	{
		for (event_t e : extern_buffer)
			out_event_port.fire(e);

		// delete content of extern buffer, do not change capacity,
		// since we want to avoid allocations in next cycle.
		extern_buffer.clear();
		assert(extern_buffer.empty());
	}

	event_in_port<void> in_switch_tick;
	event_in_port<void> in_send_tick;
	event_in_port<event_t> in_event_port;
	event_out_port<event_t> out_event_port;

	typedef std::vector<event_t> buffer_t;
	buffer_t intern_buffer;
	buffer_t extern_buffer;
};

} // namespace fc

#endif /* SRC_PORTS_EVENT_BUFFER_HPP_ */
