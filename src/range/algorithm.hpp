#ifndef SRC_RANGE_ALGORITHM_HPP_
#define SRC_RANGE_ALGORITHM_HPP_

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>

#include <functional>

namespace fc {

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

template<class predicate>
auto filter(predicate pred)
{
	return filter_view<predicate>{pred};
}

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

template<class operation>
auto map(operation op)
{
	return map_view<operation>{op};
}

template<class binop, class T>
struct reduce_view
{
	template<class in_range>
	auto operator()(const in_range&& input)
	{
		return std::accumulate(begin(input), end(input), init_value, op);
	}
	binop op;
	T init_value;
};

template<class binop, class T>
auto reduce(binop op, T initial_value)
{
	return reduce_view<binop, T>{op, initial_value};
}

template<class T>
auto sum(T initial_value = T())
{
	return reduce(std::plus<>(), initial_value);
}

}  // namespace fc

#endif /* SRC_RANGE_ALGORITHM_HPP_ */
