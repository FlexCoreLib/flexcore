#include <boost/test/unit_test.hpp>
#include <flexcore/utils/settings/settings.hpp>
#include <flexcore/utils/settings/jsonfile_setting_backend.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_settings)

BOOST_AUTO_TEST_CASE(test_trivial_setting)
{
	int default_value = 0;

	setting<int, const_setting_backend_facade> my_setting =
			{setting_id{"setting_id"}, default_value};

	BOOST_CHECK_EQUAL(my_setting(), default_value);
}

BOOST_AUTO_TEST_CASE(test_setting_json_file_good)
{
	std::stringstream ss;
	ss << "{ "
			"\"test_int\": 1,"
			"\"test_float\": 0.5 "
			"}";

	BOOST_CHECK_NO_THROW(json_file_setting_facade backend{ss};);
}

BOOST_AUTO_TEST_CASE(test_setting_json_file_bad)
{
	std::stringstream ss;
	ss << "{ "
			"\"test_int\": 1,"
			"\"test_float\": \"incompatible value"
			"}";

	BOOST_CHECK_THROW(json_file_setting_facade backend{ss}, cereal::Exception);
}

BOOST_AUTO_TEST_CASE(test_setting_from_json_file)
{
	int default_value = 0;

	std::stringstream ss;
	ss << "{ "
			"\"test_int\": 1,"
			"\"test_float\": 0.5 "
			"}";

	json_file_setting_facade backend{ss};

	setting<int, json_file_setting_facade> int_setting =
			{setting_id{"test_int"}, backend, default_value};

	//expect value from stream and not default value, since stream was loaded.
	BOOST_CHECK_EQUAL(int_setting(), 1);

	setting<float, json_file_setting_facade> float_setting =
			{setting_id{"test_float"}, backend, 0.0};
	BOOST_CHECK_EQUAL(float_setting(), 0.5);
}

BOOST_AUTO_TEST_SUITE_END()
