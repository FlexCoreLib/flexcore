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
	typedef std::shared_ptr<void_fun> callback_fun_ptr_s;

	auto create_callback_delete_handler()
	{
		if (sinkCallback->get()==0) {
			auto handleIt = std::prev(this->event_handlers->end());
			*sinkCallback =
				std::make_shared<void_fun>(
					void_fun(
						[this, handleIt]()
						{
							std::cout<<"callback called\n";
							this->event_handlers->erase(handleIt);
						}
					)
				);
		}
		return *sinkCallback;
	}

private:

	std::shared_ptr<callback_fun_ptr_s> sinkCallback = std::make_shared<callback_fun_ptr_s>();
};

template<class event_sink_t>
struct event_sink_wrapper: public event_sink_t
{
	static_assert(std::is_class<event_sink_t>::value,
					"can only be mixed into clases, not primitives");
	typedef event_sink_t base_t;
	typedef std::weak_ptr<std::function<void(void)>> callback_fun_ptr_w;

	template<class ... args>
		event_sink_wrapper(const args& ... base_constructor_args) :
				base_t(base_constructor_args...)
	{
		++(*copy_ctr);
	}

	~event_sink_wrapper()
	{
		std::cout<<"sink destructor called\n";
		--(*copy_ctr);
		if (*copy_ctr == 0)
			return;
		std::cout<<"callback still points to something: "<<!destructCallback->expired()<<"\n";
		if(auto sharedCallbackPtr = destructCallback->lock()){
			std::cout<<"calling callback\n";
			(*sharedCallbackPtr)();
		}
	}

	void register_callback(const std::shared_ptr<std::function<void(void)>>& fun){
		std::cout<<"callback registered\n";
		*destructCallback = fun;
		std::cout<<"callback points to something: "<<!destructCallback->expired()<<"\n";
	}

private:

	std::shared_ptr<int> copy_ctr = std::make_shared<int>(0);
	std::shared_ptr<callback_fun_ptr_w> destructCallback = std::make_shared<callback_fun_ptr_w>();
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
