/*
 * settings_container.hpp
 *
 *  Created on: Mar 31, 2016
 *      Author: jschwan
 */

#ifndef SRC_UTIL_SETTINGS_CONTAINER_HPP_
#define SRC_UTIL_SETTINGS_CONTAINER_HPP_

#include <flexcore/utils/generic_container.hpp>
#include <flexcore/utils/settings/settings.hpp>

namespace fc
{

/**
 * \brief Container Handling Ownership of Settings.
 *
 * Allows adding new settings which are then connected to backend.
 * Has non-owning reference to settingsbackend.
 */
template <class backend_t>
class settings_container
{
public:
	explicit settings_container(backend_t& backend) :
			backend_access(backend)
	{
	}

	template <class data_t>
	fc::setting<data_t>& add(setting_id name, data_t init_value)
	{
		return container.add<fc::setting<data_t>>(
				std::move(name),
				backend_access,
				std::move(init_value));
	}

private:
	generic_container container;
	backend_t& backend_access;
};

}

#endif /* SRC_UTIL_SETTINGS_CONTAINER_HPP_ */
