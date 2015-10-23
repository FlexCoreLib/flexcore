/*
 * statebuffer.h
 *
 *  Created on: Oct 2, 2015
 *      Author: ckielwein
 */

#ifndef SRC_PORTS_STATEBUFFER_HPP_
#define SRC_PORTS_STATEBUFFER_HPP_

#include "state_ports.hpp"

namespace fc
{

//todo merge with event_buffer to remove code duplication
template<class data_t>
struct state_buffer_interface
{
	state_buffer_interface() = default;
	virtual ~state_buffer_interface() = default;
	virtual state_sink<data_t> in() = 0;
	virtual state_source_call_function<data_t> out() = 0;

	state_buffer_interface(const state_buffer_interface&) = delete;
	state_buffer_interface& operator= (const state_buffer_interface &) = delete;
};

template<class data_t>
class state_no_buffer : public state_buffer_interface<data_t>
{
public:
	state_no_buffer()
	: 	out_port( [this]() { return in_port.get();})
	{
	}

	state_sink<data_t> in() override
	{
		return in_port;
	}
	state_source_call_function<data_t> out() override
	{
		return out_port;
	}

private:
	state_sink<data_t> in_port;
	state_source_call_function<data_t> out_port;
};

template<class T>
class state_buffer : public state_buffer_interface<T>
{
public:
	state_buffer();

	// event in port of type void, switches buffers
	auto switch_tick() { return in_switch_tick; };
	// event in port of type void, pulls data at in_port
	auto work_tick() { return in_work_tick; };

	state_sink<T> in() override
	{
		return in_port;
	}
	state_source_call_function<T> out() override
	{
		return out_port;
	}


protected:
	void switch_buffers()
	{
		using std::swap;
		if (!already_switched)
			*extern_buffer = *intern_buffer;
		already_switched = true;
	}

	event_in_port<void> in_switch_tick;
	event_in_port<void> in_work_tick;
	state_sink<T> in_port;
	state_source_call_function<T> out_port;
private:
	typedef T buffer_t;
	std::shared_ptr<buffer_t> intern_buffer;
	std::shared_ptr<buffer_t> extern_buffer;
	bool already_switched = false;
};

} /* namespace fc */

/***************************** Implementation ********************************/
template<class T>
inline fc::state_buffer<T>::state_buffer() :
		in_switch_tick( [this](){ switch_buffers(); } ),
		in_work_tick([this]()
				{
					*intern_buffer = in_port.get();
					already_switched = false;
				}),
		in_port(),
		out_port([this](){ return *extern_buffer; }),
		intern_buffer(std::make_shared<T>()), //todo, forces T to be default constructible, we should lift that restriction.
		extern_buffer(std::make_shared<T>())
{
}

#endif /* SRC_PORTS_STATEBUFFER_HPP_ */
