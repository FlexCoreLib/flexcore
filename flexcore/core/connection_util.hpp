#ifndef SRC_PORTS_CONNECTION_UTIL_HPP_
#define SRC_PORTS_CONNECTION_UTIL_HPP_

#include <flexcore/core/traits.hpp>

#include <utility>

namespace fc
{

/**
 * \brief generalization of get_source
 * Stopping criterion for recursion
 */
template <class T>
constexpr auto get_source(T& s)
	-> std::enable_if_t<!has_source<T>(0), decltype(s)>
{
	return s;
}
/**
 * \brief recursively extracts the source of a connection
 */
template <class T>
constexpr auto get_source(T& c)
	-> std::enable_if_t<has_source<T>(0),
			decltype(get_source(c.source))>
{
	return get_source(c.source);
}

/**
 * \brief generalization of get_sink
 * Stopping criterion for recursion
 */
template <class T>
constexpr auto get_sink(T& s)
	-> std::enable_if_t<!has_sink<T>(0), decltype(s)>
{
	return s;
}

/**
 * \brief recursively extracts the sink of an object with member "sink"
 */
template <class T>
constexpr auto get_sink(T& c)
	-> std::enable_if_t<has_sink<T>(0),
			decltype(get_sink(c.sink))>
{
	return get_sink(c.sink);
}

}// namespace fc
#endif /* SRC_PORTS_CONNECTION_UTIL_HPP_ */
