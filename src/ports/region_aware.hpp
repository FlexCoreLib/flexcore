#ifndef SRC_PORTS_REGION_AWARE_HPP_
#define SRC_PORTS_REGION_AWARE_HPP_

#include <ports/port_traits.hpp>
#include <ports/event_buffer.hpp>
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

template<class port_t>
region_aware<port_t> make_region_aware(const port_t& port, std::shared_ptr<region_info> region)
{
	return region_aware<port_t>(region, port);
}

template<class source_t, class sink_t>
bool same_region(const source_t& source, const sink_t& sink)
{
	return source.parent_region_info->get_id()
	        == sink.parent_region_info->get_id();
}
/**
 * \brief factory method to construct buffer
 * \returns either event_buffer or no_buffer.
 */
template<class source_t, class sink_t>
auto construct_buffer(const source_t& source, const sink_t& sink) ->
		std::shared_ptr<buffer_interface<typename result_of<source_t>::type>>
{
	typedef typename result_of<source_t>::type event_t;
	if (!same_region(source, sink))
	{
		auto result_buffer = std::make_shared<event_buffer<event_t>>();

		source.parent_region_info->switch_tick() >>
				result_buffer->switch_tick();

		sink.parent_region_info->work_tick() >>
				result_buffer->send_tick();

		return result_buffer;
	}
	else
		return std::make_shared<no_buffer<event_t>>();
}

template<class base_connection>
struct node_aware_connection: public base_connection
{
	typedef typename base_connection::payload_t event_t;

	node_aware_connection(std::shared_ptr<buffer_interface<event_t>> new_buffer,
	        const base_connection& base) :
			base_connection(base),
			buffer(new_buffer)
	{
		assert(buffer);
	}

	void operator()(event_t event)
	{
		buffer->in_events()(event);
	}

	bool already_buffered = false;
	std::shared_ptr<buffer_interface<event_t>> buffer;
private:

};


// TODO prefer to test this algorithmically
template<class T> struct is_port<region_aware<T>> : public std::true_type
{
};

namespace detail
{

template<class source_t, class sink_t, class buffer_t>
auto make_node_aware_connection(
		std::shared_ptr<buffer_t> buffer,
		const source_t& /*source*/, //only needed for type deduction
		const sink_t& sink
		)
{
	static_assert(is_active_connectable<source_t>::value, "");
	static_assert(is_passive_sink<sink_t>::value, "");
	typedef port_connection<typename source_t::base_t,
			typename sink_t::base_t> base_connection_t;

	buffer->out_events() >> sink;

	return node_aware_connection<base_connection_t>(buffer, base_connection_t());
}

}  //namespace detail

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, region_aware<sink_t> sink)
{
	//construct node_aware_connection
	//based on if source and sink are from same region
	return connect(static_cast<source_t>(source), detail::make_node_aware_connection(
			construct_buffer(source, sink),
			source,
			sink));
}

template<class source_t, class sink_t>
auto connect(region_aware<source_t> source, sink_t sink)
{
	//construct node_aware_connection
	//based on if source and sink are from same region
	return make_region_aware(
			connect(static_cast<source_t>(source), sink),
			source.parent_region_info);
}

}  //namespace fc

#endif /* SRC_PORTS_REGION_AWARE_HPP_ */
