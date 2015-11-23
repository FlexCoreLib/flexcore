#include <boost/test/unit_test.hpp>

#include <3rdparty/range-v3/include/range/v3/all.hpp>

#include <core/connection.hpp>

using namespace fc;
using namespace ranges;

BOOST_AUTO_TEST_SUITE(test_rage)

BOOST_AUTO_TEST_CASE(test_algorithm)
{
    auto tmp = view::ints(1)
                       | view::transform([](int i){return i*i;})
    				   | view::transform([](int i){return i*2;});

    auto result = tmp | view::take(10);

    std::cout << result << "\n";

    std::vector<int> bla = {1, 2, 3, 4, 5} ;

   std::vector<int> temp2 = std::move(bla) | action::transform([](int a){return a+1;});

    for (auto a : bla)
    	std::cout << "bla: " << a << "\n";

    for (auto a : temp2)
        	std::cout << "temp2: " << a << "\n";



    //auto connection = [](){ return view::ints(1); }
    //		>> [](auto&& in ){ return view::transform([](int i){return i*i;})(in);};
//    		>> [](auto&& in ){ return view::take(10)(in); }
//    		>> [](auto&& in ){ std::cout << "range connection:\n" << in << "\n"; };

    //auto blaasd = connect([](){ return view::ints(1); },  [](auto&& in ){ return view::transform([](int i){return i*i;})(in);} );

    //connection();
  //  blaasd();
}

BOOST_AUTO_TEST_SUITE_END()
