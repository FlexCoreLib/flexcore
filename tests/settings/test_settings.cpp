#include <boost/test/unit_test.hpp>
#include <settings/settings.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_settings)

BOOST_AUTO_TEST_CASE(setting_registration)
{
	int default_value = 0;

	setting<int, const_setting_backend_facade> my_setting =
			{setting_identifier{"setting_id"}, default_value};
}

BOOST_AUTO_TEST_SUITE_END()
