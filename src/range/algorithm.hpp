#ifndef SRC_RANGE_ALGORITHM_HPP_
#define SRC_RANGE_ALGORITHM_HPP_

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/combine.hpp>
#include "boost/tuple/tuple.hpp"

#include <iterator>
#include <numeric>

namespace fc
{
namespace views
{


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
	auto operator()(in_range&& input)
	{
		return boost::adaptors::filter(input, pred);
	}
	predicate pred;
};

/// Create connectable which performs higher order function filter.
template<class predicate>
auto filter(predicate pred)
{
	return filter_view<predicate> { pred };
}

/**
 * \brief Higher order map aka transform as a connectable.
 *
 * \tparam operation operation to apply to each element of range.
 *
 * \see https://en.wikipedia.org/wiki/Map_%28higher-order_function%29
 */
template<class operation>
struct map_view
{
	template<class in_range>
	auto operator()(in_range&& input)
	{
		return boost::adaptors::transform(input, op);
	}
	operation op;
};

/// Create connectable which performs higher order function map.
template<class operation>
auto map(operation op)
{
	return map_view<operation> { op };
}

/**
 * \brief Lazy Version of Zip Higher Order Function.
 *
 * Takes range as input and provides a single range as output.
 * Elements of single range are result of pairwise application of binop.
 * Calculations are only done when the output range is used.
 *
 * \tparam binop Operation type to apply to elements input and parameter range.
 * \tparam param_range type of range which serves as parameter to algorithm.
 */
template<class binop, class param_range>
struct zip_view
{
	template<class in_range>
	auto operator()(in_range&& input)
	{
		assert(static_cast<size_t>(input.size()) ==
				static_cast<size_t>(zip_with.size()));

		return boost::adaptors::transform(
				boost::combine(zip_with, input), //zips ranges to tuple
				[op = this->op](auto&& in){ return op(boost::get<0>(in), boost::get<1>(in)); });
	}

	binop op;
	param_range zip_with;
};

/**
 * \brief Generates Lazy Zip View with binary Operation and parameter range.
 * \param op Binary Operator which is applied pairwise to elements of param and input.
 * \param param Second Range of Zip. Elements of this are the rhs of op.
 * \return zip_view with op and param.
 */
template<class binop, class param_range>
auto zip(binop op, param_range param)
{
	return zip_view<binop, param_range>{op, param};
}

} //namespace views


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
	explicit reduce_view(const binop& op = binop(), const T&  init_value = T())
		: op(op), init_value(init_value)
	{
	}

	template<class in_range>
	auto operator()(in_range&& input)
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
	return reduce_view<binop, T> { op, initial_value };
}

///alias of reduce for common case of using reduce to sum all elements
template<class T>
auto sum(T initial_value = T())
{
	return reduce(std::plus<>(), initial_value);
}

}  // namespace fc

#endif /* SRC_RANGE_ALGORITHM_HPP_ */
