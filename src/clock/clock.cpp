/*
 * clock.hpp
 *
 *  Created on: Sep 21, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CLOCK_CLOCK_HPP_
#define SRC_CLOCK_CLOCK_HPP_

#include "clock.hpp"

namespace fc
{

namespace chrono
{

virtual_clock::system::time_point virtual_clock::system::now()
{
	return current_time;
}

std::time_t virtual_clock::system::to_time_t(const time_point& t)
{
	//ToDo
}

virtual_clock::system::time_point virtual_clock::system::from_time_t(std::time_t t)
{
	//ToDo
}

void virtual_clock::system::advance()
{
	current_time += current_time.min();
}

void virtual_clock::system::set_time(time_point r)
{
	//ToDo
}

virtual_clock::steady::time_point virtual_clock::steady::now()
{
	return current_time;
}

std::time_t virtual_clock::steady::to_time_t(const time_point& t)
{
	//ToDo
}

virtual_clock::steady::time_point virtual_clock::steady::from_time_t(std::time_t t)
{
	//ToDo
}

void virtual_clock::steady::advance()
{
	current_time += current_time.min();
}

} //namespace chrono
}  //namespace fc

#endif /* SRC_CLOCK_CLOCK_HPP_ */
