#ifndef SRC_PORTS_CONNECTION_UTIL_HPP_
#define SRC_PORTS_CONNECTION_UTIL_HPP_

#include <utility>

#include <core/connection.hpp>
#include <core/traits.hpp>

namespace fc
{

/**
 * \brief generalization of get_source
 * Stopping criterion for recursion
 */
template <class T>
auto get_source(const T& s)
	-> const typename std::enable_if<!has_source<T>(0), T>::type&
{
	return s;
}
/**
 * \brief recursively extracts the source of a connection
 */
template <class T>
auto get_source(const T& c)
	-> const typename std::enable_if<has_source<T>(0),
			decltype(get_source(c.source))>::type&
{
	return get_source(c.source);
}

/**
 * \brief generalization of get_sink
 * Stopping criterion for recursion
 */
template <class T>
auto get_sink(const T& s)
	-> const typename std::enable_if<!has_sink<T>(0), T>::type&
{
	return s;
}

/**
 * \brief recursively extracts the sink of an object with member "sink"
 */
template <class T>
auto get_sink(const T& c)
	-> const typename std::enable_if<has_sink<T>(0),
			decltype(get_sink(c.sink))>::type&
{
	return get_sink(c.sink);
}

/**
 * \brief gets the source type of the connectalbe
 * \pre connectable is either a connection or a source
 */
template <class T, class Enable = void>
struct get_source_t
{
	typedef T value;
};

/**
 * \brief specialization for the case of a connection
 */
template <class T>
struct get_source_t<T,
		typename std::enable_if< has_source<T>(0) >::type
	>
{
	typedef typename get_source_t<decltype(std::declval<T>().source)>::value value;
};

/**
 * \brief gets the sink type of the connectalbe
 * \pre connectable is either a connection or a sink
 */
template <class T, class Enable = void>
struct get_sink_t
{
	typedef T value;
};

/**
 * \brief specialization for the case of a connection
 */
template <class T>
struct get_sink_t<T,
		typename std::enable_if< has_sink<T>(0) >::type
	>
{
	typedef typename get_sink_t<decltype(std::declval<T>().sink)>::value value;
};

}// namespace fc
#endif /* SRC_PORTS_CONNECTION_UTIL_HPP_ */
