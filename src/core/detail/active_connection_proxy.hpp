#ifndef SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_
#define SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_

#include <functional>

#include <core/detail/connection.hpp>
#include <core/ports.hpp>

namespace fc
{

namespace detail
{

template<class T>
struct is_active_source:
		std::integral_constant<bool,
		is_active_connectable<T>::value &&
		is_port<T>::value>

{
};

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
 * \brief contains connections of active_connectables during creation.
 *
 * active_connection_proxy is used to store the intermediate objects created,
 * when a connection is build up from an active connectable
 * Until the active connectable is connected to a proper passive,
 * the connection is not complete and no value can be pulled or send through.
 * Nonetheless, the connection needs to be stored to allow further connections.
 * the active_connection_proxy stores these temporary objects.
 *
 * \tparam active_ is the active connection,
 * \tparam passive_t the passive side of the connection durint buildup
 * \tparam
 * {
 * connect_policy a policy class which makes sure parameters to connect
 * inside the proxy are in the correct order.
 * }
 * */
template<class active_t, class passive_t, class connect_policy>
struct active_connection_proxy
{
	static_assert(is_active<active_t>::value,
			"active_t in proxy needs to be active connectable");

	typedef typename
			connect_policy::template first<active_t, passive_t>::result_t payload_t;
	typedef typename
			connect_policy::template second<active_t, passive_t>::result_t result_t;

	active_connection_proxy(active_t active_, passive_t passive) :
			active(active_),
			stored_passive(passive)
	{}

	/**
	 * \brief connects a passive connectable to the active_connection_proxy.
	 *
	 * connects the new_source to the current stored source
	 * then connects this connection to the sink which completes the connection
	 *
	 * \pre new_passive_t needs to be passive connectable.
	 * \post new_passive is now connected to active via the connection stored in the proxy.
	 * \returns port connection tag object.
	 */
	template<class new_passive_t, class enable = void>
	auto connect(new_passive_t new_passive,
			typename std::enable_if< is_passive<new_passive_t>::value>::type* = 0)
	{
		static_assert(is_passive<new_passive_t>::value,
				"new_passive_t in proxy needs to be passive connectable");

		auto tmp = connect_policy()(stored_passive, new_passive);

		return active.connect(tmp);
	}

	/**
	 * \brief connects a connectable, which is not a passive connectable to the active_connection_proxy.
	 *
	 * connects the new_connectable to the current stored source
	 *
	 * \pre new_connectable_t needs to be connectable
	 * \post new_connectable_t is connected to the old passive.
	 * \returns a active_connection_proxy, which contains the new_connectable in its connection.
	 */
	template<	class new_connectable_t,
				class = typename std::enable_if< not is_passive<new_connectable_t>::value >::type
			>
	auto connect(new_connectable_t new_connectable)
	{
		auto connection = connect_policy()(stored_passive, new_connectable);
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
 * Specialization of active_passive_connect_impl for the case of connecting a standard connectable
 * which is not a is_passive_sink to a is_passive_source.
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
			&& ((is_active_source<active_t>::value
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
 * \pre active_t needs to be a active_connectable.
 * \pre passive_t needs to be a passive_connectable.
 * \post active is now connected to passive
 * \return port_connection tag object with the type information of the connection.
 */
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl
	<	active_t,
		passive_t,
		argument_order,
		typename std::enable_if
			<	!is_instantiation_of< active_connection_proxy,active_t >::value
			&& ((is_active_source<active_t>::value
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
