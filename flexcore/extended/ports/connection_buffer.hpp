#ifndef SRC_PORTS_CONNECTION_BUFFER_HPP_
#define SRC_PORTS_CONNECTION_BUFFER_HPP_

#include <functional>
#include <memory>

#include <flexcore/pure/pure_ports.hpp>
#include <flexcore/extended/ports/token_tags.hpp>

namespace fc
{

//Note: Many asserts in this file might seem stupid (like checking empty after clear)
//but this code is multi-threaded and race conditions might trigger them.

/**
 * \brief common interface of nodes serving as buffers within connections.
 *
 * Classes implementing this are independent of the way they implement buffering.
 * Classes might even directly forward events.
 *
 * \tparam token_t type of tokens passing through the buffer
 * \tparam tag either event_tag or state_tag, specifies if events or states are buffered.
 */
template<class token_t, class tag>
struct buffer_interface
{
	typedef typename pure::out_port<token_t, tag>::type out_port_t;
	typedef typename pure::in_port<token_t, tag>::type in_port_t;

	buffer_interface() = default;
	virtual ~buffer_interface() = default;

	///input port for events, expects event_t
	virtual in_port_t& in() = 0;
	///output port for events, sends event_t
	virtual out_port_t& out() = 0;

	buffer_interface(const buffer_interface&) = delete;
	buffer_interface& operator= (const buffer_interface &) = delete;
};

/// Implementation of buffer_interface, which directly forwards events.
template<class token_t>
class event_no_buffer final : public buffer_interface<token_t, event_tag>
{
public:
	event_no_buffer()
		: in_event_port([this](auto&&... in_event)
		{
			out_event_port.fire(std::forward<decltype(in_event)>(in_event)...);
		})
	{
	}

	pure::event_sink<token_t>& in() override
	{
		return in_event_port;
	}
	pure::event_source<token_t>& out() override
	{
		return out_event_port;
	}

private:
	pure::event_sink<token_t> in_event_port;
	pure::event_source<token_t> out_event_port;
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
class event_buffer final : public buffer_interface<event_t, event_tag>
{
public:
	event_buffer()
		: switch_active_tick_([this] { switch_active_buffers(); })
		, switch_passive_tick_([this] { switch_passive_buffers(); })
		, switch_active_passive_tick_([this] { switch_active_passive_buffers(); })
		, in_send_tick( [this](){ send_events(); } )
		, in_event_port( [this](event_t in_event) { intern_buffer.push_back(in_event);})
		, intern_buffer()
		, extern_buffer()
		, read(false)
	{
	}

	typedef typename pure::out_port<event_t, event_tag>::type out_port_t;
	typedef typename pure::in_port<event_t, event_tag>::type in_port_t;

	/// event in port of type void, switches active-side buffers
	auto& switch_active_tick() { return switch_active_tick_; }
	/// event in port of type void, switches passive-side buffers
	auto& switch_passive_tick() { return switch_passive_tick_; }
	/// event in port of type void, directly switches active- and passive-side buffers
	auto& switch_active_passive_tick() { return switch_active_passive_tick_; }
	/// event in port of type void, fires outgoing buffer
	auto& work_tick() { return in_send_tick; }

	in_port_t& in() override { return in_event_port; }
	out_port_t& out() override { return out_event_port; }

private:
	/**
	 * \brief switches intern_buffer to middle_buffer
	 * \post intern_buffer.empty()
	 * \post read == false
	 */
	void switch_active_buffers()
	{
		// If middle buffer has been switched with outgoing_buffer, then we can swap the incoming
		// buffers without data loss. If middle buffer has not been read then data needs to be
		// appended.
		if (read)
			swap(intern_buffer, middle_buffer);
		else
			middle_buffer.insert(end(middle_buffer), begin(intern_buffer), end(intern_buffer));
		read = false;
		intern_buffer.clear();
		assert(intern_buffer.empty());
		assert(!read);
	}

	/**
	 * \brief switches middle_buffer to extern_buffer
	 * \post middle_buffer.empty()
	 * \post read ==true
	 */
	void switch_passive_buffers()
	{
		// Switching the outgoing buffers means the previous value in extern_buffer has already been
		// processed. So a new value is unconditionally needed. Swap should do.
		swap(middle_buffer, extern_buffer);
		read = true;
		middle_buffer.clear();
		assert(middle_buffer.empty());
		assert(read);
	}

	/**
	 * \brief directly switches intern to extern buffer
	 * \post intern_buffer.empty()
	 */
	void switch_active_passive_buffers()
	{
		if(extern_buffer.empty())
		{
			swap(intern_buffer, extern_buffer);
		}
		else
		{
			extern_buffer.insert(end(extern_buffer), begin(intern_buffer), end(intern_buffer));
			intern_buffer.clear();
		}
		assert(intern_buffer.empty());
	}

	/**
	 * \brief sends all events stored in outgoing buffer to targets
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

	pure::event_sink<void> switch_active_tick_;
	pure::event_sink<void> switch_passive_tick_;
	pure::event_sink<void> switch_active_passive_tick_;
	pure::event_sink<void> in_send_tick;
	in_port_t in_event_port;
	out_port_t out_event_port;

	typedef std::vector<event_t> buffer_t;
	buffer_t intern_buffer;
	buffer_t extern_buffer;
	buffer_t middle_buffer;
	bool read;
};

/**
 * \brief Template Specialization for events of type void
 *
 * Instead of real buffers we just count the events.
 */
template<>
class event_buffer<void> final : public buffer_interface<void, event_tag>
{
public:
	event_buffer()
		: switch_active_tick_([this] { switch_active_buffers(); })
		, switch_passive_tick_([this] { switch_passive_buffers(); })
		, switch_active_passive_tick_([this] { switch_active_passive_buffers(); })
		, in_send_tick( [this](){ send_events(); } )
		, in_event_port( [this]() { intern_buffer++;})
		, intern_buffer(0)
		, extern_buffer(0)
		, middle_buffer(0)
		, read(false)
		{
		}

