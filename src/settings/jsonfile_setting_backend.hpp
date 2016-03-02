#ifndef SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_
#define SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_

#include <settings/settings.hpp>
#include <cereal/archives/json.hpp>

namespace fc
{

class json_file_setting_facade
{
public:
	typedef cereal::JSONInputArchive json_archive;

	/**
	 * @brief Creates a setting facade that reads values from
	 * stream using json format.
	 *
	 * @param stream The input stream containing the data in json syntax.
	 * @throw ::cereal::Exception if the given @p stream cannot be parsed
	 * by json parser, e.g. if syntax is wrong.
	 */
	json_file_setting_facade(std::istream& stream)
		: archive(stream)
	{
	}

	template<class data_t, class setter_t>
	void register_setting(
			setting_id id,
			data_t initial_v,
			setter_t setter) //todo add constraint
	{
		auto value = initial_v;
		try
		{
			//tries to read value from json parser.
			//the value remains unchanged if an error occurs.
			archive(cereal::make_nvp(id.key, value));
		}
		catch(const cereal::Exception& ex)
		{
			std::cerr << "json_file_setting_facade.register_setting():"
					" '"	<< ex.what() << "'" << std::endl;
		}
		setter(value);
	}

	/**
	 * \brief registers Setting together with region
	 *
	 * Region can be ignored in this case,
	 * as parameters from json file don't change after loading.
	 */
	template<class data_t, class setter_t, class region_t>
	void register_setting(
			setting_id id,
			data_t initial_v,
			setter_t setter,
			region_t& /*region*/) //todo add constraint
	{
		register_setting(id, initial_v, setter);
	}

	json_archive archive;
};

}  // namespace fc

#endif /* SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_ */
