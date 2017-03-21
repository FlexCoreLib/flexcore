#include <boost/test/unit_test.hpp>

#include <flexcore/utils/settings/settings_container.hpp>
#include <flexcore/utils/settings/jsonfile_setting_backend.hpp>
#include <flexcore/ports.hpp>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_generic_container)

BOOST_AUTO_TEST_CASE(test_storage)
{
	generic_container gc;
	pure::event_source<int> source;

	source
		>> gc.add<pure::event_sink<int>>([](int in){BOOST_CHECK_EQUAL(in, 3);});

	source.fire(3);
}

BOOST_AUTO_TEST_CASE(test_setting_container)
{
	int default_value = 0;

	std::stringstream ss;
	ss << "{ "
			"\"test_int\": 1,"
			"\"test_float\": 0.5 "
			"}";

	json_file_setting_facade backend{ss};

	settings_container<json_file_setting_facade> sc(backend);

	auto int_setting = sc.add<int>(setting_id{"test_int"}, default_value);

	//expect value from stream and not default value, since stream was loaded.
	BOOST_CHECK_EQUAL(int_setting(), 1);

	auto float_setting = sc.add<float>(setting_id{"test_float"}, 0.0);
	BOOST_CHECK_EQUAL(float_setting(), 0.5);
}

BOOST_AUTO_TEST_SUITE_END()
