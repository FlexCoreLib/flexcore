#ifndef SRC_SERIALISATION_DESERIALIZER_HPP_
#define SRC_SERIALISATION_DESERIALIZER_HPP_

namespace fc
{

template<class data_t, class archive_t>
class single_object_deserializer
{
public:
	using result_t = data_t;
	/**
	 * \brief Deserializeses input string to data_t
	 *
	 * \throws exception depending on archive used,
	 * if input cannot be serialized to data_t.
	 */
	data_t operator()(const std::string& serialized)
	{
		std::istringstream stream{serialized};
		data_t output;
		archive_t{stream}(output);
		return output;
	}
};

}

#endif /* SRC_SERIALISATION_DESERIALIZER_HPP_ */
