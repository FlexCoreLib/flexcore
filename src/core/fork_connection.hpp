/*
 * fork_connection.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CORE_FORK_CONNECTION_HPP_
#define SRC_CORE_FORK_CONNECTION_HPP_

namespace fc
{

namespace detail
{

template<class source_t, class sink_t>
struct fork_connection : public connection_trait<source_t, sink_t>::type
{
	fork_connection(const source_t& source,const sink_t& sink)
			: connection_trait<source_t, sink_t>::type {source, sink}
	{
	}
};

template<class source_t, class sink_t, class enable = void>
struct
fork_impl
{
	auto operator()(const source_t& source, const sink_t& sink)
	{
		return fork_connection<source_t, sink_t>(source, sink);
	}
};

template<class source_t, class sink_t>
struct
fork_impl<source_t, sink_t,
		typename std::enable_if<is_instantiation_of<fork_connection, source_t>::value>::type>
{
	auto //fork_connection<decltype(source.source), sink_t>
	operator()(const source_t& source, const sink_t& sink)
	{
		return fork_connection<decltype(source.source), sink_t>(source.source, sink);
	}
};

} // namespace detail

template<class source_t, class sink_t,
		class = typename std::enable_if<is_connectable<source_t>::value>::type,
		class = typename std::enable_if<is_connectable<sink_t>::value>::type>
detail::fork_impl<source_t,sink_t> fork(const source_t& source, const sink_t& sink)
{
	return detail::fork_impl<source_t,sink_t>()(source, sink);
}

template<class source_t, class sink_t,
	class = typename std::enable_if<is_connectable<source_t>::value>::type,
	class = typename std::enable_if<is_connectable<sink_t>::value>::type>
detail::fork_impl<source_t,sink_t> operator |(const source_t& source, const sink_t& sink)
{
	return fork(source, sink);
}
} // namespace fc

#endif /* SRC_CORE_FORK_CONNECTION_HPP_ */
