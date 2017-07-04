// TestTraits.cpp ---
//
// Filename: TestTraits.cpp
// Description:
// Author: Thomas Karolski
// Created: Mo Sep  7 16:10:29 2015 (+0200)
//
//
//

#include <boost/test/unit_test.hpp>

#include <flexcore/core/traits.hpp>

#include <iostream>
#include <type_traits>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_fc_traits)

namespace
{

//////////////////////////////////////////////////
/// MACROS

/// Staticly assert that the expr has a result_of of the specified type
#define ASSERT_RESULT_OF(expr, t)										\
	{																	\
		auto callable_fd3s81sdk = expr;									\
		static_assert( (std::is_same< result_of_t< decltype(callable_fd3s81sdk) >, t >{}), \
					   "result_of should return type '" #t "' for '" #expr "'."); \
	}

/// Staticly assert, that the expr has an nth argument with the specified type
#define ASSERT_ARG_TYPE(expr, n, t)										\
	{																	\
		auto callable_fd3s81sdk = expr;									\
		static_assert(std::is_same< argtype_of<decltype(callable_fd3s81sdk), n>::type, t >{}, \
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

class CustomCallableArg
{
public:
	bool operator()(int)
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

struct result_haver
{
	using result_t = int; // has a result_t
};

} //namespace
//////////////////////////////////////////////////
/// Actual tests

BOOST_AUTO_TEST_CASE( test_callable_traits )
{
	/// Check the return value of a bunch of callables
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

	static_assert(is_callable<CustomCallable>{},
			"this type was made to be callable");
	static_assert(is_connectable<CustomCallable>{},
			"struct was made to be connectable");

	auto test = [](int){ };
	static_assert(is_callable<decltype(test)>{},
			"lambda taking int is callable");
	static_assert(is_connectable<decltype(test)>{},
			"lambda taking int is connectable");

	auto con_lambda = [](int)->int{return 1;};
	static_assert(is_connectable<decltype(con_lambda)>{},
			"lambda takes and returns int, is connectable");

	static_assert(!has_result<CustomCallable>{}, "CustomCallable does not have a result_t");
	static_assert(has_result<result_haver>{}, "result_haver has a result_t");

	static_assert(std::is_same<result_of_t<result_haver>, int>{}, "result_t is int");
}

namespace
{
struct not_callable
{
	//note the absence of operator()
};
}

BOOST_AUTO_TEST_CASE( test_is_callable )
{
	static_assert(is_callable<CustomCallable>{},
			"type is defined to be callable");
	static_assert(is_callable<CustomCallableArg>{},
			"this type was made to be callable");

	static_assert(!is_callable<int>{},
			"int is very much not callable");

	static_assert(!is_passive<CustomCallableArg>{},
			"CusomtCallableArg returns bool and thus cannot be passive connectable");

	static_assert(!is_callable<not_callable>{},
			"type is defined to be not callable");
}

namespace
{
struct accepting_registration
{
	void register_callback(const std::shared_ptr<std::function<void(size_t)>>&)
	{
	}
};

struct not_accepting_registration
{
};
}

BOOST_AUTO_TEST_CASE( test_has_register_function )
{
	static_assert(has_register_function<accepting_registration>(0),
			"type is defined with ability to register a callback");
	static_assert(!has_register_function<not_accepting_registration>(0),
				"type is defined without ability to register a callback");
}

BOOST_AUTO_TEST_SUITE_END()

// TestTraits.cpp ends here
