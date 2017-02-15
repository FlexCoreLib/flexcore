#include <boost/test/unit_test.hpp>
#include <flexcore/utils/settings/settings.hpp>
#include <flexcore/utils/settings/settings_backend.hpp>

BOOST_AUTO_TEST_SUITE(test_setting_registry)

BOOST_AUTO_TEST_CASE(test_int_setting)
{
	fc::settings_backend backend{};
	fc::settings_facade facade{backend};

	int default_value = 0;
	fc::setting<int> my_setting =
			{fc::setting_id{"setting_id"},facade, default_value};

	BOOST_CHECK_EQUAL(my_setting(), default_value);


	const std::string serialized{"{\"test_int\": 1" "}"};

	backend.write(fc::setting_id{"setting_id"}, serialized);

	BOOST_CHECK_EQUAL(my_setting(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
