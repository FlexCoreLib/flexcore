#ifndef SRC_PORTS_REGION_AWARE_HPP_
#define SRC_PORTS_REGION_AWARE_HPP_

#include <ports/port_traits.hpp>
#include <ports/event_buffer.hpp>
#include <ports/statebuffer.hpp>

#include <core/connection.hpp>
#include <threading/parallelregion.hpp>

namespace fc
{

/**
 * \brief A mixin for ports that are aware of the region, they belong to.
 */
template<class port_t>
struct region_aware: public port_t
{
	//allows explicit access to base of this mixin.
	typedef port_t base_t;

	template<class ... args>
	region_aware(std::shared_ptr<region_info> region,
	        const args& ... base_constructor_args) :
			port_t(base_constructor_args...),
			parent_region_info(region)
	{
		assert(region);
	}

	std::shared_ptr<region_info> parent_region_info;
};

///factory method to get region_aware, mainly there to use template deduction from parameter.
template<class port_t>
region_aware<port_t> make_region_aware(const port_t& port, std::shared_ptr<region_info> region)
{
	return region_aware<port_t>(region, port);
}

///checks if two region_aware connectables are from the same region
template<class source_t, class sink_t>
bool same_region(const region_aware<source_t>& source,
		const region_aware<sink_t>& sink)
{
	return source.parent_region_info->get_id()
	        == sink.parent_region_info->get_id();
}
/**
 * \brief factory method to construct buffer
 * \returns either event_buffer or no_buffer.
 */
template<class source_t, class sink_t>
auto construct_event_buffer(const source_t& source, const sink_t& sink) ->
		std::shared_ptr<buffer_interface<typename result_of<source_t>::type>>
{
	typedef typename result_of<source_t>::type payload_t;
	if (!same_region(source, sink))
	{
		auto result_buffer = std::make_shared<event_buffer<payload_t>>();

		source.parent_region_info->switch_tick() >>
				result_buffer->switch_tick();

		sink.parent_region_info->work_tick() >>
				result_buffer->send_tick();

		return result_buffer;
	}
	else
		return std::make_shared<no_buffer<payload_t>>();
}

//todo remove this really nasty code duplication
template<class source_t, class sink_t>
auto construct_state_buffer(const source_t& source, const sink_t& sink) ->
		std::shared_ptr<state_buffer_interface<typename result_of<source_t>::type>>
{
	typedef typename result_of<source_t>::type payload_t;
	if (!same_region(source, sink))
	{
		auto result_buffer = std::make_shared<state_buffer<payload_t>>();

		sink.parent_region_info->switch_tick() >>
				result_buffer->switch_tick();

		source.parent_region_info->work_tick() >>
				result_buffer->send_tick();

		return result_buffer;
	}
	else
		return std::make_shared<state_no_buffer<payload_t>>();
}

/// mixin for connection that is aware of regions the connectables are from
template<class base_connection>
struct region_aware_event_connection: public base_connection
{
	typedef typename base_connection::payload_t payload_t;

	region_aware_event_connection(std::shared_ptr<buffer_interface<payload_t>> new_buffer,
	        const base_connection& base) :
			base_connection(base),
			buffer(new_buffer)
	{
		assert(buffer);
	}

	void operator()(payload_t event)
	{
		buffer->in()(event);
	}

private:
	std::shared_ptr<buffer_interface<payload_t>> buffer;
};

//also remove this code duplication
template<class base_connection>
struct region_aware_state_connection: public base_connection
{
	typedef typename base_connection::payload_t payload_t;

	region_aware_state_connection(std::shared_ptr<state_buffer_interface<payload_t>> new_buffer,
	        const base_connection& base) :
			base_connection(base),
			buffer(new_buffer)
	{
		assert(buffer);
	}

