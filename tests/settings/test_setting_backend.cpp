#include <boost/test/unit_test.hpp>
#include <flexcore/utils/settings/settings.hpp>
#include <flexcore/utils/settings/settings_backend.hpp>

BOOST_AUTO_TEST_SUITE(test_setting_registry)

/// Example of using a setting with the backend
BOOST_AUTO_TEST_CASE(test_backend_example)
{
	fc::settings_backend backend{};

	//the facade holds a reference to the backend
	fc::settings_facade facade{backend};

	const int default_value = 0;

	//Lets create a setting containing an int with our facade and the id "setting_id".
	fc::setting<int> my_setting =
			{fc::setting_id{"setting_id"},facade, default_value};

	BOOST_CHECK_EQUAL(my_setting(), default_value);

	//a new value for our setting serialized as json
	const std::string serialized{"{\"test_int\": 1" "}"};

	//This call simulates an external source, which writes the setting
	backend.write(fc::setting_id{"setting_id"}, serialized);

	//the value of the setting has changed.
	BOOST_CHECK_EQUAL(my_setting(), 1);
}

BOOST_AUTO_TEST_CASE(test_backend)
{
	const float default_value{0};
	float test_val{default_value};

	fc::settings_backend backend{};

	//we just use a lambda to mock the setting callback
	backend.register_setting<float>(fc::setting_id{"setting_id"},
			[&](float val){ test_val  = val;});

	BOOST_CHECK_EQUAL(test_val, default_value);

	const std::string serialized{"{\"test_int\": 1.5" "}"};
	backend.write(fc::setting_id{"setting_id"}, serialized);
	BOOST_CHECK_EQUAL(test_val, 1.5);
}

BOOST_AUTO_TEST_CASE(test_constraints)
{
	fc::settings_backend backend{};
	fc::settings_facade facade{backend};

	const int default_value = 0;

	fc::setting<int> my_setting =
			{fc::setting_id{"setting_id"},
					facade,
					default_value,
					[](auto in){ return in >= 0;} };

	BOOST_CHECK_EQUAL(my_setting(), default_value);

	const std::string valid_string{"{\"test_int\": 1" "}"};

	backend.write(fc::setting_id{"setting_id"}, valid_string);
	BOOST_CHECK_EQUAL(my_setting(), 1);

	const std::string invalid_string{"{\"test_int\": -1" "}"};

	 BOOST_CHECK_THROW(backend.write(fc::setting_id{"setting_id"}, invalid_string),
			fc::setting_constraint_violation);
	BOOST_CHECK_EQUAL(my_setting(), 1);
}

BOOST_AUTO_TEST_CASE(test_deserialisation_failure)
{
	fc::settings_backend backend{};
	fc::settings_facade facade{backend};

	fc::setting<int> my_setting =
			{fc::setting_id{"setting_id"},
					facade,
					0,
					[](auto in){ return in >= 0;} };

	const std::string not_json{"arglblarghl"};

	BOOST_CHECK_THROW(backend.write(fc::setting_id{"setting_id"}, not_json),
			cereal::Exception);
	BOOST_CHECK_EQUAL(my_setting(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
