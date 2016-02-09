#ifndef SRC_SETTINGS_SETTINGS_HPP_
#define SRC_SETTINGS_SETTINGS_HPP_

#include <cereal/archives/json.hpp>

namespace fc
{

class setting_backend_facade
{
	template<class data_t, class setter_t>
	void register_setting(data_t initial_v, setter_t setter); //todo add constraint

	template<class data_t, class setter_t>
	void register_setting(data_t initial_v, setter_t setter, region_info& region); //todo add constraint
};


struct setting_identifier
{

};

/**
 * \brief Provides access to values which can be configured by the user.
 *
 * \tparam data_t type of data provided by setting.
 * Needs to be serializable by chosen serialization framework.
 *
 * \tparam backend_t type of backend used to store and retrieve values.
 * Usually access to serialization framework.
 *
 * \invariant will always contain valid state of data_t.
 * The Constructor will fail and throw an exception if value cannot be loaded.
 */
template<class data_t, template<class> class backend_t>
class setting
{
public:
	setting(setting_identifier identifier)
		: backend_access(identifier)
	{
	}

	data_t operator()()
	{
		return backend_access.get_value();
	}

private:
	data_t cache;



	backend_t<data_t> backend_access;
};

/// Setting Backend using a json file.
template<class data_t>
class json_file
{
public:
	json_file(std::string file_name, std::string setting_name, data_t initial)
		: stored_value(initial)
		, name(setting_name)
		, archive(file_name) //todo properly open file
	{
		// try to load value, this will throw if value doesn't exist
		archive(cereal::make_nvp(name, stored_value));
	}

private:
	data_t stored_value;
	std::string name;
	cereal::JSONInputArchive archive; //access to json de-serialisation
};

} // namesapce fc

#endif /* SRC_SETTINGS_SETTINGS_HPP_ */
