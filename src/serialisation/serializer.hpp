#ifndef SRC_SERIALISATION_SERIALIZER_HPP_
#define SRC_SERIALISATION_SERIALIZER_HPP_

namespace fc
{

/**
 * \brief Serializes inputs of given type to string.
 *
 * \tparam data_t type of data to serialize.
 * \tparam archive_t type of archive used for serialization.
 *
 * The serializer is only tested with Cereal archives.
 * Thus data_t needs to confirm to the cereal api.
 */
template<class data_t, class archive_t>
class single_object_serializer
{
public:
	using result_t = std::string;
	std::string operator()(const data_t& in)
	{
		std::ostringstream serialized_storage;
		{
			archive_t archive{serialized_storage};
			archive(in);
		}
		return serialized_storage.str();
	}
private:
};

}  // namespace fc
#endif /* SRC_SERIALISATION_SERIALIZER_HPP_ */
