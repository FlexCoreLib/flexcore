#ifndef SRC_CORE_CONNECTABLES_HPP_
#define SRC_CORE_CONNECTABLES_HPP_

#include <cmath>
#include <cassert>

namespace fc
{

/**
 * A collection of different useful connectables.
 * A lot of them are names for simples lambdas do make code less verbose.
 */

/// Increments input using prefix operator ++.
auto increment = [](auto in) { return ++in; };
/// Decrements input using prefix operator --.
auto decrement = [](auto in) { return --in; };
/// Returns input unchanged.
auto identity = [](auto in) { return in; };

/// Adds a constant addend to inputs.
auto add_to = [](const auto summand)
{
	return [summand](auto in){ return in + summand; };
};

/// Subtracts a constant subtrahend from inputs.
auto subtract_value = [](const auto subtrahend)
{
	return [subtrahend](auto in){ return in - subtrahend; };
};

/// Multiples input by a constant factor.
auto multiply_with = [](const auto factor)
{
	return [factor](auto in) { return factor * in; };
};

/// Divides inputs by a constant divisor.
auto divide_by = [](const auto divisor)
{
	return [divisor](auto in) { return in / divisor; };
};

/// Returns absolute value on input using std::abs.
auto abs = [](auto in) { return std::abs(in); };

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

}  // namespace fc

#endif /* SRC_CORE_CONNECTABLES_HPP_ */
