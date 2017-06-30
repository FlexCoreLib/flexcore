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

/**
 * \brief  Wall clock for measurements of system time.
 * Timings in Solutions should pretty much always use virtual clock.
 */
struct wall_clock
{
	/// system_wall_clock is just a forward to std::system_clock
	using system = std::chrono::system_clock ;
	/// steady_wall_clock is just a forward to std::steady_clock
	using steady = std::chrono::steady_clock;
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
	using duration = std::chrono::nanoseconds ;
	using rep = duration::rep; ///<storage format of the time
	using period = duration::period; ///<duration of a tick == smallest duration possible

	/**
	 * \brief virtual clock for measuring time points in simulation time
	 *
	 * Fulfills trivial_clock from std.
	 */
	class system
	{
	public:
		static constexpr bool is_steady = false;

		using rep = virtual_clock::rep;
		using period = virtual_clock::period;
		using duration = virtual_clock::duration;
		using time_point = std::chrono::time_point<virtual_clock::system, duration>;
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

		using rep = virtual_clock::rep;
		using period = virtual_clock::period;
		using duration = virtual_clock::duration;
		using time_point = std::chrono::time_point<virtual_clock::steady, duration>;

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

/**
 * \brief controls the time of the two virtual clocks.
 *
 * \tparam period_t the period of a single tick. Is the smallest duration possible.
 */
template<class period_t>
class master_clock
{
public:
	using duration = std::chrono::duration<int64_t, period_t>;
	using rep = typename duration::rep; ///<storage format of the time
	using period = typename duration::period; ///<duration of a tick == smallest duration possible

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
