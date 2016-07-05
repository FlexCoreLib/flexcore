#ifndef SRC_RANGE_ACTIONS_HPP_
#define SRC_RANGE_ACTIONS_HPP_

#include <numeric>
#include <algorithm>

namespace fc
{
namespace actions
{

/**
 * \brief Eager Version Higher order map aka transform as a connectable.
 *
 * \tparam operation operation to apply to each element of range.
 *
 * \see https://en.wikipedia.org/wiki/Map_%28higher-order_function%29
 */
template<class operation>
struct map_action
{
	template<class in_range>
	auto operator()(in_range input)
	{
		std::transform(begin(input), end(input), begin(input), op);
		return input;
	}
	operation op;
};

/// Create connectable which performs higher order function map.
template<class operation>
auto map(operation op)
{
	return map_action<operation> { op };
}

/**
 * \brief Eager Version of Higher order function filter as a connectable.
 *
 * \tparam predicate predicate to check if element of range passes filter.
 * needs to be function taking object convertible from elements of range
 * and return boolean.
 *
 * \note the current implementation is not overly efficient as it allocates memory on every call.
 *
 * \see https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29
 */
template<class predicate>
struct filter_action
{
	template<class in_range>
	auto operator()(in_range input)
	{
		in_range output;
		std::copy_if(begin(input), end(input), std::back_inserter(output), pred);
		return output;
	}
	predicate pred;
};

/// Create connectable which performs higher order function filter.
template<class predicate>
auto filter(predicate pred)
{
	return filter_action<predicate> { pred };
}

/**
 * \brief Eager Version of Zip Higher Order Function.
 *
 * Takes range as input and provides a single range as output.
 * Manipulates input range!
 *
 * \tparam binop Operation type to apply to elements input and parameter range.
 * \tparam param_range type of range which serves as parameter to algorithm.
 */
template<class binop, class param_range>
struct zip_action
{
	///
	template<class in_range>
	auto operator()(in_range input)
	{
		assert(static_cast<size_t>(input.size()) ==
				static_cast<size_t>(zip_with.size()));

		std::transform(
				begin(input), end(input), begin(zip_with), begin(input), op);
		return input;
	}

	binop op;
	param_range zip_with;
};

/**
 * \brief Generates Eager Zip Action with binary Operation and parameter range.
 * \param op Binary Operator which is applied pairwise to elements of param and input.
 * \param param Second Range of Zip. Elements of this are the rhs of op.
 * \return zip_action with op and param.
 */
template<class binop, class param_range>
auto zip(binop op, param_range param)
{
	return zip_action<binop, param_range>{op, param};
}

}  // namespace actions

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

#endif /* SRC_RANGE_ACTIONS_HPP_ */
