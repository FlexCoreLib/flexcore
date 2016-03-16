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
auto get_source(T& s)
	-> std::enable_if_t<!has_source<T>(0), decltype(s)>
{
	return s;
}
/**
 * \brief recursively extracts the source of a connection
 */
template <class T>
auto get_source(T& c)
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
auto get_sink(T& s)
	-> std::enable_if_t<!has_sink<T>(0), decltype(s)>
{
	return s;
}

/**
 * \brief recursively extracts the sink of an object with member "sink"
 */
template <class T>
auto get_sink(T& c)
	-> std::enable_if_t<has_sink<T>(0),
			decltype(get_sink(c.sink))>
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
	typedef T type;
};

/**
 * \brief specialization for the case of a connection
 */
template <class T>
struct get_source_t<T,
		std::enable_if_t< has_source<T>(0) >
	>
{
	typedef typename get_source_t<decltype(std::declval<T>().source)>::type type;
};

/**
 * \brief gets the sink type of the connectalbe
 * \pre connectable is either a connection or a sink
 */
template <class T, class Enable = void>
struct get_sink_t
{
	typedef T type;
};

/**
 * \brief specialization for the case of a connection
 */
template <class T>
struct get_sink_t<T,
		std::enable_if_t< has_sink<T>(0) >
	>
{
	typedef typename get_sink_t<decltype(std::declval<T>().sink)>::type type;
};

}// namespace fc
#endif /* SRC_PORTS_CONNECTION_UTIL_HPP_ */
