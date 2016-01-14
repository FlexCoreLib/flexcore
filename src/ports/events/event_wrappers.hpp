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

template<class T>
struct event_sink_wrapper;

/**
 * \brief A mixin for sources that provide callbacks to deregister sinks
 * \tparam event_source_t is type of event source that event_source_wrapper is mixed into.
 *
 * example:
 * \code{cpp}
 * typedef event_source_wrapper<event_out_port<int>> observer_event_source;
 * \endcode
 */
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

	/**
	 * \brief Creates a callback to deregister event sinks
	 * \returns std::shared_ptr<std::function<void(void)>>, pointer to a callback method
	 * \pre sink_callback != null_ptr
	 * \pre event_source_t::event_handlers != null_ptr
	 * \pre *event_source_t::event_handlers is container that does not invalidate its iterators (e.g. std::list)
	 * \pre Method is not called in parallel i.e. sinks are connected in serial fashion
	 * \post sink_callback->empty() == false
	 */
	auto create_callback_delete_handler()
	{
		assert(this->event_handlers);
		assert(sink_callback);

		//Assumes event_wrappers::connect is not called simultaneously -> not thread safe!
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

		assert(!sink_callback->empty());
		return sink_callback->back();
	}

	template<class event_sink_t>
	auto connect(event_sink_wrapper<event_sink_t> sink)
	{
		auto connect_result = event_source_t::connect(
				static_cast<event_sink_t>(sink));
		sink.register_callback(this->create_callback_delete_handler());
		return connect_result;
	}

	template<class sink_t>
	auto connect(const sink_t& sink)
	{
		return event_source_t::connect(sink);
	}

private:

	std::shared_ptr<std::list<callback_fun_ptr_strong>> sink_callback = std::make_shared<std::list<callback_fun_ptr_strong>>();
};

/**
 * \brief A mixin for sinks that implement observable pattern to deregister from sources
 * \tparam event_sink_t is type of event sink that event_sink_wrapper is mixed into.
 *
 * example:
 * \code{cpp}
 * typedef event_sink_wrapper<event_in_port<int>> observable_event_sink;
 * \endcode
 */
template<class event_sink_t>
struct event_sink_wrapper: public event_sink_t
{
	static_assert(std::is_class<event_sink_t>::value,
					"can only be mixed into clases, not primitives");
	typedef event_sink_t base_t;
	typedef std::weak_ptr<std::function<void(void)>> callback_fun_ptr_weak;

	template<class ... args>
	event_sink_wrapper(const args& ... base_constructor_args) :
		base_t(base_constructor_args...),
		dereigsterer(std::make_shared<raii_deregister>(
				[this](){deregister();}))
	{
	}

	/**
	 * \brief Registers the source to the sink
	 * \pre fun != null_ptr
	 * \post destruct_callback->expired() == false
	 */
	void register_callback(const std::shared_ptr<std::function<void(void)>>& fun)
	{
		assert(fun);
		*destruct_callback = fun;
		assert(!destruct_callback->expired());
	}

private:
	/**
	 * \brief raii wrapper around deregister call
	 *
	 * Makes sure deregister is only called, when the last copy of the port is destroyed
	 */
	struct raii_deregister
	{
		raii_deregister(const std::function<void(void)>& callback)
			: deregister_callback(callback)
		{
		}

		~raii_deregister()
		{
			deregister_callback();
		}

		std::function<void(void)> deregister_callback;
	};

	/**
	 * \brief Deregisters from the source
	 * \pre destruct_callback != null_ptr
	 */
	void deregister()
	{
		assert(destruct_callback);
		if(auto sharedCallbackPtr = destruct_callback->lock())
		{
			(*sharedCallbackPtr)();
		}
	}

	std::shared_ptr<raii_deregister> dereigsterer;
	std::shared_ptr<callback_fun_ptr_weak> destruct_callback = std::make_shared<callback_fun_ptr_weak>();
};

/**
 * \brief overload of connect with event_source_wrapper<> and event_sink_wrapper<>
 * connects base objects normally and registers the callbacks
 * \pre Previous calls to this method have finished; execution is serial
 * \returns normal connection result of event_source_t and event_sink_t
 */
//template<class event_source_t, class event_sink_t>
//auto connect(
//		event_source_wrapper<event_source_t> source,
//		event_sink_wrapper<event_sink_t> sink)
//{
//	auto connect_result = connect(
//			static_cast<event_source_t>(source),
//			static_cast<event_sink_t>(sink));
//	sink.register_callback(source.create_callback_delete_handler());
//	return connect_result;
//}

template<class connection>
struct deletable_connection : public connection
{
	explicit deletable_connection(const connection& con)
		: connection(con)
	{

	}

	void register_callback(
			const std::shared_ptr<std::function<void(void)>>& fun)
	{
		this->sink.register_callback(fun);
	}
};

template<class connection>
deletable_connection<connection> make_deletable_connection(const connection& con)
{
	return deletable_connection<connection>(con);
}

template<class event_source_t, class event_sink_t>
auto connect(
		event_source_t source,
		event_sink_wrapper<event_sink_t> sink)
{
	return make_deletable_connection(
			connect(source, static_cast<event_sink_t>(sink)));
}

// TODO prefer to test this algorithmically
template<class T> struct is_port<event_sink_wrapper<T>> : public std::true_type {};
template<class T> struct is_port<event_source_wrapper<T>> : public std::true_type {};
template<class T> struct is_active_source<event_source_wrapper<T>> : public is_active_source<T> {};
template<class T> struct is_passive_sink<event_sink_wrapper<T>> : public is_passive_sink<T> {};


} // namespace fc

#endif /* SRC_PORTS_EVENTS_EVENT_WRAPPERS_HPP_ */
