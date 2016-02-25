#ifndef SRC_CORE_DETAIL_CONNECTION_UTILS_HPP_
#define SRC_CORE_DETAIL_CONNECTION_UTILS_HPP_

#include <core/detail/connection.hpp>

namespace fc
{
namespace detail
{

template <class conn_t>
struct apply_helper;
template <class functor, class conn_t>
void apply(functor, conn_t&);

template <class source_t, class sink_t>
struct apply_helper<connection<source_t, sink_t>>
{
	template <typename functor>
	void operator()(functor f, connection<source_t, sink_t>& conn) const
	{
		apply(f, conn.source);
		apply(f, conn.sink);
	}
};

template <class conn_t>
struct apply_helper
{
	template <typename functor>
	void operator()(functor f, conn_t& conn) const
	{
		f(conn);
	}
};

template <class functor, class conn_t>
void apply(functor f, conn_t& conn)
{
	apply_helper<conn_t>()(f, conn);
}

} // namespace fc
} // namespace detail

#endif // SRC_CORE_DETAIL_CONNECTION_HPP_
