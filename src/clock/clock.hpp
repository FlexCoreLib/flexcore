/*
 * clock.hpp
 *
 *  Created on: Sep 21, 2015
 *      Author: ckielwein
 */

#ifndef SRC_CLOCK_CLOCK_HPP_
#define SRC_CLOCK_CLOCK_HPP_

#include <atomic>
#include <chrono>

namespace fc
{

//forward declaration for friend declartations in virtual_clock
template<class T> class master_clock;

struct wall_clock
{
	/// system_wall_clock is just a forward to std::system_clock
	typedef std::chrono::system_clock system;
	/// steady_wall_clock is just a forward to std::steady_clock
	typedef std::chrono::steady_clock steady;
};


/**
 * \brief virtual clock controls the time within the flexcore application
 *
 * The virtual clock is independent of the system real time clock.
 * It is used to determine timings in simulations and replays of logged data.
 * The clock itself is controlled by the scheduler of the application.
 */
struct virtual_clock
{
	/**
	 * \brief type determining the time discretization of the virtual clock
	 *
	 * currently the timing period is set to nanoseconds,
	 * although the actual step size is determined by the master clock.
	 * we set the base type of the duration to a fixed width integer
	 * to have the same value on all platforms.
	 */
	typedef std::chrono::nanoseconds duration;
	typedef duration::rep rep; ///<storage format of the time
	typedef duration::period period; ///<duration of a tick == smallest duration possible

	class system
	{
	public:
		static constexpr bool is_steady = false;

		typedef virtual_clock::rep rep;
		typedef virtual_clock::period period;
		typedef virtual_clock::duration duration;
		typedef std::chrono::time_point<virtual_clock::system, duration> time_point;
		/**
		 * \brief returns current absolute simulation time
		 * \return A time point representing the current virtual time.
		 */
		static time_point now() noexcept;
		static std::time_t to_time_t( const time_point& t );
		static time_point from_time_t( std::time_t t );

		template<class T>
		friend class master_clock;
	private:

		static void advance(duration d) noexcept;
		static void set_time(time_point r) noexcept;

		static std::atomic<time_point> current_time;
	};

	/**
	 * \brief steady virtual clock for measuring time differences in simulation time
	 *
	 * Fulfills trivial_clock from std.
	 */
	struct steady
	{
		static constexpr bool is_steady = true;

		typedef virtual_clock::rep rep;
		typedef virtual_clock::period period;
		typedef virtual_clock::duration duration;
		typedef std::chrono::time_point<virtual_clock::steady, duration> time_point;

		/**
		 * \brief returns current relative simulation time
		 * \return A time point representing the current virtual time.
		 * \post if now is called twice with results t1 and t2, t2 >= t1 holds.
		 */
		static time_point now() noexcept;

	private:
		template<class T>
		friend class master_clock;

		static void advance(duration d) noexcept;

		static std::atomic<time_point> current_time;
	};
};

template<class period_t>
class master_clock
{
public:
	typedef std::chrono::duration<int64_t, period_t> duration;
	typedef typename duration::rep rep; ///<storage format of the time
	typedef typename duration::period period; ///<duration of a tick == smallest duration possible

	/**
	 * \brief advances clock by a single tick
	 *
	 * this method controls the flow of time of the virtual clock.
	 * The Scheduler calls advance during runtime of the program to advance the time.
	 */
	static void advance() noexcept
	{
		steady_clock.advance(
				std::chrono::duration_cast<virtual_clock::steady::duration>
				(duration(1)));
		system_clock.advance(
				std::chrono::duration_cast<virtual_clock::system::duration>
				(duration(1)));
	}
	static void set_time(virtual_clock::system::time_point r) noexcept
	{
		system_clock.set_time(r);
		//do not set time of steady clock, as it has only relative timings.
	}

private:
	static virtual_clock::steady steady_clock;
	static virtual_clock::system system_clock;
};

}  //namespace fc

#endif /* SRC_CLOCK_CLOCK_HPP_ */
