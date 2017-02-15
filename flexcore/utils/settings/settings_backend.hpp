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

/**
 * \brief Manages external access to settings
 */
class settings_backend
{
public:
	template<class T>
	using deserializer = fc::single_object_deserializer<T, cereal::JSONInputArchive>;

	settings_backend() = default;

	template<class data_t, class T>
	auto make_setting_model(T setting)
	{
		return std::make_unique<
				detail::setting_model<data_t, deserializer, T>>(
						std::move(setting));
	}

	void write(const setting_id& id, const std::string& val)
	{
		settings.at(id)->write(val);
	}

	template<class data_t, class Calllback>
	void register_setting(setting_id id, Calllback setting)
	{
		//todo what to do on duplicate settings? error?
		settings.emplace(
				std::move(id),
				make_setting_model<data_t>(std::move(setting))
				);
	}

private:
	std::map<setting_id, std::unique_ptr<detail::serialized_setting>>
			settings{};
};

class settings_facade
{
public:
	explicit settings_facade(settings_backend& b)
		: backend(b)
	{
	}

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
			//todo error handling
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

		auto set_value = [constraint, setter](const data_t& v)
		{
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
