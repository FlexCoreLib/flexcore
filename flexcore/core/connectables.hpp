#ifndef SRC_CORE_CONNECTABLES_HPP_
#define SRC_CORE_CONNECTABLES_HPP_

#include <cmath>
#include <cassert>
#include <utility>

namespace fc
{

/**
 * \defgroup connectables
 * \brief A collection of different useful connectables.
 *
 * A lot of them are names for simple lambdas to make code less verbose.
 */

/**
 * \addtogroup connectables
 * @{
 */

/// Increments input using prefix operator ++.
auto increment = [](auto in) { return ++in; };
/// Decrements input using prefix operator --.
auto decrement = [](auto in) { return --in; };
/// Returns input unchanged.
auto identity = [](auto in) { return in; };

/// Adds a constant addend to inputs.
auto add = [](const auto summand)
{
	return [summand](auto in){ return in + summand; };
};

/// Subtracts a constant subtrahend from inputs.
auto subtract = [](const auto subtrahend)
{
	return [subtrahend](auto in){ return in - subtrahend; };
};

/// Multiples input by a constant factor.
auto multiply = [](const auto factor)
{
	return [factor](auto in) { return factor * in; };
};

/// Divides inputs by a constant divisor.
auto divide = [](const auto divisor)
{
	return [divisor](auto in) { return in / divisor; };
};

/// Returns absolute value on input using std::abs.
auto absolute = [](auto in) { return std::abs(in); };

/// Negates input using unary -.
auto negate = [](auto in) { return -in; };

/// Returns logical not (operator !) of input.
auto logical_not = [](auto in) { return !in; };

/**
 * \brief  Clamps input to closed range [min, max].
 *
 * \pre min <= max
 * \post output >= min && output <= max
 */
auto clamp = [](auto min, auto max)
{
	assert(min <= max);
	return [min, max](auto in)
	{
		return in < min ? min: (max < in ? max : in);
	};
};

/**
 * \brief State Source, which returns a given constant every time it is called.
 *
 * \pre constant value needs to fulfill copy_constructible.
 */
auto constant = [](auto x)
{
	return [x]()
	{
		return x;
	};
};

namespace detail
{
	template<class T>
	struct tee_op
	{
		template<class data_t>
		auto operator()(data_t&& in) -> data_t
		{
			// call callback with const_ref to make sure it cannot change token
			// But token can still be move_only
			const auto& temp_ref = in;
			callback(temp_ref);

			return std::forward<data_t>(in);
		}

		T callback;
	};
}

/**
 * \brief Calls a given callback and then returns value every time it is called.
 *
 * \pre @param callback needs to fulfill copy_constructible.
 */
template<class T>
auto tee(T&& op)
{
	return detail::tee_op<T>{std::forward<T>(op)};
}

/**
 * \brief Event_sink, which prints all incoming tokens to given stream.
 *
 * Ends every token with a new_line character.
 * Use this together with tee to print tokens in chains.
 * source >> tee(print(std::cout)) >> sink;
 *
 * \param stream Results are printed to this using operator <<.
 * print does not take ownership of stream.
 */
auto print = [](auto& stream)
{
	return [&](auto in)
	{
		stream << in << "\n";
	};
};

/** @} doxygen group connectables */

}  // namespace fc

#endif /* SRC_CORE_CONNECTABLES_HPP_ */
