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

class settings_container
{
public:

	template <class data_t, class backed_t>
	fc::setting<data_t, backed_t>& add(const std::string& name, backed_t backend, data_t value)
	{
		return container.add<fc::setting<data_t, backed_t>>(setting_id{name}, backend, value);
	}

private:
	generic_container container;
};

}

#endif /* SRC_UTIL_SETTINGS_CONTAINER_HPP_ */
