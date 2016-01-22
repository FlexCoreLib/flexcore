#ifndef SRC_PORTS_NODE_AWARE_HPP_
#define SRC_PORTS_NODE_AWARE_HPP_

#include <ports/detail/port_traits.hpp>
#include <core/connection.hpp>
#include <threading/parallelregion.hpp>
#include "connection_buffer.hpp"
#include <nodes/base_node.hpp>

#include <graph/graph_interface.hpp>

#include <iostream>

namespace fc
{

/**
 * \brief A mixin for elements that are aware of the node they belong to.
 * Used as mixin for ports and connections.
 * \tparam base is type node_aware is mixed into.
 * \invariant tree_base_node* node is always valid.
 *
 * example:
 * \code{cpp}
 * typedef node_aware<pure::event_in_port<int>> node_aware_event_port;
 * \endcode
 */
template<class base>
struct node_aware: public base
{
	static_assert(std::is_class<base>::value,
			"can only be mixed into clases, not primitives");
	//allows explicit access to base of this mixin.
	typedef base base_t;

	template<class ... args>
	node_aware( tree_base_node* node_ptr,
				const args& ... base_constructor_args )
		: base_t(base_constructor_args ...)
		, node(node_ptr)
	{
		assert(node_ptr);
	}

	auto node_info() const
	{
		assert(node);
		return graph::graph_node_properties(node->own_name());
	}

	auto port_info() const
	{
		return graph::graph_port_information();
	}

	tree_base_node* node;
};


/**
 * checks if two node_aware connectables are from the same region
 */
template<class source_t, class sink_t>
bool same_region( const node_aware<source_t>& source,
				  const node_aware<sink_t>& sink )
{
	return source.node->region().get_id()
		  == sink.node->region().get_id();
}

/**
 * \brief factory method to construct a buffer
 *
 * \returns either buffer or no_buffer.
 */
template<class result_t>
struct buffer_factory
{
	template<class active_t, class passive_t, class tag>
	static auto construct_buffer(const active_t& active, const passive_t& passive, tag) ->
			std::shared_ptr<buffer_interface<result_t, tag>>
	{
		if (!same_region(active, passive))
		{
			auto result_buffer = std::make_shared<typename buffer<result_t, tag>::type>();

			active.node->region().switch_tick() >> result_buffer->switch_tick();
			passive.node->region().work_tick() >> result_buffer->work_tick();

			return result_buffer;
		}
		else
			return std::make_shared<typename no_buffer<result_t, tag>::type>();
	}
};

/**
 * \brief Connection that contain a buffer_interface
 *
 * Connection that contains a buffer_interface (see buffer_factory).
 * This will be a buffer if source and sink are from different regions,
 * a no_buffer otherwise.
 *
 * \tparam base_connection connection type, the buffer is mixed into.
 * \invariant buffer != null_ptr
 */
template<class base_connection>
struct buffered_event_connection : public base_connection
{
	typedef typename base_connection::result_t result_t;

	buffered_event_connection( std::shared_ptr<buffer_interface<result_t, event_tag>> new_buffer, 
							   const base_connection& base ) 
		: base_connection(base)
		, buffer(new_buffer)
	{
		assert(buffer);
	}

	void operator()(const result_t& event)
	{
		assert(buffer);
		buffer->in()(event);
	}

private:
	std::shared_ptr<buffer_interface<result_t, event_tag>> buffer;
};

//todo hackhack
template<class T> struct is_passive_sink<buffered_event_connection<T>> : public std::true_type {};

/**
 * see buffered_event_connection
 * remove this code duplication if possible
 */
template<class base_connection>
struct buffered_state_connection: public base_connection
{
	typedef typename base_connection::result_t result_t;

	buffered_state_connection( std::shared_ptr<buffer_interface<result_t, state_tag>> new_buffer,
							   const base_connection& base ) 
		: base_connection(base)
		, buffer(new_buffer)
	{
		assert(buffer);
	}

