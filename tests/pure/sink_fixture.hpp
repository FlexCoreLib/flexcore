/*
 * sink_fixture.hpp
 *
 *  Created on: Mar 18, 2017
 *      Author: ckielwein
 */

#ifndef TESTS_PURE_SINK_FIXTURE_HPP_
#define TESTS_PURE_SINK_FIXTURE_HPP_

#include <vector>

namespace fc
{
namespace pure
{

/**
 * \brief automatically checks if certain events where received
 * \tparam T type of token accepted by sink_fixture
 */
template<class T>
class sink_fixture
{
public:
	///Initialize sink_fixture with a list of expected values
	explicit sink_fixture(std::vector<T> expected_values = std::vector<T>{})
		: expected{std::move(expected_values)}
	{
	}

	///Initialize sink_fixture with a list of expected values
	explicit sink_fixture(std::initializer_list<T> expected_values)
		: expected{expected_values}
	{
	}

	///push another value to back of the expected values
	void expect(T t)
	{
		expected.push_back(t);
	}

	///write a value to the list of received values
	void operator()(T token)
	{
		received.push_back(std::move(token));
	}

	~sink_fixture()
	{
		BOOST_CHECK_EQUAL_COLLECTIONS(
				received.begin(), received.end(),
				expected.begin(), expected.end());
	}

private:
	std::vector<T> expected;
	std::vector<T> received{};
};

} //namespace pure
} //namespace fc

#endif /* TESTS_PURE_SINK_FIXTURE_HPP_ */
