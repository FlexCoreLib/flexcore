#include <boost/test/unit_test.hpp>

#include <nodes/list_manipulation.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE( test_list_manipulation )

BOOST_AUTO_TEST_CASE( test_list_splitter )
{
	typedef list_splitter
		<	std::vector<std::string>,
			size_t
		> splitter_t;

	splitter_t splitter([](const std::string& s) { return s.size(); });

	std::vector<std::vector<std::string>> output(5);
	for (size_t i = 0; i < 5; ++i)
		splitter.out(i) >> [&](const auto& v)
				{
					output.at(i).insert(output.begin(), boost::begin(v), boost::end(v));
				};
}

BOOST_AUTO_TEST_SUITE_END()

