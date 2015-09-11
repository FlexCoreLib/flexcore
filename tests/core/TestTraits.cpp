// TestTraits.cpp ---
//
// Filename: TestTraits.cpp
// Description:
// Author: Thomas Karolski
// Created: Mo Sep  7 16:10:29 2015 (+0200)
//
//
//

// Code:

// boost
#include <boost/test/unit_test.hpp>

#include <core/traits.hpp>

#include <iostream>
#include <type_traits>

//////////////////////////////////////////////////
/// MACROS

/// Staticly assert that the expr has a result_of of the specified type
#define ASSERT_RESULT_OF(expr, t)										\
	{																	\
		auto callable_fd3s81sdk = expr;									\
		static_assert( (std::is_same< result_of< decltype(callable_fd3s81sdk) >::type, t >::value), \
					   "result_of should return type '" #t "' for '" #expr "'."); \
	}

/// Staticly assert, that the expr has an nth argument with the specified type
#define ASSERT_ARG_TYPE(expr, n, t)										\
	{																	\
		auto callable_fd3s81sdk = expr;									\
		static_assert(std::is_same< argtype_of<decltype(callable_fd3s81sdk), n>::type, t >::value, \
						  "Parameter #" #n " of '" #expr "' should be of type '" #t "'"); \
																		\
	}



//////////////////////////////////////////////////
/// Dummy things to check against

/// Dummy class to test result_of & argument deduction
class CustomCallable
{
public:
	bool operator()()
	{
		return 0;
	}
};

/// Dummy class to test result_of & argument deduction
struct CustomCallable2
{
	void operator()(const CustomCallable&, CustomCallable&&)
	{
	}
};

/// Dummy function to test result_of & argument deduction
static bool StaticFunction(int)
{
	return true;
}



//////////////////////////////////////////////////
/// Actual tests

BOOST_AUTO_TEST_CASE( test_callable_traits )
{
	{ /// Check the return value of a bunch of callables
		std::function<int(int)> callable1;
		auto callable2 = []() { return std::string("Test"); };
		CustomCallable callable3;
		CustomCallable2 callable4;
		auto callable5 = StaticFunction;

		ASSERT_RESULT_OF(callable1, int);
		static_assert(utils::function_traits<decltype(callable1)>::arity == 1,
				"Arity of std::function<int(int)> should be 1.");
		ASSERT_ARG_TYPE(callable1, 0, int);

		ASSERT_RESULT_OF(callable2, std::string);
		ASSERT_RESULT_OF(callable3, bool);

		ASSERT_ARG_TYPE(callable4, 0, const CustomCallable&);
		ASSERT_ARG_TYPE(callable4, 1, CustomCallable&&);

		ASSERT_RESULT_OF(callable5, bool);


		static_assert(is_callable<CustomCallable>::value,
				"this type was made to be callable");
		static_assert(is_connectable<CustomCallable>::value,
				"struct was made to be connectable");
	}
}


//
// TestTraits.cpp ends here
