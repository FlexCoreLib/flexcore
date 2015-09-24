/*
 * clock.hpp
 *
 *  Created on: Sep 21, 2015
 *      Author: ckielwein
 */

#include "clock.hpp"

namespace fc
{

namespace chrono
{

namespace chr = std::chrono;

std::atomic<virtual_clock::system::time_point>
		virtual_clock::system::current_time(virtual_clock::system::time_point::min());
std::atomic<virtual_clock::steady::time_point>
		virtual_clock::steady::current_time(virtual_clock::steady::time_point::min());

virtual_clock::system::time_point virtual_clock::system::now() noexcept
{
	return current_time.load();
}

std::time_t virtual_clock::system::to_time_t(const time_point& t)
{
	const auto duration = t.time_since_epoch();
	const auto seconds = chr::duration_cast<chr::seconds>(duration);
	return seconds.count();
}

virtual_clock::system::time_point virtual_clock::system::from_time_t(std::time_t t)
{
	typedef std::chrono::time_point <virtual_clock::system,
			std::chrono::seconds>	from_t;

	auto tmp= from_t(std::chrono::seconds(t));
	auto result = std::chrono::time_point_cast<virtual_clock::duration>(tmp);
	return result;
}

void virtual_clock::system::advance() noexcept
{
	const auto tmp = current_time.load();
	current_time.store(tmp + tmp.min().time_since_epoch());
}

void virtual_clock::system::set_time(time_point r) noexcept
{
	current_time.store(r);
}

virtual_clock::steady::time_point virtual_clock::steady::now() noexcept
{
	return current_time.load();
}

void virtual_clock::steady::advance() noexcept
{
	const auto tmp = current_time.load();
	current_time.store(tmp + tmp.min().time_since_epoch());
}

} //namespace chrono
}  //namespace fc
