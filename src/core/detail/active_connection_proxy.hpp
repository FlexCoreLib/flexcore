#ifndef SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_
#define SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_

#include <functional>

#include <ports/port_traits.hpp>
#include <core/detail/connection.hpp>

namespace fc
{

namespace detail
{


/**
 * \brief connection of an active_connectable and a connectable
 * fulfills active_connectable
 */
template<class active_t, class passive_t>
struct active_connection_proxy
{
	static_assert(is_active<active_t>::value,
			"active_t needs to be active connectable");

//	typedef typename result_of<active_t>::type payload_t;
//	typedef typename result_of<passive_t>::type result_t;

	active_connection_proxy(active_t active_, passive_t passive) :
			active(active_),
			stored_passive(passive)
	{}

	template<class new_passive_t, class enable = void>
	auto connect(new_passive_t new_active,
			typename std::enable_if< is_passive<new_passive_t>::value>::type* = 0)
	{
		auto tmp = ::fc::connect(stored_passive, new_active);

		return active.connect(tmp);
	}

	template<	class new_passive_t,
				class = typename std::enable_if< not is_passive<new_passive_t>::value >::type
			>
	auto connect(new_passive_t new_active)
	{
		auto connection = ::fc::connect(stored_passive, new_active);
		return active_connection_proxy<active_t, decltype(connection)>(active, connection);
	}

	active_t active;
	passive_t stored_passive;
};

template<class active_t, class passive_t, class Enable = void>
struct active_passive_connect_impl {};

/// Specialization of connect to call member connect of stream_proxy.
template<class active_t, class passive_t>
struct active_passive_connect_impl
	<	active_t,
		passive_t,
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
 * which is not a active_port to a passive_port.
 * \return a stream_proxy which contains the active and the passive.
 */
template<class active_t, class passive_t>
struct active_passive_connect_impl
	<
		active_t,
		passive_t,
		typename std::enable_if
			<
			fc::is_active<active_t>::value
			&& !is_instantiation_of< active_connection_proxy,active_t >::value
			&& !fc::is_passive<passive_t>::value
			>::type
	>
{
	active_connection_proxy<active_t, passive_t> operator()(active_t active, passive_t passive)
	{
		return active_connection_proxy<active_t, passive_t>(active, passive);
	}
};

/**
 * Specialization for the case of connecting a active_port to a passive_port.
 * \pre active_t needs to be a stream_active.
 * \pre passive_t needs to be a stream_passive.
 * \post active is now connected to passive
 * \return nothing, the connection is complete
 */
template<class active_t, class passive_t>
struct active_passive_connect_impl
	<	active_t,
		passive_t,
		typename std::enable_if
			<	fc::is_active<active_t>::value
			and !is_instantiation_of< active_connection_proxy,active_t >::value
			and fc::is_passive<passive_t>::value
			>::type
	>
{
	auto operator()(active_t active, passive_t passive)
	{
		active.connect(passive);
		return;// port_connection<active_t, passive_t>();
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
		return active_passive_connect_impl<source_t, sink_t>()(source, sink);
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
		return active_passive_connect_impl<sink_t, source_t>()(sink, source);
	}
};

}  //namespace detail
}  //namespace fc

#endif /* SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_ */
