#ifndef SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_
#define SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_

#include <flexcore/utils/settings/settings.hpp>
#include <cereal/archives/json.hpp>
#include <boost/lexical_cast.hpp>

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
		try : archive(stream)
	{
	}
	catch (const ::cereal::Exception& ex)
	{
		throw ::cereal::Exception(std::string(ex.what())
				+ ". you should check for json syntax errors.");
	}

	/**
	 * \brief Registers Setting and immediately tries to read value from archive.
	 * @param id Key of Setting in json format.
	 * @param initial_v initial value of setting, here for completeness of interface,
	 * as value is read immediately from archive
	 * @param setter callback to write value from archive to setting.
	 * @tparam data_t type of data stored in setting.
	 * @throw ::cereal::Exception if json value under @p id cannot be converted to data_t.
	 * @pre initial value needs to fulfill constraint
	 */
	template<class data_t, class setter_t, class constraint_t>
	void register_setting(
			setting_id id,
			data_t initial_v,
			setter_t setter,
			constraint_t constraint)
	{
		assert(constraint(initial_v));
		auto value = initial_v;
		try
		{
			using std::to_string;
			archive(cereal::make_nvp(id.key, value));
			if (constraint(value))
				setter(value);
			else throw(std::runtime_error(
					"Value of Setting "
					+ id.key
					+ ": " + boost::lexical_cast<std::string>(value)
					+ " violated constraint"));
		}
		catch (const ::cereal::Exception& ex)
		{
			throw ::cereal::Exception(std::string(ex.what())
					+ ". Value of Setting "
					+ id.key
					+ " could not be read from json archive");
		}
	}

	/**
	 * \brief registers Setting together with region
	 *
	 * Region can be ignored in this case,
	 * as parameters from json file don't change after loading.
	 */
	template<class data_t, class setter_t, class region_t, class constraint_t>
	void register_setting(
			setting_id id,
			data_t initial_v,
			setter_t setter,
			region_t& /*region*/,
			constraint_t constraint)
	{
		register_setting(id, initial_v, setter, constraint);
	}

	json_archive archive;
};

}  // namespace fc

#endif /* SRC_SETTINGS_JSONFILE_SETTING_BACKEND_HPP_ */
