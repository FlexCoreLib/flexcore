#ifndef SRC_RANGE_ACTIONS_HPP_
#define SRC_RANGE_ACTIONS_HPP_

#include <numeric>
#include <algorithm>
#include <cassert>

namespace fc
{

/// Range Actions are eager versions of algorithms working on iterator ranges.
namespace actions
{

/**
 * \brief Eager Version Higher order map aka transform as a connectable.
 *
 * This version is used when the operation changes the type of the range values.
 *
 * \tparam operation operation to apply to each element of range.
 * \tparam target_range type of range or container the result should be stored in.
 *
 * \note The value type received needs to be default constructable
 * as the output vector is resized to the correct size at the moment.
 *
 * \see https://en.wikipedia.org/wiki/Map_%28higher-order_function%29
 */
template<class operation, class target_range>
struct map_action
{
	template<class in_range>
	decltype(auto) operator()(in_range&& input)
	{
		using std::begin;
		using std::end;
		target.resize(input.size());
		std::transform(begin(input), end(input), begin(target), op);
		return target;
	}
	operation op;
	target_range target;
};

/// Specialization for map_action where type of output range is the same as input range.
template<class operation>
struct map_action<operation, void>
{
	template<class in_range>
	decltype(auto) operator()(in_range input)
	{
		using std::begin;
		using std::end;
		std::transform(begin(input), end(input), begin(input), op);
		return input;
	}
	operation op;
};

/**
 * \brief Create connectable which performs higher order function map
 * \param op operation to execute on each element in range
 */
template<class operation>
auto map(operation op)
{
	return map_action<operation, void> { op };
}

/**
 * \brief Create connectable which performs higher order function map
 * where the output type is different than input type.
 * \param op operation to execute on each element in range
 * \param t target range, needs to be a range which can take the result of op.
 */
template<class operation, class target_range>
auto map(operation op, target_range t = target_range{} )
{
	return map_action<operation, target_range> { op, t };
}

/**
 * \brief Eager Version of Higher order function filter as a connectable.
 *
 * \tparam predicate predicate to check if element of range passes filter.
 * needs to be function taking object convertible from elements of range
 * and return boolean.
 *
 * \see https://en.wikipedia.org/wiki/Filter_%28higher-order_function%29
 */
template<class predicate>
struct filter_action
{
	template<class in_range>
	auto operator()(in_range data)
	{
		auto negatef = [](auto f)
		{
			return [f](auto in){ return !f(in); };
		};
            
		using std::begin;
		using std::end;
		//if in_range::value_t is trivialy destructable,
		//no destructors are called, and we just swap values.
    		auto mid = std::remove_if(begin(data), end(data), negatef(pred));
        	data.erase(mid, end(data));
		return data;
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
	template<class in_range>
	auto operator()(in_range input)
	{
		assert(static_cast<size_t>(input.size()) ==
				static_cast<size_t>(zip_with.size()));

		using std::begin;
		using std::end;
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
