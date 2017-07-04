#ifndef SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_
#define SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_

#include <flexcore/core/connection.hpp>
#include <flexcore/core/traits.hpp>
#include <flexcore/pure/port_connection.hpp>

#include <cassert>

namespace fc
{
namespace detail
{

//policy classes for determining argument order in connect calls in the proxy
struct active_sink_first
{
	template<class sink_t, class source_t>
	using source = source_t;

	template<class sink_t, class source_t>
	using sink = sink_t;

	template<class sink_t, class source_t>
	using result_t = void;

	template<class sink_t, class source_t>
	auto operator()(sink_t&& sink, source_t&& source)
	{
		return ::fc::connect(std::forward<source_t>(source), std::forward<sink_t>(sink));
	}
};
struct active_source_first
{
	template<class source_t, class sink_t>
	using source = source_t;

	template<class source_t, class sink_t>
	using sink = sink_t;

	template<class source_t, class sink_t>
	using result_t = typename detail::result_of_fwd<sink_t, result_of_t<source_t>>::type::type;

	template<class source_t, class sink_t>
	auto operator()(source_t&& source, sink_t&& sink)
	{
		return ::fc::connect(std::forward<source_t>(source), std::forward<sink_t>(sink));
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
 * \tparam active_t is the active connection,
 * \tparam passive_t the passive side of the connection during buildup
 * \tparam connect_policy a policy class which makes sure parameters to connect
 * inside the proxy are in the correct order.
 */
template<class active_t, class passive_t, class connect_policy>
struct active_connection_proxy
{
	using source_t = std::decay_t<typename connect_policy::template source<active_t, passive_t>>;
	using sink_t = std::decay_t<typename connect_policy::template sink<active_t, passive_t>>;

	static_assert(class_is_active_v<active_t>,
			"active_t in proxy needs to be active connectable");
	static_assert(!std::is_same<connect_policy, active_source_first>::value ||
			has_result_of_type<sink_t, result_of_t<source_t>>(),
			"Return type of source and parameter type of sink need to be compatible");

	using result_t = typename
			connect_policy::template result_t<active_t, passive_t>;

	active_connection_proxy(active_t active_, passive_t passive) :
			active(std::forward<active_t>(active_)),
			stored_passive(std::forward<passive_t>(passive)),
			connected{false}
	{}

	active_connection_proxy(const active_connection_proxy&) = default;
	active_connection_proxy(active_connection_proxy&&) = default;

	~active_connection_proxy()
	{
		assert(connected && "active_connection_proxy was destroyed but not connected.");
	}

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
	auto connect(new_passive_t&& new_passive,
			std::enable_if_t<(class_is_passive_source_v<new_passive_t>
					|| is_passive_sink_for<std::decay_t<new_passive_t>, result_t>{})>* = nullptr) &&
	{
		auto tmp = connect_policy()(
				std::forward<passive_t>(stored_passive),
				std::forward<new_passive_t>(new_passive));
		connected = true;
		return std::forward<active_t>(active).connect(std::move(tmp));
	}

	/**
	 * \brief connects a connectable, which is not a passive connectable to the
	 *        active_connection_proxy.
	 *
	 * \pre new_connectable_t needs to be connectable
	 * \post new_connectable_t is connected to the old passive.
	 * \returns a active_connection_proxy, which contains the new_connectable in its connection.
	 */
	template <class new_connectable_t,
			class = std::enable_if_t<!(class_is_passive_source_v<new_connectable_t>
					|| is_passive_sink_for<std::decay_t<new_connectable_t>, result_t>{})>>
	auto connect(new_connectable_t&& new_connectable) &&
	{
		auto connection = connect_policy()(
				std::forward<passive_t>(stored_passive),
				std::forward<new_connectable_t>(new_connectable));
		connected = true;
		return active_connection_proxy<
				active_t,
				decltype(connection),
				connect_policy>(active, std::move(connection));
	}

	/// catch function to prevent active_connection_proxies from being treated as lvalues.
	template <class new_connectable_t>
	void connect(new_connectable_t&&) &
	{
		static_assert(fc::always_false<new_connectable_t>{},
				"active_connection_proxies cannot be used multiple times.");
	}

private:
	active_t active;
	passive_t stored_passive;
	bool connected;
};

template<class active_t, class passive_t, class argument_order, class Enable = void>
struct active_passive_connect_impl;

/// Specialization of connect to call member connect of stream_proxy.
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl<
		active_t,
		passive_t,
		argument_order,
		std::enable_if_t<
				is_instantiation_of<active_connection_proxy, std::decay_t<active_t>>{}>>
{
	auto operator()(active_t&& active, passive_t&& passive)
	{
		return std::forward<active_t>(active).connect(std::forward<passive_t>(passive));
	}
};

/**
 * Specialization of active_passive_connect_impl for the case of connecting a standard connectable
 * which is not a is_passive_sink to a is_passive_source.
 */
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl<
		active_t,
		passive_t,
		argument_order,
		std::enable_if_t<
				!is_instantiation_of< active_connection_proxy,std::decay_t<active_t>>{}
				&& (( class_is_active_source_v<active_t>
						&& !fc::is_passive_sink_for<std::decay_t<passive_t>,
								typename std::decay_t<active_t>::result_t>{}
						)
					|| ( class_is_active_sink_v<active_t>
						&& !class_is_passive_source_v<passive_t>
						)
					)>>
{
	/// \return a stream_proxy which contains the active && the passive.
	auto operator()(active_t&& active, passive_t&& passive) const
	{
		return active_connection_proxy<active_t, passive_t, argument_order>
				(std::forward<active_t>(active), std::forward<passive_t>(passive));
	}
};

/**
 * Specialization for the case of connecting a active_port to a passive_port.
 * \pre active_t needs to be a active_connectable.
 * \pre passive_t needs to be a passive_connectable.
 * \post active is now connected to passive
 */
template<class active_t, class passive_t, class argument_order>
struct active_passive_connect_impl<
		active_t,
		passive_t,
		argument_order,
		std::enable_if_t<
				!is_instantiation_of< active_connection_proxy, std::decay_t<active_t>>{}
				&& ((class_is_active_source_v<active_t>
						&& fc::is_passive_sink_for<std::decay_t<passive_t>,
								typename std::decay_t<active_t>::result_t>{}
					)
					|| (class_is_active_sink_v<active_t>
						&& class_is_passive_source_v<passive_t>
						)
					)>>
{
	/// \return port_connection tag object with the type information of the connection.
	auto operator()(active_t&& active, passive_t&& passive) const
	{
		std::forward<active_t>(active).connect(std::forward<passive_t>(passive));
		using source_t = typename argument_order::template source<active_t, passive_t>;
		using sink_t = typename argument_order::template sink<active_t, passive_t>;
		return port_connection<source_t, sink_t, result_of_t<active_t>>();
	}
};

template<class source_t, class sink_t>
struct connect_impl<
		source_t,
		sink_t,
		std::enable_if_t<class_is_active_v<source_t>>>
{
	auto operator()(source_t&& source, sink_t&& sink) const
	{
		// source is active, thus it is the first parameter
		return active_passive_connect_impl<source_t, sink_t, active_source_first>()
				(std::forward<source_t>(source), std::forward<sink_t>(sink));
	}
};

template<class source_t, class sink_t>
struct connect_impl<
		source_t,
		sink_t,
		std::enable_if_t<class_is_active_v<sink_t>>>
{
	auto operator()(source_t&& source, sink_t&& sink) const
	{
		// sink is active, thus it is the first parameter
		return active_passive_connect_impl<sink_t, source_t, active_sink_first>()
				(std::forward<sink_t>(sink), std::forward<source_t>(source));
	}
};

}  //namespace detail

///active_connection_proxy is active itself
template<class T, class U> struct is_active_sink<
	detail::active_connection_proxy<T, U, detail::active_sink_first>>
		: std::true_type {};
///active_connection_proxy is active itself
template<class T, class U> struct is_active_source<
	detail::active_connection_proxy<T, U, detail::active_source_first>>
		: std::true_type {};
}  //namespace fc

#endif /* SRC_PORTS_DETAIL_ACTIVE_SOURCE_PROXY_HPP_ */
