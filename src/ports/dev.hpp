
#ifndef SRC_PORTS_DEV_HPP_
#define SRC_PORTS_DEV_HPP_

#include <memory>
#include <type_traits>
#include <functional>
#include <iostream>

#include <core/connection.hpp>

template<class data_t>
class state
{
public:
	state(data_t d_) : d(d_){}

	data_t operator()() { return d; }

	void set(data_t d_) { d = d_; }

private:
	data_t d;
};

template<class data_t>
class fetcher
{
public:
	fetcher()
		: con(new std::function<data_t()>())
	{ }
	fetcher(const fetcher& other) : con(other.con) {  }

	data_t get() { return (*con)(); }

	template<class con_t>
	void connect(con_t c)
	{
		(*con) = c;
	}

private:
	std::shared_ptr<std::function<data_t()>> con;
};

template<>
struct is_sink_port<fetcher<int> > : public std::true_type
{
};

#endif /* SRC_PORTS_DEV_HPP_ */
