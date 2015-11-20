#include <boost/test/unit_test.hpp>

#include <nodes/list_manipulation.hpp>

// std
#include <deque>
#include <list>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_list_manipulation )

template<class BLA>
struct helper_sink
{
	typedef void result_t;
	void operator()(const auto& v)
	{
		bla(v);
	}
	BLA bla;
};

template<class FOO>
auto helper_helper(FOO foo)
{
	return helper_sink<FOO>{foo};
}

BOOST_AUTO_TEST_CASE( test_list_splitter )
{
	typedef list_splitter <std::list<std::string>, size_t> splitter_t;
	auto predicate = [](const std::string& s) { return s.size(); };
	splitter_t splitter(predicate);

	std::vector<std::vector<std::string>> output(5);
	typedef decltype(splitter.out(0))::result_t out_range_t;

	for (size_t i = 0; i < 5; ++i)
		splitter.out(i) >> [&output, i](const out_range_t& v) // [&output, i](const auto& v)
				{ output.at(i) = std::vector<std::string>(boost::begin(v), boost::end(v)); };

	// send data
	std::list<std::string> input { "aa", "bbb", "c", "d" };
	splitter.in(input);

	// check result
	BOOST_CHECK((output.at(0) == std::vector<std::string>{ }));
	BOOST_CHECK((output.at(1) == std::vector<std::string>{"c", "d"}));
	BOOST_CHECK((output.at(2) == std::vector<std::string>{"aa"}));
	BOOST_CHECK((output.at(3) == std::vector<std::string>{"bbb"}));
	BOOST_CHECK((output.at(4) == std::vector<std::string>{ }));
}

/*
 * test for predicate_result == bool
 */
BOOST_AUTO_TEST_CASE( test_list_splitter_bool )
{
	typedef list_splitter <std::list<int>, bool> splitter_t;
	auto predicate = [](int v) { return v >= 0; };
	splitter_t splitter(predicate);

	std::vector<int> out_true;
	std::vector<int> out_false;
	typedef decltype(splitter.out(0))::result_t out_range_t;

	splitter.out(true) >> [&out_true](const out_range_t& v)
			{ out_true = std::vector<int>(boost::begin(v), boost::end(v)); };
	splitter.out(false) >> [&out_false](const out_range_t& v)
			{ out_false = std::vector<int>(boost::begin(v), boost::end(v)); };

	// send data
	std::list<int> input { 1, -1, 5, -6, -6, 1 };
	splitter.in(input);

	BOOST_CHECK((out_true  == std::vector<int>{ 1, 5, 1 }));
	BOOST_CHECK((out_false == std::vector<int>{ -1, -6, -6}));
}

BOOST_AUTO_TEST_CASE( simple )
{
	event_out_port<int> p;
	p >> [](const auto&){std::cout <<"BAR" << std::endl;};
	p.fire(4);
}

BOOST_AUTO_TEST_SUITE_END()

