#ifndef SRC_PORTS_MUX_PORTS_HPP_
#define SRC_PORTS_MUX_PORTS_HPP_

#include <tuple>
#include <utility>
#include <core/function_traits.hpp>
#include <core/traits.hpp>
#include <core/tuple_meta.hpp>

namespace fc
{
template <class... ports>
struct mux_port;
template <class op>
struct unloaded_merge_port;
template <class op, class... ports>
struct loaded_merge_port;

struct default_tag {};
struct merge_tag {};
struct mux_tag {};

/// Traits used to determine whether a muxed port is connecting to a muxed
/// port, a merge port or any other connectable object.
template <typename T>
struct port_traits
{
	using mux_category = default_tag;
};

template <class... ports>
struct port_traits<mux_port<ports...>>
{
	using mux_category = mux_tag;
};

template <class op>
struct port_traits<unloaded_merge_port<op>>
{
	using mux_category = merge_tag;
};

namespace detail
{
/// Helper: check if any of the passed bools are true.
template <bool... vals>
constexpr bool any()
{
	bool values[] = {vals...};
	for (auto value : values)
		if (value)
			return true;
	return false;
}

/** \brief Helper: return false regardless of what the parameters are.
 *
 * Useful for static_assert conditions in templates that are always invalid -
 * prevents them from firing before the template is instantiated.
 */
template <typename... T>
constexpr bool always_false_fun(T...)
{
	return false;
}

/// Helper: check if all of the passed bools are true.
template <bool... vals>
constexpr bool all()
{
	bool values[] = {vals...};
	for (auto value : values)
		if (!value)
			return false;
	return true;
}
} // namespace detail

template <class base>
struct node_aware;

struct many_to_many_tag {};
struct one_to_many_tag {};
struct many_to_one_tag {};

/// Trait used to determine how many muxed ports are being connected on each side.
template <size_t left_ports, size_t right_ports>
struct mux_traits
{
	static_assert(
	    detail::always_false_fun(left_ports, right_ports),
	    "Only N->N, 1->N and N->1 connections possible between muxed ports. PS: don't try 1->1.");
};
template <size_t ports>
struct mux_traits<ports, ports>
{
	using connection_category = many_to_many_tag;
};
template <size_t ports>
struct mux_traits<ports, 1>
{
	using connection_category = many_to_one_tag;
};
template <size_t ports>
struct mux_traits<1, ports>
{
	using connection_category = one_to_many_tag;
};

/** \brief A wrapper around multiple ports that simplifies establishing
 *         *identical* connections.
 *
 * Basically will lead to the following transformation:
 *
 *     mux(a,b,c) >> d       ===>      a >> d
 *                                     b >> d
 *                                     c >> d
 *
 * with special cases for mux(...) >> mux(...), and mux(...) >> merge_port.
 *
 * Mux ports can be used on both sides of a connection but need to be at the
 * ends of a connection chain. Mux ports don't fulfill the concept of
 * connectable (or active connectable for that matter) but they do have special
 * overloads of operator>> which allow them to participate in connections.
 */
template <class... port_ts>
struct mux_port
{
	std::tuple<port_ts...> ports;

private:
	/** \brief Overload for connecting to an unloaded_merge_port
	 *
	 * T is expected to be an instantiation of unloaded_merge_port.
	 * T and T.merge are assumed to be copyable.
	 *
	 * \returns loaded_merge_port with the supplied merge operation and the
	 *          ports previously held by *this.
	 */
	template <class T>
	auto connect(T t, merge_tag)
	{
		static_assert(
		    detail::has_result_of_type<decltype(t.merge), decltype(std::declval<port_ts>()())...>(),
		    "The muxed ports can not be merged using the provided merge function.");
		static_assert(!detail::any<detail::is_derived_from<node_aware, port_ts>::value...>(),
		              "Merge port can not be used with node aware ports. See merge_node.");
		return loaded_merge_port<decltype(t.merge), port_ts...>{t.merge, std::move(ports)};
	}

	/** \brief Overload for connecting to another muxed port.
	 *
	 * other_mux is expected to be an instantiation of mux_port, the ports
	 * in *this and in other should have compatible types.
	 * The amount of ports on both sides also needs to be compatible:
	 *
	 *   * N - N
	 *   * 1 - N
	 *   * N - 1
	 *
	 * for N > 1. The appropriate overload of connect_mux is selected using mux_traits.
	 */
	template <class other_mux>
	auto connect(other_mux&& other, mux_tag)
	{
		constexpr size_t this_ports = sizeof...(port_ts);
		constexpr size_t other_ports = std::tuple_size<decltype(other.ports)>::value;
		using connection_tag = typename mux_traits<this_ports, other_ports>::connection_category;
		return connect_mux(std::forward<other_mux>(other), connection_tag{});
	}

	/** \brief Connect each port in *this with each port in other.
	 *
	 * mux-mux connections should be the end of a connection chain.
	 * \returns (if used correctly) std::tuple<port_connections...>
	 */
	template <class other_mux_port>
	auto connect_mux(other_mux_port&& other, many_to_many_tag)
	{
		auto pairwise_connect = [](auto&& l, auto&& r)
		{
			return std::forward<decltype(l)>(l) >> std::forward<decltype(r)>(r);
		};
		return fc::tuple::transform(std::move(ports), std::forward<other_mux_port>(other).ports,
		                            pairwise_connect);
	}

