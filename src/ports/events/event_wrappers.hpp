/*
 * event_wrappers.hpp
 *
 *  Created on: Nov 10, 2015
 *      Author: jschwan
 */

#ifndef SRC_PORTS_EVENTS_EVENT_WRAPPERS_HPP_
#define SRC_PORTS_EVENTS_EVENT_WRAPPERS_HPP_

#include <functional>
#include <memory>
#include <iterator>
#include <list>

#include <core/connection.hpp>


namespace fc
{


template<class event_source_t>
struct event_source_wrapper: public event_source_t
{
	static_assert(std::is_class<event_source_t>::value,
				"can only be mixed into clases, not primitives");
	typedef event_source_t base_t;
	typedef std::function<void(void)> void_fun;
	typedef std::shared_ptr<void_fun> callback_fun_ptr_strong;

	template<class ... args>
	event_source_wrapper(const args& ... base_constructor_args) :
		base_t(base_constructor_args...)
	{
	}

	auto create_callback_delete_handler()
	{
		//Assumes event_wrappers::connect is not called simultaneously; not thread-safe
		//Assumes that event_handlers does not invalidate its iterators (i.e. type is std::list)
		auto handleIt = std::prev(this->event_handlers->end());

		sink_callback->push_back(std::make_shared<void_fun>());
		auto callbackIt = std::prev(this->sink_callback->end());
		sink_callback->back()=std::make_shared<void_fun>(
				[this, handleIt, callbackIt]()
				{
					this->event_handlers->erase(handleIt);
					this->sink_callback->erase(callbackIt);
				}
			);
		return sink_callback->back();
	}

private:

	std::shared_ptr<std::list<callback_fun_ptr_strong>> sink_callback = std::make_shared<std::list<callback_fun_ptr_strong>>();
};

template<class event_sink_t>
struct event_sink_wrapper: public event_sink_t
{
	static_assert(std::is_class<event_sink_t>::value,
					"can only be mixed into clases, not primitives");
	typedef event_sink_t base_t;
	typedef std::weak_ptr<std::function<void(void)>> callback_fun_ptr_weak;

	template<class ... args>
	event_sink_wrapper(const args& ... base_constructor_args) :
		base_t(base_constructor_args...)
	{
		++(*copy_ctr);
	}

	~event_sink_wrapper()
	{
		--(*copy_ctr);
		if (*copy_ctr == 0)
			return;
		deregister();
	}

	void register_callback(const std::shared_ptr<std::function<void(void)>>& fun)
	{
		*destruct_callback = fun;
	}

	void deregister()
	{
		if(auto sharedCallbackPtr = destruct_callback->lock())
		{
			(*sharedCallbackPtr)();
		}
	}

private:

	std::shared_ptr<int> copy_ctr = std::make_shared<int>(0);
	std::shared_ptr<callback_fun_ptr_weak> destruct_callback = std::make_shared<callback_fun_ptr_weak>();
};

template<class event_source_t, class event_sink_t>
auto connect(event_source_wrapper<event_source_t> source, event_sink_wrapper<event_sink_t> sink)
{
	auto connect_result = connect(static_cast<event_source_t>(source), static_cast<event_sink_t>(sink));
	sink.register_callback(source.create_callback_delete_handler());
	return connect_result;
}

} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_WRAPPERS_HPP_ */
