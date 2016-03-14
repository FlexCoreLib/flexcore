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


}  // namespace actions
}  // namespace fc

#endif /* SRC_RANGE_ACTIONS_HPP_ */
