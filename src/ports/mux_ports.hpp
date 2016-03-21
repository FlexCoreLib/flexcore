#pragma once

#include <tuple>
#include <utility>
#include <core/traits.hpp>
#include <core/tuple_meta.hpp>

namespace fc
{

template <class... port_ts>
struct mux_port
{
	std::tuple<port_ts...> ports;

	template <class... other_ports>
	auto connect(mux_port<other_ports...> other, std::true_type)
	{
		constexpr size_t N = sizeof...(port_ts);
		constexpr size_t M = sizeof...(other_ports);
		static_assert(N == M, "Mux ports need to have the same number of subports.");
		auto pairwise_connect = [](auto&& l, auto&& r)
		{
			return std::forward<decltype(l)>(l) >> std::forward<decltype(r)>(r);
		};
		return fc::tuple::transform(std::move(ports), std::move(other.ports), pairwise_connect);
	}

	template <class T>
	auto connect(T&& t, std::false_type)
	{
		auto connect_to_copy = [&t](auto&& elem)
		{
			// t needs to be copied if it's not an lvalue ref, forward won't work because can't move
			// from smth multiple times.
			return std::forward<decltype(elem)>(elem) >> static_cast<T>(t);
		};
		return mux_from_tuple(fc::tuple::transform(std::move(ports), connect_to_copy));
	}

	template <class T>
	auto operator>>(T&& t)
	{
		using is_mux_port = fc::is_instantiation_of<fc::mux_port, std::decay_t<T>>;
		return this->connect(std::forward<T>(t), is_mux_port{});
	}
};

template <class... port_ts>
auto mux(port_ts&... ports)
{
	return mux_port<std::remove_const_t<port_ts>&...>{std::tie(ports...)};
}

template <class... conn_ts>
mux_port<conn_ts...> mux_from_tuple(std::tuple<conn_ts...> tuple_)
{
	return mux_port<conn_ts...>{std::move(tuple_)};
}
} // namespace fc
