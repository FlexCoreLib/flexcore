#ifndef SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_
#define SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_

#include <functional>

#include <ports/port_traits.hpp>
#include <core/detail/connection.hpp>

namespace fc
{

namespace detail
{

//policy classes for determining argument order in connect calls in the proxy
struct active_sink_first
{
	template<class source_t, class sink_t>
	struct first
	{
		typedef sink_t type;
		typedef typename result_of<sink_t>::type result_t;
	};

	template<class source_t, class sink_t>
	struct second
	{
		typedef source_t type;
		typedef void result_t;
	};

	template<class source_t, class sink_t>
	auto operator()(source_t sink, sink_t source)
	{
		return ::fc::connect(source, sink);
	}
};
struct active_source_first
{
	template<class source_t, class sink_t>
	struct first
	{
		typedef source_t type;
		typedef void result_t;
	};

	template<class source_t, class sink_t>
	struct second
	{
		typedef sink_t type;
		typedef typename result_of<sink_t>::type result_t;
	};
	template<class source_t, class sink_t>
	auto operator()(source_t source, sink_t sink)
	{
		return ::fc::connect(source, sink);
	}
};

/**
 * \brief connection of an active_connectable && a connectable
 * fulfills active_connectable
 */
template<class active_t, class passive_t, class connect_policy>
struct active_connection_proxy
{
	static_assert(is_active<active_t>::value,
			"active_t needs to be active connectable");

	typedef typename
			connect_policy::template first<active_t, passive_t>::result_t payload_t;
	typedef typename
			connect_policy::template second<active_t, passive_t>::result_t result_t;

	active_connection_proxy(active_t active_, passive_t passive) :
			active(active_),
			stored_passive(passive)
	{}

	template<class new_passive_t, class enable = void>
	auto connect(new_passive_t new_passive,
			typename std::enable_if< is_passive<new_passive_t>::value>::type* = 0)
	{
		auto tmp = connect_policy()(stored_passive, new_passive); //todo order of parameters

		return active.connect(tmp);
	}

	template<	class new_passive_t,
				class = typename std::enable_if< not is_passive<new_passive_t>::value >::type
			>
	auto connect(new_passive_t new_passive)
	{
		auto connection = connect_policy()(stored_passive, new_passive); //todo order of parameters
		return active_connection_proxy<
				active_t,
				decltype(connection),
				connect_policy>(
						active, connection);
	}

	active_t active;
	passive_t stored_passive;
};

template<class active_t, class passive_t, class argument_order, class Enable = void>
struct active_passive_connect_impl {};

/// Specialization of connect to call member connect of stream_proxy.
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl
	<	active_t,
		passive_t,
		argument_order,
        typename std::enable_if
			<	is_instantiation_of< active_connection_proxy,
									 active_t >::value
			>::type
	>
{
	auto operator()(active_t active, passive_t passive)
	{
		return active.connect(passive);
	}
};

/**
 * Specialization of active_passive_connect_impl for the case of connecting a st&&ard connectable
 * which is not a active_port to a passive_port.
 * \return a stream_proxy which contains the active && the passive.
 */
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl
	<
		active_t,
		passive_t,
		argument_order,
		typename std::enable_if
			<
			!is_instantiation_of< active_connection_proxy,active_t >::value
			&& ((fc::is_active_source<active_t>::value
					&& !fc::is_passive_sink<passive_t>::value)
			||(fc::is_active_sink<active_t>::value
					&& !fc::is_passive_source<passive_t>::value))>::type
	>
{
	auto operator()(active_t active, passive_t passive)
	{
		return active_connection_proxy<active_t, passive_t, argument_order>
				(active, passive);
	}
};

/**
 * Specialization for the case of connecting a active_port to a passive_port.
 * \pre active_t needs to be a stream_active.
 * \pre passive_t needs to be a stream_passive.
 * \post active is now connected to passive
 * \return nothing, the connection is complete
 */
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl
	<	active_t,
		passive_t,
		argument_order,
		typename std::enable_if
			<	!is_instantiation_of< active_connection_proxy,active_t >::value
			&& ((fc::is_active_source<active_t>::value
					&& fc::is_passive_sink<passive_t>::value)
			||(fc::is_active_sink<active_t>::value
					&& fc::is_passive_source<passive_t>::value))>::type
	>
{
	auto operator()(active_t active, passive_t passive)
	{
		active.connect(passive);
		typedef typename
				argument_order::template first<active_t, passive_t>
				::type source_t;
		typedef typename
				argument_order::template second<active_t, passive_t>
				::type sink_t;
		return port_connection<source_t, sink_t>();
	}
};

template<class source_t, class sink_t>
struct connect_impl
	<	source_t,
		sink_t,
		typename std::enable_if
			<fc::is_active<source_t>::value	>::type
	>
{
	auto operator()(source_t source, sink_t sink)
	{
		// source is active, thus it is the first parameter
		return active_passive_connect_impl<source_t, sink_t, active_source_first>()(source, sink);
	}
};

template<class source_t, class sink_t>
struct connect_impl
	<	source_t,
		sink_t,
		typename std::enable_if
		<fc::is_active<sink_t>::value>::type

	>
{
	auto operator()(source_t source, sink_t sink)
	{
		// sink is active, thus it is the first parameter
		return active_passive_connect_impl<sink_t, source_t, active_sink_first>()(sink, source);
	}
};

}  //namespace detail
}  //namespace fc

#endif /* SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_ */
