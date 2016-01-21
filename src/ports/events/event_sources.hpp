#ifndef SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_
#define SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_

#include <cassert>
#include <functional>
#include <memory>
#include <list>

#include <core/traits.hpp>
#include <core/connection.hpp>

#include <ports/detail/port_traits.hpp>
#include <core/detail/active_connection_proxy.hpp>

#include <ports/events/event_sinks.hpp>

#include <iostream>

namespace fc
{

/**
 * \brief minimal output port for events.
 *
 * fulfills active_source
 * can be connected to multiples sinks and stores these in a shared std::vector.
 *
 * \tparam event_t type of event stored, needs to fulfill copy_constructable.
 * \invariant shared pointer event_handlers != 0.
 */
template<class event_t>
struct event_out_port
{
	typedef typename std::remove_reference<event_t>::type result_t;
	typedef typename detail::handle_type<result_t>::type handler_t;

	typedef std::function<void(void)> void_fun;
	typedef std::shared_ptr<void_fun> callback_fun_ptr_strong;

	event_out_port() :
		sink_callbacks(std::make_shared<std::list<callback_fun_ptr_strong>>())
	{
	};

	event_out_port(const event_out_port&) = default;

	template<class... T>
	void fire(T&&... event)
	{
		static_assert(sizeof...(T) == 0 || sizeof...(T) == 1,
				"we only allow single events, or void events atm");

		static_assert(std::is_void<event_t>() || std::is_constructible<
				typename std::remove_reference<T>::type...,
				typename std::remove_reference<event_t>::type>(),
				"tried to call fire with a type, not implicitly convertible to type of port."
				"If conversion is required, do the cast before calling fire.");

		assert(event_handlers);
		for (auto& target : (*event_handlers))
		{
			assert(target);
			target(static_cast<event_t>(event)...);
		}
	}

	void add_handler(handler_t new_handler)
	{
		event_handlers->push_back(new_handler);
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
		assert(sink_callbacks);

		//Assumes event_wrappers::connect is not called simultaneously -> not thread safe!
		auto handleIt = std::prev(event_handlers->end());

		sink_callbacks->push_back(std::make_shared<void_fun>());
		auto callbackIt = std::prev(sink_callbacks->end());
		sink_callbacks->back()=std::make_shared<void_fun>(
				[this, handleIt, callbackIt]()
				{
					event_handlers->erase(handleIt);
					sink_callbacks->erase(callbackIt);
				}
			);

		assert(!sink_callbacks->empty());

		std::cout<<"callback created, num of callbacks is "<<sink_callbacks->size()<<"\n";
		return sink_callbacks->back();
	}

//	/**
//	 * \brief connects new connectable target to port.
//	 * \param new_handler the new target to be connected.
//	 * \pre new_handler is not empty function
//	 * \post event_handlers.empty() == false
//	 */
//	auto connect(handler_t new_handler)
//	{
//		assert(event_handlers);
//		assert(new_handler); //connecting empty functions is illegal
//		event_handlers->push_back(new_handler);
//
//		assert(!event_handlers->empty());
//		return port_connection<decltype(*this), handler_t, result_t>();
//	}

	template<class source_ptr_t, class sink_t, class Enable = void>
	struct connect_impl {};

	/**
	 * \brief specialization for connection with event_sink
	 * \param sink the new target to be connected.
	 * \pre sink is not empty function
	 * \pre sink of type event_sink
	 * \post event_handlers.empty() == false
	 */
	template <class source_ptr_t, class sink_t>
	struct connect_impl
		<	source_ptr_t,
			sink_t,
			typename std::enable_if
				<
				has_register_function<sink_t>(0)
				>::type
		>
	{
		typedef typename std::remove_pointer<source_ptr_t>::type source_t;

		auto operator()(const source_ptr_t source, sink_t sink)
		{
			assert(source->event_handlers);
//			assert(sink); //connecting empty functions is illegal
			source->add_handler(typename source_t::handler_t(sink));

			sink.register_callback(source->create_callback_delete_handler());

			assert(!source->event_handlers->empty());
			return port_connection<decltype(*this), sink_t, result_t>();
		}
	};

	/**
	 * \brief specialization for connection with event_sink
	 * \param sink the new target to be connected.
	 * \pre sink is not empty function
	 * \pre sink not of type event_sink
	 * \post event_handlers.empty() == false
	 */
	template <class source_ptr_t, class sink_t>
	struct connect_impl
		<	source_ptr_t,
			sink_t,
			typename std::enable_if
				<
				!has_register_function<sink_t>(0)
				>::type
		>
	{
		typedef typename std::remove_pointer<source_ptr_t>::type source_t;

		auto operator()(const source_ptr_t source, const sink_t sink)
		{
			assert(source->event_handlers);
//			assert(sink); //connecting empty functions is illegal
			source->add_handler(typename source_t::handler_t(sink));

			assert(!source->event_handlers->empty());
			return port_connection<decltype(this), sink_t, result_t>();
		}
	};

	template <class sink_t>
	auto connect(sink_t sink)
	{
		return connect_impl<decltype(this), sink_t>()(this, sink);
	}

//	auto connect(handler_t new_handler)
//	{
//		return connect_impl<decltype(this), handler_t>()(this, new_handler);
//	}


//	template<class event_sink_t>
//	auto connect(event_sink_t sink)
//	{
//		auto connect_result = connect(sink);
//		sink.register_callback(create_callback_delete_handler());
//		return connect_result;
//	}

protected:

	// stores event_handlers in shared vector, since the port is stored in a node
	// but will be copied, when it is connected. The node needs to send
	// to all connected event_handlers, when an event is fired.
	typedef std::list<handler_t> handler_vector;
	std::shared_ptr<handler_vector> event_handlers = std::make_shared<handler_vector>(0);

private:

	std::shared_ptr<std::list<callback_fun_ptr_strong>> sink_callbacks;
};

// traits
// TODO prefer to test this algorithmically
template<class T> struct is_active_source<event_out_port<T>> : public std::true_type {};

} // namespace fc

#endif /* SRC_PORTS_EVENT_SOURCES_EVENT_SOURCES_HPP_ */