	typedef typename pure::out_port<void, event_tag>::type out_port_t;
	typedef typename pure::in_port<void, event_tag>::type in_port_t;

	/// event in port of type void, switches active-side buffers
	auto& switch_active_tick() { return switch_active_tick_; }
	/// event in port of type void, switches passive-side buffers
	auto& switch_passive_tick() { return switch_passive_tick_; }
	/// event in port of type void, directly switches active- and passive-side buffers
	auto& switch_active_passive_tick() { return switch_active_passive_tick_; }
	/// event in port of type void, fires out port once for each event stored.
	auto& work_tick() { return in_send_tick; }

	in_port_t& in() override { return in_event_port; }
	out_port_t& out() override { return out_event_port; }

private:
	void switch_active_buffers()
	{
		if (read)
			middle_buffer = intern_buffer;
		else
			middle_buffer += intern_buffer;
		read = false;
		intern_buffer = 0;
	}

	void switch_passive_buffers()
	{
		// Switching the outgoing buffers means the previous value in extern_buffer has already been
		// processed. So a new value is unconditionally needed. Swap should do.
		std::swap(middle_buffer, extern_buffer);
		read = true;
		middle_buffer = 0;
	}

	void switch_active_passive_buffers()
	{
		extern_buffer += intern_buffer;
		intern_buffer = 0;
	}


	/**
	 * \brief sends all events stored in outgoing buffer to targets
	 * \post extern_buffer == 0
	 */
	void send_events()
	{
		for (size_t i = 0; i < extern_buffer; ++i)
			out_event_port.fire();

		extern_buffer = 0;
	}

	pure::event_sink<void> switch_active_tick_;
	pure::event_sink<void> switch_passive_tick_;
	pure::event_sink<void> switch_active_passive_tick_;
	pure::event_sink<void> in_send_tick;
	in_port_t in_event_port;
	out_port_t out_event_port;

	size_t intern_buffer;
	size_t extern_buffer;
	size_t middle_buffer;
	bool read;
};

/// Implementation of buffer_interface, which directly forwards state.
template<class data_t>
class state_no_buffer final : public buffer_interface<data_t, state_tag>
{
public:
	state_no_buffer()
	: 	out_port( [this]() { return in_port.get();})
	{
	}

	pure::state_sink<data_t>& in() override
	{
		return in_port;
	}
	pure::state_source<data_t>& out() override
	{
		return out_port;
	}

private:
	pure::state_sink<data_t> in_port;
	pure::state_source<data_t> out_port;
};

/** \brief buffer for states using double buffering
 *
 * \tparam data_t type of state stored in buffer. needs to be copy_constructable.
 */
template<class data_t>
class state_buffer final : public buffer_interface<data_t, state_tag>
{
public:
	state_buffer();

	/// event in port of type void, switches incoming buffers
	auto& switch_active_tick() { return switch_active_tick_; }
	/// event in port of type void, switches outgoing buffers
	auto& switch_passive_tick() { return switch_passive_tick_; }
	/// event in port of type void, directly switches active- and passive-side buffers
	auto& switch_active_passive_tick() { return switch_active_passive_tick_; }
	/// event in port of type void, pulls data at in_port
	auto& work_tick() { return in_work_tick; }

	pure::state_sink<data_t>& in() override
	{
		return in_port;
	}
	pure::state_source<data_t>& out() override
	{
		return out_port;
	}

private:
	void switch_passive_buffers()
	{
		middle_buffer = intern_buffer;
	}

	void switch_active_buffers()
	{
		extern_buffer = middle_buffer;
	}

	void switch_active_passive_buffers()
	{
		extern_buffer = intern_buffer;
	}

	pure::event_sink<void> switch_active_tick_;
	pure::event_sink<void> switch_passive_tick_;
	pure::event_sink<void> switch_active_passive_tick_;
	pure::event_sink<void> in_work_tick;
	pure::state_sink<data_t> in_port;
	pure::state_source<data_t> out_port;

	data_t intern_buffer;
	data_t extern_buffer;
	data_t middle_buffer;
};


template<class data_t, class tag>
struct no_buffer {};

template<class data_t>
struct no_buffer<data_t, event_tag>
{
	typedef event_no_buffer<data_t> type;
};

template<class data_t>
struct no_buffer<data_t, state_tag>
{
	typedef state_no_buffer<data_t> type;
};

template<class data_t, class tag>
struct buffer {};

template<class data_t>
struct buffer<data_t, event_tag>
{
	typedef event_buffer<data_t> type;
};

template<class data_t>
struct buffer<data_t, state_tag>
{
	typedef state_buffer<data_t> type;
};


} // namespace fc

/***************************** Implementation ********************************/
template<class T>
inline fc::state_buffer<T>::state_buffer() :
		switch_active_tick_([this] { switch_active_buffers(); }),
		switch_passive_tick_([this] { switch_passive_buffers(); }),
		switch_active_passive_tick_([this] { switch_active_passive_buffers(); }),
		in_work_tick([this]() { intern_buffer = in_port.get(); }),
		in_port(),
		out_port([this](){ return extern_buffer; }),
		intern_buffer(), //todo, forces T to be default constructible, we should lift that restriction.
		extern_buffer(),
		middle_buffer()
{
}

#endif /* SRC_PORTS_CONNECTION_BUFFER_HPP_ */
