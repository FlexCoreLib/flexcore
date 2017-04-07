#include <boost/test/unit_test.hpp>
#include <flexcore/utils/serialisation/serializer.hpp>
#include <flexcore/utils/serialisation/deserializer.hpp>
#include <flexcore/core/connection.hpp>

#include <boost/mpl/list.hpp>

#include <limits>
#include <vector>
#include <iostream>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/vector.hpp>

BOOST_AUTO_TEST_SUITE(test_serialization)

using namespace fc;

namespace
{
template <class archive_in_t, class archive_out_t, class data_t>
static void round_trip_test(data_t to_send)
{
	single_object_serializer<data_t, archive_out_t> serializer;
	single_object_deserializer<data_t, archive_in_t> deserializer;

	auto round_trip = serializer >> deserializer;

	BOOST_CHECK(round_trip(to_send) == to_send);
}
}

BOOST_AUTO_TEST_CASE(test_round_trip)
{
	round_trip_test<cereal::BinaryInputArchive, cereal::BinaryOutputArchive>(0.);
	round_trip_test<cereal::BinaryInputArchive, cereal::BinaryOutputArchive>(std::numeric_limits<float>::max());
}

BOOST_AUTO_TEST_CASE(test_stl_type)
{
	single_object_serializer<std::vector<double>, cereal::JSONOutputArchive> serializer;
	single_object_deserializer<std::vector<double>, cereal::JSONInputArchive> deserializer;

	auto round_trip = serializer >> deserializer;

	std::vector<double> test_vec = {
			0.0, 1.1, 2.2, 3.3
	};

	auto result = round_trip(test_vec);

	for(size_t i = 0; i < test_vec.size(); ++i)
	{
		BOOST_CHECK_CLOSE(test_vec[i], result[i], 1e-8); // json produces a small error usign floats
	}

	round_trip_test<cereal::BinaryInputArchive, cereal::BinaryOutputArchive>(test_vec);
	round_trip_test<cereal::XMLInputArchive, cereal::XMLOutputArchive>(test_vec);
}

namespace
{
class class_with_internal_serialize
{
public:
	class_with_internal_serialize() :
		x(0)
		, y(0)
		, z(0)
	{
	}

	class_with_internal_serialize(int px, int py, int pz) :
		x(px)
		, y(py)
		, z(pz)
	{
	}

	// comparison operator only needed for test
	inline bool operator==(class_with_internal_serialize &other)
	{
		return x == other.x && y == other.y && z == other.z;
	}

	// This function is sufficient for serializing and deserializing
	// Use split save/load when handling const objects is necessary
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive( x, y, z);
	}
private:
	int x, y, z;
};

struct struct_with_external_serialize
{
	int x, y, z;

	// comparison operator only needed for test
	inline bool operator==(struct_with_external_serialize &other)
	{
		return x == other.x && y == other.y && z == other.z;
	}
};
template <class Archive>
void serialize(Archive& archive, struct_with_external_serialize& s)
{
	archive(s.x, s.y, s.z);
}

struct struct_with_split_save_load
{
	int x, y, z;

	// comparison operator only needed for test
	inline bool operator==(struct_with_split_save_load &other)
	{
		return x == other.x && y == other.y && z == other.z;
	}

	// needed when const object must be serialized
	template<class Archive>
	void save(Archive & archive) const
	{
		archive(x, y, z);
	}

	template<class Archive>
	void load(Archive & archive)
	{
		archive(x, y, z);
	}
};

using archive_types = boost::mpl::list<class_with_internal_serialize,
		struct_with_external_serialize,
		struct_with_split_save_load>;

} //anon namespace


BOOST_AUTO_TEST_CASE_TEMPLATE(test_archive_types, T, archive_types)
{
	round_trip_test<cereal::BinaryInputArchive, cereal::BinaryOutputArchive>(T{1, 2, 3});
	round_trip_test<cereal::JSONInputArchive, cereal::JSONOutputArchive>(T{1, 2, 3});
	round_trip_test<cereal::XMLInputArchive, cereal::XMLOutputArchive>(T{1, 2, 3});
}

BOOST_AUTO_TEST_SUITE_END()
