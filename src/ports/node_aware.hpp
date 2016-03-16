#ifndef SRC_PORTS_NODE_AWARE_HPP_
#define SRC_PORTS_NODE_AWARE_HPP_

#include <ports/detail/port_traits.hpp>
#include <ports/connection_util.hpp>
#include <core/connection.hpp>
#include <threading/parallelregion.hpp>
#include "connection_buffer.hpp"
#include <nodes/base_node.hpp>

namespace fc
{
template <class base>
struct node_aware;

/**
 * checks if two node_aware connectables are from the same region
 */
template<class source_t, class sink_t>
bool same_region(const node_aware<source_t>& source,
        const node_aware<sink_t>& sink)
{
	return source.node->region()->get_id() == sink.node->region()->get_id();
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
	static auto construct_buffer(const active_t& active,
	        const passive_t& passive,
	        tag) ->
	        std::shared_ptr<buffer_interface<result_t, tag>>
	{
		if (!same_region(active, passive))
		{
			auto result_buffer =
					std::make_shared<typename buffer<result_t, tag>::type>();

			active.node->region()->switch_tick() >> result_buffer->switch_active_tick();
			passive.node->region()->switch_tick() >> result_buffer->switch_passive_tick();
			passive.node->region()->work_tick() >> result_buffer->work_tick();

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
struct buffered_event_connection: base_connection
{
	typedef typename base_connection::result_t result_t;

	buffered_event_connection(std::shared_ptr<
	        buffer_interface<result_t, event_tag>> new_buffer,
	        const base_connection& base) :
			base_connection(base), buffer(new_buffer)
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

/**
 * \see buffered_event_connection
 * remove this code duplication if possible
 */
template<class base_connection>
struct buffered_state_connection: base_connection
{
	typedef typename base_connection::result_t result_t;

	buffered_state_connection(std::shared_ptr<
	        buffer_interface<result_t, state_tag>> new_buffer,
	        const base_connection& base) :
			base_connection(base), buffer(new_buffer)
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

template<class T> struct is_active_sink<node_aware<T>> : is_active_sink<T> {};
template<class T> struct is_active_source<node_aware<T>> : is_active_source<T> {};
template<class T> struct is_passive_sink<node_aware<T>> : is_passive_sink<T> {};
template<class T> struct is_passive_source<node_aware<T>> : is_passive_source<T> {};

template<class source_t, class sink_t>
struct result_of<node_aware<connection<source_t, sink_t>>>
{
	using type = result_of_t<source_t>;
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
auto make_buffered_connection(std::shared_ptr<
        buffer_interface<buffer_t, event_tag>> buffer,
        const source_t& /*source*/,  //only needed for type deduction
        sink_t&& sink)
{
	assert(buffer);
	typedef port_connection<
			typename source_t::base_t,
			sink_t,
			buffer_t
			> base_connection_t;

	connect(buffer->out(), std::forward<sink_t>(sink));

	return buffered_event_connection<base_connection_t>(std::move(buffer),
	        base_connection_t());
}

/**
 * \brief creates buffered_connection for states
 * \param buffer the buffer used for the connection
 * \pre buffer != null_ptr
 * The buffer is owned by the active part of the connection
 * This is the sink, since state_sinks are active
 */
template<class source_t, class sink_t, class buffer_t>
auto make_buffered_connection(std::shared_ptr<
        buffer_interface<buffer_t, state_tag>> buffer,
        source_t&& source,
        const sink_t&) /*sink*/  //only needed for type deduction
{
	assert(buffer);
	typedef port_connection<source_t, typename sink_t::base_t, buffer_t> base_connection_t;

	connect(std::forward<source_t>(source), buffer->in());

	return buffered_state_connection<base_connection_t>(std::move(buffer),
	        base_connection_t());
}
}  // namespace detail

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
template <class base>
struct node_aware: base
{
	static_assert(std::is_class<base>{},
			"can only be mixed into clases, not primitives");
	//allows explicit access to base of this mixin.
	typedef base base_t;

	template <class ... args>
	node_aware(tree_base_node* node_ptr, args&&... base_constructor_args)
		: base(std::forward<args>(base_constructor_args)...), node(node_ptr)
	{
		assert(node);
	}

	template<class conn_t,
			class base_t = base,
			class enable = std::enable_if_t<is_active<base_t>{}>>
	auto connect(conn_t&& conn)
	{
		return connect_impl(std::forward<conn_t>(conn),
		        std::integral_constant<bool, has_node_aware<conn_t>()> { });
	}

	tree_base_node* node;

private:
	// helper aliases to make method prototypes easier to read.
	using connection_has_node_aware = std::true_type;
	using connection_doesnt_have_node_aware = std::false_type;
	using base_is_source = std::true_type;
	using base_is_sink = std::false_type;

	template <class conn_t>
	auto connect_impl(conn_t&& conn, connection_has_node_aware)
	{
		return base::connect(
		    introduce_buffer(std::forward<conn_t>(conn), is_active_source<base>{}));
	}

	template <class conn_t>
	auto connect_impl(conn_t&& conn, connection_doesnt_have_node_aware)
	{
		return base::connect(std::forward<conn_t>(conn));
	}

	template <class conn_t>
	auto introduce_buffer(conn_t&& conn, base_is_source)
	{
		using result_t = result_of_t<base_t>;
		const auto& sink = get_sink(conn);
		return detail::make_buffered_connection(
				buffer_factory<result_t>::construct_buffer(
						*this,  // event source is active, thus first
						sink,  // event sink is passive thus second
						event_tag()), *this, std::forward<conn_t>(conn));
	}

	template <class conn_t>
	auto introduce_buffer(conn_t&& conn, base_is_sink)
	{
		using result_t = result_of_t<conn_t>;
		const auto& source = get_source(conn);
		return detail::make_buffered_connection(
				buffer_factory<result_t>::construct_buffer(
						*this,  // state sink is active thus first
						source,  // state source is passive thus second
						state_tag()), std::forward<conn_t>(conn), *this);
	}

	template <class conn_t>
	static constexpr bool has_node_aware()
	{
		if (is_active_source<base> { })
			return detail::is_derived_from<fc::node_aware,
			                               decltype(get_sink(std::declval<conn_t&>()))>::value;
		else
			return detail::is_derived_from<fc::node_aware,
			                               decltype(get_source(std::declval<conn_t&>()))>::value;
	}
};

}  //namespace fc

#endif /* SRC_PORTS_NODE_AWARE_HPP_ */
