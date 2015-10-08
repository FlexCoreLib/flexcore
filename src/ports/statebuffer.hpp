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

template<class T>
class state_buffer
{
public:
	state_buffer();

	event_in_port<void> in_switch_tick;
	event_in_port<void> in_work_tick;

	state_sink<T> in_port;
	state_source_call_function<T> out_port;

protected:
	void switch_buffers()
	{
		using std::swap;
		if (!already_switched)
			swap(intern_buffer, extern_buffer);
		already_switched = true;
	}

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
