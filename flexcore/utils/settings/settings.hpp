#ifndef SRC_SETTINGS_SETTINGS_HPP_
#define SRC_SETTINGS_SETTINGS_HPP_

#include <string>

namespace fc
{

/// identifier of setting in context (for example key in ini file)
struct setting_id
{
	explicit setting_id(std::string id)
		: key{ std::move(id) }
	{

	}

	std::string key;
};

inline bool operator<(const setting_id& l, const setting_id& r)
{
	return l.key < r.key;
}

inline bool operator==(const setting_id& l, const setting_id& r)
{
	return l.key == r.key;
}

/// Trivial constraint which is always valid
struct always_valid
{
	template<class T>
	bool operator()(const T&) const
	{
		return true;
	}
};

/// minimal setting facade, which only provides the initial value, useful for tests.
class const_setting_backend_facade
{
public:
	template<class data_t, class setter_t, class constraint_t>
	void register_setting(setting_id /*id*/,
			data_t initial_v,
			setter_t setter,
			constraint_t constraint)
	{
		assert(constraint(initial_v));
		(void)(constraint); //prevent unused variable warning with NDEBUG.
		// since this setting never changes,
		// we just call the setter with the initial value and be done.
		// id can be ignored
		setter(initial_v);
	}

	template<class data_t, class setter_t, class region_, class constraint_t>
	void register_setting(setting_id id,
			data_t initial_v,
			setter_t setter,
			region_& /*region*/,
			constraint_t constraint)
	{
		register_setting(id, initial_v, setter, constraint);
	}
};

/**
 * \brief Provides access to values which can be configured by the user.
 *
 * \tparam data_t type of data provided by setting.
 * Needs to be serializable by chosen serialization framework.
 *
 * \invariant will always contain valid state of data_t.
 * The Constructor will fail and throw an exception if value cannot be loaded.
 */
template<class data_t>
class setting
{
public:
	/**
	 * \brief Constructs Setting with id, reference to backend, initial value and optionally a constraint.
	 * \param id identifier of the setting
	 * \param backend Reference to the backend used by this setting
	 * \param initial_value Value the setting has on construction.
	 * Depending on the value stored in the backend, this might not be the value the setting has
	 * on first call to it.
	 * \param constraint any function object with signature \code{ bool(data_t) const } \endcode
	 * \tparam backend_facade type of backend used to store and retrieve values.
	 * Usually access to serialization framework.
	 * \pre initial_value needs to fulfill constraint
	 */
	template <class backend_facade, class constraint_t = always_valid>
	setting(setting_id id,
			backend_facade& backend,
			data_t initial_value,
			constraint_t constraint = constraint_t{})
		: cache(std::make_shared<data_t>(initial_value))
	{
		backend.register_setting(
				id, //unique id of setting in registry
				initial_value, //initial value, in case it needs to be stored
				[c = this->cache](data_t i){ *c = i; }, //callback to let registry write cache
				constraint);
	}

	/**
	 * \return Returns the setting's current value
	 * \post return value fulfills constraint given in constructor
	 */
	data_t operator()()
	{
		return *cache;
	}


private:
	//cache is a shared_ptr since references store a callback to set the cache.
	//Making the cache a shared_ptr allows users to move and copy the setting
	//without causing dangling references in the backend.
	std::shared_ptr<data_t> cache;
};

} // namesapce fc

#endif /* SRC_SETTINGS_SETTINGS_HPP_ */
