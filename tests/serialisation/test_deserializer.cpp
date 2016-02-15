#include <boost/test/unit_test.hpp>
#include <serialisation/serializer.hpp>
#include <serialisation/deserializer.hpp>

#include <core/connection.hpp>

#include <cereal/archives/binary.hpp>
#include <limits>

using namespace fc;

BOOST_AUTO_TEST_SUITE(test_serialization)

BOOST_AUTO_TEST_CASE(test_round_trip)
{
	single_object_serializer<float, cereal::BinaryOutputArchive> serializer;
	single_object_deserializer<float, cereal::BinaryInputArchive> deserializer;

	auto round_trip = connect(serializer, deserializer);

	BOOST_CHECK_EQUAL(round_trip(0.), 0.);
	BOOST_CHECK_EQUAL(round_trip(std::numeric_limits<float>::max()),
			std::numeric_limits<float>::max());

}

BOOST_AUTO_TEST_SUITE_END()
