/*
 * settings_container.hpp
 *
 *  Created on: Mar 31, 2016
 *      Author: jschwan
 */

#ifndef SRC_UTIL_SETTINGS_CONTAINER_HPP_
#define SRC_UTIL_SETTINGS_CONTAINER_HPP_

#include <util/generic_container.hpp>
#include <settings/settings.hpp>

namespace fc
{

template <class backend_t>
class settings_container
{
public:
	settings_container(backend_t& backend) :
		mBackend(backend)
	{
	}

	template <class data_t>
	fc::setting<data_t, backend_t>& add(const std::string& name, data_t value)
	{
		return container.add<fc::setting<data_t, backend_t>>(setting_id{name}, mBackend, value);
	}

private:
	generic_container container;
	backend_t& mBackend;
};

}

#endif /* SRC_UTIL_SETTINGS_CONTAINER_HPP_ */
