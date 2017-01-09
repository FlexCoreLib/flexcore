Serialization
=============

Flexcore uses the [cereal](http://uscilab.github.io/cereal/) serialization
library. The `single_object_serializer<data_t, archive_t>` and
`single_object_deserializer<data_t, archive_t>` classes are [connectable](CoreConcept#connectable)
wrappers to serialize data from and to flexcore ports.

Both wrappers overload the function call operator as demanded by the _connectable_ concept. The serializer takes an object of `data_t` and sends a `std::string` containing either plain text or binary data, depending on `archive_t`. The deserializer works accordingly.

Data Types
----------

The template parameter `data_t` can by default be any primitive and most STL
types.

For STL types the respective cereal header must be included, e.g. `std::vector`
requires an include of `cereal/types/vector.hpp`. Additional headers can be
found [here][cereal-stl-doc].

For own classes, the default constructor must exist and serialization methods
must be implemented, which can either be a single method named `serialize`,
e.g.

```cpp
template <class Archive>
void class_name::serialize(Archive & archive)
{
	archive(member1, member2, ...);
}

```

or split `save` and `load` methods, e.g.

```cpp
template<class Archive>
void class_name::save(Archive & archive) const
{
	archive(member1, member2, ...);
}

template<class Archive>
void class_name::load(Archive & archive)
{
	archive(member1, member2, ...);
}
```

The save method must be declared const, otherwise the code will not compile.

Non-member serialize or save/load methods may also be provided, e.g.
```cpp
template <class Archive>
void serialize(Archive& archive, some_class& s)
{
	archive(s.member1, s.member2, ...);
}
```

More information on this topic can be found on the [cereal website][cereal-doc].

Archive Types
-------------

The template parameter `archive_t` can be any of the three cereal archive
types. Take care to provide the serializer with an output archive and the
deserializer with an input archive.

Cereal provides serialization in binary, json and xml form. The archives are
located in the `cereal` namespace and the respective includes are:

| Name                                    | Header                     |
| :-------------------------------------: | :------------------------: |
| BinaryInputArchive, BinaryOutputArchive | cereal/archives/binary.hpp |
| JSONInputArchive, JSONOutputArchive     | cereal/archives/json.hpp   |
| XMLInputArchive, XMLOutputArchive       | cereal/archives/xml.hpp    |

Example
-------

Binary serialization of a `std::vector`:
```cpp
#include <core/connection.hpp>
#include <serialisation/serializer.hpp>
#include <serialisation/deserializer.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <vector>

using namespace fc;

bool serialize_something()
{
	single_object_serializer<std::vector, cereal::BinaryOutputArchive> serializer;
	single_object_deserializer<std::vector, cereal::BinaryInputArchive> deserializer;

	auto round_trip = serializer >> deserializer;

	std::vector<double> my_vec = {
			0.0, 1.1, 2.2, 3.3
	};

	return round_trip(my_vec) == my_vec; // returns success
}

```

[cereal-stl-doc]: http://uscilab.github.io/cereal/assets/doxygen/group__STLSupport.html
[cereal-doc]: http://uscilab.github.io/cereal/serialization_functions.html