	payload_t operator()(void)
	{
		return buffer->out()();
	}

private:
	std::shared_ptr<state_buffer_interface<payload_t>> buffer;
};

// TODO prefer to test this algorithmically
template<class T> struct is_port<region_aware<T>> : public std::true_type {};
template<class T> struct is_active_sink<region_aware<T>> : public is_active_sink<T> {};
//template<class T> struct is_active_source<region_aware<T>> : public is_active_source<T> {};
template<class T> struct is_passive_sink<region_aware<T>> : public is_passive_sink<T> {};
template<class T> struct is_passive_source<region_aware<T>> : public is_passive_source<T> {};

namespace detail
{

template<class source_t, class sink_t, class buffer_t>
auto make_node_aware_connection(
		std::shared_ptr<buffer_interface<buffer_t>> buffer,
		const source_t& /*source*/, //only needed for type deduction
		const sink_t& sink
		)
{
	typedef port_connection<typename source_t::base_t,
			typename sink_t::base_t> base_connection_t;

	::fc::connect<decltype(buffer->out()), typename sink_t::base_t>(
			buffer->out(), sink);

	return region_aware_event_connection<base_connection_t>(buffer, base_connection_t());
}

template<class source_t, class sink_t, class buffer_t>
auto make_node_aware_connection(
		std::shared_ptr<state_buffer_interface<buffer_t>> buffer,
		const source_t& source,
		const sink_t& /*sink*/ //only needed for type deduction
		)
{
	typedef port_connection<typename source_t::base_t,
			typename sink_t::base_t> base_connection_t;

	::fc::connect<typename source_t::base_t, decltype(buffer->in())>(
			source, buffer->in());

	return region_aware_state_connection<base_connection_t>(buffer, base_connection_t());
}

template<class source_t, class sink_t, class Enable = void>
struct region_aware_connect_impl
{

};

template<class source_t, class sink_t>
struct region_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	::fc::is_instantiation_of<region_aware, source_t>::value &&
        	::fc::is_active_source<source_t>::value &&
			::fc::is_instantiation_of<region_aware, sink_t>::value &&
			::fc::is_passive_sink<sink_t>::value
			>::type
	>
{

	auto operator()(source_t source, sink_t sink)
	{
		return connect(static_cast<source_t>(source), detail::make_node_aware_connection(
				construct_event_buffer(source, sink),
				source,
				sink));
	}
};

template<class source_t, class sink_t>
struct region_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	::fc::is_instantiation_of<region_aware, source_t>::value &&
         	::fc::is_active_source<source_t>::value &&
			!::fc::is_instantiation_of<region_aware, sink_t>::value
			>::type
	>
{

	auto operator()(source_t source, sink_t sink)
	{
		//construct region_aware_connection
		//based on if source and sink are from same region
		return make_region_aware(
				connect(static_cast<typename source_t::base_t>(source), sink),
				source.parent_region_info);
	}
};


template<class source_t, class sink_t>
struct region_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
    	::fc::is_instantiation_of<region_aware, source_t>::value &&
		::fc::is_instantiation_of<region_aware, sink_t>::value &&
    	::fc::is_active_sink<sink_t>::value
			>::type
	>
{

	auto operator()(source_t source, sink_t sink)
	{
		return connect(detail::make_node_aware_connection(
				construct_state_buffer(source, sink),
				source,
				sink),
				static_cast<typename sink_t::base_t>(sink));
	}
};

template<class source_t, class sink_t>
struct region_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
		::fc::is_instantiation_of<region_aware, sink_t>::value &&
		!::fc::is_instantiation_of<region_aware, sink_t>::value &&
    	::fc::is_active_sink<source_t>::value
			>::type
	>
{

	auto operator()(source_t source, sink_t sink)
	{
		//construct region_aware_connection
		//based on if source and sink are from same region
		return make_region_aware(
				connect(static_cast<typename source_t::base_t>(source), sink),
				source.parent_region_info);
	}
};


} //namesapce detail

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, region_aware<sink_t> sink)
{
	//construct region_aware_connection
	//based on if source and sink are from same region
	return detail::region_aware_connect_impl<
			region_aware<source_t>,
			region_aware<sink_t>>()(source, sink);
}

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, sink_t sink)
{
	//construct region_aware_connection
	//based on if source and sink are from same region
	return detail::region_aware_connect_impl<
			region_aware<source_t>,
			sink_t>()(source, sink);
}

template<class source_t, class sink_t>
auto connect(source_t source, region_aware<sink_t> sink)
{
	//construct region_aware_connection
	//based on if source and sink are from same region
	return detail::region_aware_connect_impl<
			source_t,
			region_aware<sink_t>>()(source, sink);
}

}  //namespace fc

#endif /* SRC_PORTS_REGION_AWARE_HPP_ */