	/** \brief Connect each port in *this with the one (sink) port in other.
	 *
	 * The sink port is copied if it was held by value inside other, or else it
	 * will be passed to the connection as an lvalue.
	 *
	 * \returns std::tuple<port_connections...>
	 */
	template <class sink_t>
	auto connect_mux(mux_port<sink_t>&& other, many_to_one_tag)
	{
		sink_t&& sink = std::get<0>(std::move(other).ports);
		auto all_to_sink = [&sink](auto&& port)
		{
			return std::forward<decltype(port)>(port) >> static_cast<sink_t>(sink);
		};
		return fc::tuple::transform(std::move(ports), all_to_sink);
	}

	/** \brief Connect each port in other with the (source) port in *this.
	 *
	 * The source port will be copied, for each connection, if it was held by
	 * value inside *this, otherwise it will be connected as an lvalue.
	 *
	 * \returns std::tuple<port_connections...>
	 */
	template <class other_mux_port>
	auto connect_mux(other_mux_port&& other, one_to_many_tag)
	{
		static_assert(
		    sizeof...(port_ts) == 1,
		    "(*this).ports should have a single port; if you are here, your logic must be wrong.");
		using src_t = typename std::tuple_element<0, decltype(ports)>::type;
		src_t&& src = std::get<0>(std::move(ports));
		auto src_to_all = [&src](auto&& port)
		{
			return static_cast<src_t>(src) >> std::forward<decltype(port)>(port);
		};
		return fc::tuple::transform(std::forward<other_mux_port>(other).ports, src_to_all);
	}

	/** \brief Overload for connecting all ports in *this with a connectable.
	 *
	 * \param T the connectable - will be copied unless it is an lvalue.
	 * \returns mux_port<connection...> (or mux_port<port_connection...>)
	 */
	template <class T>
	auto connect(T&& t, default_tag)
	{
		auto connect_to_copy = [&t](auto&& elem)
		{
			// t needs to be copied if it's not an lvalue ref, forward won't work because can't move
			// from smth multiple times.
			return std::forward<decltype(elem)>(elem) >> static_cast<T>(t);
		};
		return mux_from_tuple(fc::tuple::transform(std::move(ports), connect_to_copy));
	}

public:
	/// Function to forward to the correct connect overload based on the type of T.
	template <class T>
	auto operator>>(T&& t) &&
	{
		using decayed = std::decay_t<T>;
		using tag = typename fc::port_traits<decayed>::mux_category;
		return this->connect(std::forward<T>(t), tag{});
	}
	template <class T>
	auto operator>>(T&&) & = delete;
};

/// Connect src with all ports in mux. Useful only with event_sources.
template <class T, class... ports,
          class = std::enable_if_t<is_active_source<std::remove_reference_t<T>>{} &&
                                   detail::all<is_connectable<ports>{}...>()>>
auto operator>>(T&& src, mux_port<ports...> mux)
{
	return mux_port<T>{std::forward_as_tuple(std::forward<T>(src))} >> std::move(mux);
}

/** \brief Create a mux port from lvalue references to the supplied ports.
 *
 * \params ports should be non-const connectables.
 */
template <class... port_ts>
auto mux(port_ts&&... ports)
{
	return mux_port<std::remove_const_t<port_ts>...>{
	    std::forward_as_tuple(std::forward<port_ts>(ports)...)};
}

template <class... conn_ts>
mux_port<conn_ts...> mux_from_tuple(std::tuple<conn_ts...> tuple_)
{
	return mux_port<conn_ts...>{std::move(tuple_)};
}

/// A merge port that is not connected to a mux_port yet. Think of this as a proxy.
template <class merge_op>
struct unloaded_merge_port
{
	merge_op merge;
};

/** \brief A merge_port with a merge operation and a set of ports (connectables).
 *
 * If a connectable is held by value then loaded_merge_port is the owner of
 * that connectable.
 */
template <class merge_op, class... port_ts>
struct loaded_merge_port
{
	merge_op op;
	std::tuple<port_ts...> ports;

	auto operator()()
	{
		auto op = this->op;
		auto call_and_apply = [op](auto&&... src)
		{
			return op(std::forward<decltype(src)>(src)()...);
		};
		return tuple::invoke_function(call_and_apply, ports,
		                              std::make_index_sequence<sizeof...(port_ts)>{});
	}
};

/** \brief Create a merge_port proxy that only holds the merge operation.
 *
 * A merge port is an entity that can merge incoming states from several state
 * sources. It does not take into account region information and so it can't be
 * used with node aware ports.
 *
 * A merge port needs to appear to the right of a mux port in a connection and
 * can _only_ be connected to a mux port. Changing connection precedence using
 * parentheses is not supported. Example:
 *
 *     mux(a,b,c) >> lambda >> merge(op) >> state_sink
 *
 * This connection will work because `mux(a,b,c) >> lambda` returns another mux
 * port. Changing the connection precedence:
 *
 *     mux(a,b,c) >> lambda >> (merge(op) >> state_sink)
 *
 * will not work because a merge port can not be connected to anything but a mux
 * port.
 *
 * **Impl detail**: Connecting a merge port to a mux_port does not call
 *                  operator>> of any underlying port, so any mixins (including
 *                  graph information) will not get a chance to perform their
 *                  job.
 */
template <class merge_op>
auto merge(merge_op op)
{
	return unloaded_merge_port<merge_op>{op};
}
} // namespace fc
#endif // SRC_PORTS_MUX_PORTS_HPP_

