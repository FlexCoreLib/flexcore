/*
 * settings_backend.hpp
 *
 *  Created on: Feb 15, 2017
 *      Author: ckielwein
 */

#ifndef FLEXCORE_UTILS_SETTINGS_SETTINGS_BACKEND_HPP_
#define FLEXCORE_UTILS_SETTINGS_SETTINGS_BACKEND_HPP_

#include <flexcore/utils/settings/settings.hpp>
#include <flexcore/utils/serialisation/deserializer.hpp>
#include <flexcore/pure/event_sinks.hpp>

#include <cereal/archives/json.hpp>

#include <map>
#include <cassert>
#include <exception>

namespace fc
{

namespace detail
{

///Type Erasure for settings setter
class serialized_setting
{
public:
	serialized_setting() = default;
	virtual ~serialized_setting() = default;
	virtual void write(const std::string& val) = 0;

	serialized_setting(const serialized_setting&) = delete;
	serialized_setting(serialized_setting&&) = delete;
};

template<class data_t, template<class>class Deserializer, class Setting>
class setting_model final: public serialized_setting
{
public:
	explicit setting_model(Setting s)
		: serialized_setting()
		, setting(std::move(s))
	{
	}

	void write(const std::string& val) override final
	{
		//write deserialized value to setting
		//todo constraint handling
		//todo handling of deserialization failures
		setting(Deserializer<data_t>{}(val));
	}
private:
	Setting setting;
};

/**
 * \brief stores new setting value on changed and forwards on switch tick
 *
 * \tparam data_t value stored in setting
 * \tparam callback_t callback from setting which sets new value
 */
template<class data_t, class callback_t>
class setting_forwarder
{
public:
	explicit setting_forwarder(callback_t c)
		: callback(std::move(c))
		, forward_port([this](){ forward_if_changed();})
	{
	}

	setting_forwarder(setting_forwarder&&) = delete;
	setting_forwarder(const setting_forwarder&) = delete;

	auto& in_forward() { return forward_port; }

	void operator()(data_t in)
	{
		buffer = in;
		changed = true;
	}

private:
	callback_t callback;
	bool changed = false;
	fc::pure::event_sink<void> forward_port;
	data_t buffer = data_t();

	void forward_if_changed()
	{
		if (changed)
		{
			callback(buffer);
			changed = false;
		}
	}
};

template<class data_t, class callback_t>
struct forward_holder
{
	explicit forward_holder(callback_t c)
		: internal{std::make_unique<setting_forwarder<data_t, callback_t>>(c)}
	{
	}

	void operator()(data_t in)
	{
		(*internal)(in);
	}

	fc::pure::event_sink<void>& in_forward()
	{
		return internal->in_forward();
	}

private:
	std::unique_ptr<setting_forwarder<data_t, callback_t>> internal;
};

} //namespace detail

struct setting_constraint_violation : std::runtime_error
{
	using std::runtime_error::runtime_error;
};

/**
 * \brief Manages external access to settings
 *
 * Holds a map of setting_id to callbacks, which write values to the settings.
 * When the settings_backend is informed of new values it deserializes them
 * and forwards the values to the setting with the correct id.
 *
 * settings_backend does not dynamically delete values in the map.
 */
class settings_backend
{
public:
	template<class T>
	using deserializer = fc::single_object_deserializer<T, cereal::JSONInputArchive>;

	settings_backend() = default;

	/**
	 * \brief Writes a serialized value to a setting identified by its id
	 * \param id Identifier of the Setting
	 * \param val serialized value to be written to setting
	 * \post the setting corresponding to id has a value, which does not violate it's constraint
	 * \throws std::out_of_range if no setting with id is registered
	 * \throws deserialisation failure if val cannot be deserialized to setting value.
	 * \throws setting_constraint_violation if deserialized value violates constraint of setting.
	 */
	void write(const setting_id& id, const std::string& val)
	{
		settings.at(id)->write(val);
	}

	/**
	 * \brief Registers a new Setting ad the setting_backend, called by setting_facade
	 * \param id Identifier of the setting
	 * \param setting Callback to set new values
	 * \pre no setting with identifier == id is registered
	 * \post setting is registered with id in the backend
	 */
	template<class data_t, class Calllback>
	void register_setting(setting_id id, Calllback setting)
	{
		assert(settings.count(id) == 0);
		settings.emplace(
				std::move(id),
				make_setting_model<data_t>(std::move(setting))
				);
	}

private:

	template<class data_t, class T>
	static auto make_setting_model(T setting)
	{
		return std::make_unique<
				detail::setting_model<data_t, deserializer, T>>(
						std::move(setting));
	}


	std::map<setting_id, std::unique_ptr<detail::serialized_setting>>
			settings{};
};

/**
 * \brief Default facade which connects any fc::setting to the settings_backend
 */
class settings_facade
{
public:
	/// Constructs a new facade which holds a reference to backend @p b.
	explicit settings_facade(settings_backend& b)
		: backend(b)
	{
	}

	/**
	 * \brief registeres a new setting at the facade
	 * \param id Identifier of the setting
	 * \param initial_v Initial Value of the setting
	 * \param setter Callback to set a new value in the setting
	 * \param constraint functor returning true for every valid value of the setting
	 * \pre constraint(initial_v) == true
	 */
	template<class data_t, class setter_t, class constraint_t>
	void register_setting(
			setting_id id,
			data_t initial_v,
			setter_t setter,
			constraint_t constraint)
	{
		assert(constraint(initial_v));
		setter(initial_v);

		auto set_value = [constraint, setter](const data_t& v)
		{
			if(constraint(v))
				setter(v);
			else
				throw setting_constraint_violation{
						"new setting value violated constraint"};
		};

		backend.register_setting<data_t>(std::move(id), set_value);
	}

	/**
	 * \brief registers Setting together with region.
	 * used by the ctor of fc::setting.
	 */
	template<class data_t, class setter_t, class region_t, class constraint_t>
	void register_setting(
			fc::setting_id id,
			data_t initial_v,
			setter_t setter,
			region_t& region,
			constraint_t constraint)
	{
		setter(initial_v);

		// this callback will be executed every time the value of the setting is changed
		auto set_value = [constraint, setter](const data_t& v)
		{
			//todo constraint needs to be checked immediately not delayed, when the setting receives the value
			if(constraint(v))
				setter(v);
		};

		detail::forward_holder<data_t,setter_t> forwarder{set_value};

		using fc::operator>>;
		region.switch_tick() >> forwarder.internal->in_forward();

		backend.register_setting<data_t>(std::move(id), std::move(forwarder));
	}

private:
	settings_backend& backend;
};


} //namespace fc

#endif /* FLEXCORE_UTILS_SETTINGS_SETTINGS_BACKEND_HPP_ */
