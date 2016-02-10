#ifndef SRC_SETTINGS_SETTINGS_HPP_
#define SRC_SETTINGS_SETTINGS_HPP_

namespace fc
{

struct setting_id
{
	/// identifier of setting in context (for example key in ini file)
	std::string key;
};

/// minimal setting facade, which only provides the initival value, useful for tests.
class const_setting_backend_facade
{
public:
	template<class data_t, class setter_t>
	void register_setting(setting_id /*id*/, data_t initial_v, setter_t setter) //todo add constraint
	{
		// since this setting never changes,
		// we just call the setter with the initial value and be done.
		// id can be ignored
		setter(initial_v);
	}

	template<class data_t, class setter_t, class region_t>
	void register_setting(setting_id /*id*/, data_t initial_v, setter_t setter, region_t& /*region*/) //todo add constraint
	{
		// since this setting never changes,
		// we just call the setter with the initial value and be done.
		// We can also ignore the region and id in this case.
		setter(initial_v);
	}
};

/**
 * \brief Provides access to values which can be configured by the user.
 *
 * \tparam data_t type of data provided by setting.
 * Needs to be serializable by chosen serialization framework.
 *
 * \tparam backend_facade type of backend used to store and retrieve values.
 * Usually access to serialization framework.
 *
 * \invariant will always contain valid state of data_t.
 * The Constructor will fail and throw an exception if value cannot be loaded.
 */
template<class data_t, class backend_facade>
class setting
{
public:
	setting(setting_id id, backend_facade& backend, data_t initial_value)
		: cache(initial_value)
	{
		backend.register_setting(
				id, //unique id of setting in registry
				initial_value, //initial value, in case it needs to be stored
				[this](data_t i){ cache = i; }); //callback to let registry write cache
	}

	setting(setting_id id, data_t initial_value)
		: cache(initial_value)
	{
		backend_facade{}.register_setting(
				id, //unique id of setting in registry
				initial_value, //initial value, in case it needs to be stored
				[this](data_t i){ cache = i; }); //callback to let registry write cache
	}

	data_t operator()()
	{
		return cache;
	}

private:
	data_t cache;
};

} // namesapce fc

#endif /* SRC_SETTINGS_SETTINGS_HPP_ */
