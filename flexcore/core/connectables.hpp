#ifndef SRC_CORE_CONNECTABLES_HPP_
#define SRC_CORE_CONNECTABLES_HPP_

#include <cmath>
#include <cassert>
#include <utility>

namespace fc
{

/**
 * \defgroup connectables connectables
 * \brief A collection of different useful connectables.
 *
 * A lot of them are names for simple lambdas to make code less verbose.
 */

/**
 * \addtogroup connectables
 * @{
 */

/// Increments input using prefix operator ++.
struct increment
{
	template<class T>
	auto operator()(T in) const { return ++in; }
};
/// Decrements input using prefix operator --.
struct decrement
{
	template<class T>
	auto operator()(T in) const { return --in; }
};
/// Returns input unchanged.
struct identity
{
	template<class T>
	T operator()(T in) const { return in; }
};
/// Adds a constant addend to inputs.
template<class T>
auto add(const T summand)
{
	return [summand](auto in){ return in + summand; };
}

/// Subtracts a constant subtrahend from inputs.
template<class T>
auto subtract (const T subtrahend)
{
	return [subtrahend](auto in){ return in - subtrahend; };
}

/// Multiples input by a constant factor. (aka gain)
template<class T>
auto multiply(const T factor)
{
	return [factor](auto in) { return factor * in; };
}

/// Divides inputs by a constant divisor.
template<class T>
auto divide(const T divisor)
{
	return [divisor](auto in) { return in / divisor; };
}

/// Returns absolute value on input using std::abs.
struct absolute
{
	template<class T>
	auto operator()(const T& in) const { return std::abs(in); }
};

/// Negates input using unary -.
struct negate
{
	template<class T>
	auto operator()(const T& in) const { return -in; }
};

/// Returns logical not (operator !) of input.
struct logical_not
{
	template<class T>
	auto operator()(const T& in) const { return !in; }
};

/**
 * \brief  Clamps input to closed range [min, max].
 *
 * \pre min <= max
 * \post output >= min && output <= max
 */
template<class U, class V>
auto clamp(U min, V max)
{
	assert(min <= max);
	return [min, max](auto in)
	{
		return in < min ? min: (max < in ? max : in);
	};
}

/**
 * \brief State Source, which returns a given constant every time it is called.
 *
 * \pre constant value needs to fulfill copy_constructible.
 */
template<class T>
auto constant(T x)
{
	return [x](){ return x; };
}

namespace detail
{
	template<class T>
	struct tee_op
	{
		template<class data_t>
		auto operator()(data_t&& in) const -> data_t
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
 * \param op callback which is called before forwarding tokens
 * \pre op needs to fulfill copy_constructible.
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
template<class T>
auto print(T& stream)
{
	return [&](auto in)
	{
		stream << in << '\n';
	};
}

/** @} doxygen group connectables */

}  // namespace fc

#endif /* SRC_CORE_CONNECTABLES_HPP_ */