	result_t operator()(void)
	{
		return buffer->out()();
	}

private:
	std::shared_ptr<buffer_interface<result_t, state_tag>> buffer;
};

template<class T> struct is_active_sink<node_aware<T>> : public is_active_sink<T> {};
template<class T> struct is_active_source<node_aware<T>> : public is_active_source<T> {};
template<class T> struct is_passive_sink<node_aware<T>> : public is_passive_sink<T> {};
template<class T> struct is_passive_source<node_aware<T>> : public is_passive_source<T> {};

template<class source_t, class sink_t>
struct result_of<node_aware<connection<source_t, sink_t>>>
{
	typedef typename result_of<source_t>::type type;
};

namespace detail
{

/**
 * \brief creates buffered_connection for events
 * \param buffer the buffer used for the connection
 * \pre buffer != null_ptr
 * The buffer is owned by the active part of the connection
 * This is the source, since event_sources are active
 */
template<class source_t, class sink_t, class buffer_t>
auto make_buffered_connection(
		std::shared_ptr<buffer_interface<buffer_t, event_tag>> buffer,
		const source_t& /*source*/, //only needed for type deduction
		const sink_t& sink )
{
	assert(buffer);
	typedef port_connection
		<	typename source_t::base_t,
			typename sink_t::base_t,
			buffer_t
		> base_connection_t;

	connect(buffer->out(), static_cast<typename sink_t::base_t>(sink));

	return buffered_event_connection<base_connection_t>(buffer, base_connection_t());
}

/**
 * \brief creates buffered_connection for states
 * \param buffer the buffer used for the connection
 * \pre buffer != null_ptr
 * The buffer is owned by the active part of the connection
 * This is the sink, since state_sinks are active
 */
template<class source_t, class sink_t, class buffer_t>
auto make_buffered_connection(
		std::shared_ptr<buffer_interface<buffer_t, state_tag>> buffer,
		const source_t& source,
		const sink_t& ) /*sink*/ //only needed for type deduction
{
	assert(buffer);
	typedef port_connection
		<	typename source_t::base_t,
			typename sink_t::base_t,
			buffer_t
		> base_connection_t;

	connect(static_cast<typename source_t::base_t>(source), buffer->in());

	return buffered_state_connection<base_connection_t>(buffer, base_connection_t());
}

/**
 * \brief factory method to wrap node_aware around a connection,
 *
 * Mainly there to use template deduction from parameter base.
 * Thus client code doesn't need to get type connection_base_t by hand.
 */
template<class base_connection_t>
node_aware<base_connection_t> make_node_aware(const base_connection_t& base, tree_base_node* node_ptr)
{
	assert(node_ptr);
	return node_aware<base_connection_t>(node_ptr, base);
}

template<class source_t, class sink_t, class Enable = void>
struct both_node_aware_connect_impl;

template<class source_t, class sink_t, class Enable = void>
struct source_node_aware_connect_impl;

template<class source_t, class sink_t, class Enable = void>
struct sink_node_aware_connect_impl;

template<class source_t, class sink_t>
struct both_node_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	::fc::is_active_source<source_t>::value &&
			::fc::is_passive_sink<sink_t>::value
		>::type
	>
{
	auto operator()(const source_t& source, const sink_t& sink)
	{
		graph::ad_to_graph(source.node_info(), source.port_info(),
				sink.node_info(), sink.port_info());

		typedef typename result_of<source_t>::type result_t;
		return connect(
				static_cast<const typename source_t::base_t&>(source),
				detail::make_buffered_connection(
						buffer_factory<result_t>::construct_buffer(
								source, // event source is active, thus first
								sink, // event sink is passive thus second
								event_tag()
							),
							source,
							sink
					)
			);
	}
};

template<class source_t, class sink_t>
struct both_node_aware_connect_impl
	<	source_t,
		sink_t,
		typename std::enable_if<
			::fc::is_passive_source<source_t>::value &&
			::fc::is_active_sink<sink_t>::value
		>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		graph::ad_to_graph(source.node_info(), source.port_info(),
				sink.node_info(), sink.port_info());

		typedef typename result_of<source_t>::type result_t;
		return connect(detail::make_buffered_connection(
				buffer_factory<result_t>::construct_buffer(
						sink, // state sink is active thus first
						source, // state source is passive thus second
						state_tag()),
				source, sink),
				static_cast<typename sink_t::base_t>(sink)
				);
	}
};

template<class source_t, class sink_t>
struct source_node_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	::fc::is_active_source<source_t>::value
		>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		// construct node_aware_connection
		// based on if source and sink are from same region
		return make_node_aware(
				connect(static_cast<typename source_t::base_t>(source), sink),
				source.node);
	}
};

template<class source_t, class sink_t>
struct sink_node_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	::fc::is_active_sink<sink_t>::value
        >::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		// construct node_aware_connection
		// based on if source and sink are from same region
		return make_node_aware(
				connect(source, static_cast<typename sink_t::base_t>(sink)),
						sink.node);
	}
};

template<class source_t, class sink_t>
struct sink_node_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	!is_active_source<source_t>::value &&
			::fc::is_passive_sink<sink_t>::value
		>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		//construct node_aware_connection
		//based on if source and sink are from same region
		return make_node_aware(
				connect(source, static_cast<typename sink_t::base_t>(sink)),
				sink.node);
	}
};

template<class source_t, class sink_t>
struct source_node_aware_connect_impl
	<	source_t,
		sink_t,
        typename std::enable_if<
        	is_passive_source<source_t>::value &&
			!::fc::is_active_sink<sink_t>::value
		>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		//construct node_aware_connection
		//based on if source and sink are from same region
		return make_node_aware(
				connect(static_cast<typename source_t::base_t>(source), sink),
				source.node);
	}
};

} //namesapce detail

/**
 * \brief overload of connect with node_aware<>
 * possibly adds buffer, then forwards connect call to source_t.
 *
 * \returns buffered connection with connection of source_t and sink_t mixed in.
 */
template<class source_t, class sink_t>
auto connect(node_aware<source_t> source, node_aware<sink_t> sink)
{
	// construct node_aware_connection
	// based on if source and sink are from same region
	return detail::both_node_aware_connect_impl
		<	node_aware<source_t>,
			node_aware<sink_t>
		> ()(source, sink);
}

template<class source_t, class sink_t>
auto connect(node_aware<source_t> source, sink_t sink)
{
	// construct node_aware_connection
	// based on if source and sink are from same region
	return detail::source_node_aware_connect_impl
		<	node_aware<source_t>,
			sink_t
		> ()(source, sink);
}

template<class source_t, class sink_t>
auto connect(source_t source, node_aware<sink_t> sink)
{
	// construct node_aware_connection
	// based on if source and sink are from same region
	return detail::sink_node_aware_connect_impl
		<	source_t,
			node_aware<sink_t>
		> ()(source, sink);
}

}  //namespace fc

#endif /* SRC_PORTS_NODE_AWARE_HPP_ */
