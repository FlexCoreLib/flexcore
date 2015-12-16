#ifndef SRC_RANGE_ALGORITHM_HPP_
#define SRC_RANGE_ALGORITHM_HPP_

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>

#include <functional>

namespace fc {

/**
 * \brief Higher order function filter as a connectable.
 *
 * \tparam predicate predicate to check if element of range passes filter.
 * needs to be function taking object convertible from elements of range
 * and return boolean.
 *
 * \see https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29
 */
template<class predicate>
struct filter_view
{
	template<class in_range>
	auto operator()(const in_range&& input)
	{
		return boost::adaptors::filter(input, pred);
	}
	predicate pred;
};

/// Create connectable which performs higher order function filter.
template<class predicate>
auto filter(predicate pred)
{
	return filter_view<predicate>{pred};
}

/**
 * \brief Higher order map aka transform filter as a connectable.
 *
 * \tparam operation operation to apply to each element of range.
 *
 * \see https://en.wikipedia.org/wiki/Map_%28higher-order_function%29
 */
template<class operation>
struct map_view
{
	template<class in_range>
	auto operator()(const in_range&& input)
	{
		return boost::adaptors::transform(input, op);
	}
	operation op;
};

/// Create connectable which performs higher order function map.
template<class operation>
auto map(operation op)
{
	return map_view<operation>{op};
}

/**
 * \brief Higher order function reduce aka fold as a connectable.
 *
 * \tparam binop binary operation to repeatedly apply to the whole range.
 *
 * \see https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29
 */
template<class binop, class T>
struct reduce_view
{
	template<class in_range>
	auto operator()(const in_range&& input)
	{
		using std::begin;
		using std::end;
		return std::accumulate(begin(input), end(input), init_value, op);
	}
	binop op;
	T init_value;
};

/// Create connectable which performs higher order function reduce.
template<class binop, class T>
auto reduce(binop op, T initial_value)
{
	return reduce_view<binop, T>{op, initial_value};
}

///alias of reduce for common case of using reduce to sum all elements
template<class T>
auto sum(T initial_value = T())
{
	return reduce(std::plus<>(), initial_value);
}

}  // namespace fc

#endif /* SRC_RANGE_ALGORITHM_HPP_ */
