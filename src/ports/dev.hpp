
#ifndef SRC_PORTS_DEV_HPP_
#define SRC_PORTS_DEV_HPP_

#include <memory>
#include <type_traits>
#include <functional>
#include <iostream>

#include <core/connection.hpp>

/**
 * Simple StreamSource implementation that holds a state.
 * Can be connected to any number of SinkConnections.
 * Is a SourceConnection.
 */
template<class data_t>
class state
{
public:
	state(data_t d_) : d(new data_t(d_)){}

	/// pull data
	data_t operator()() { return *d; }

	/// set current value
	void set(data_t d_) { *d = d_; }

private:
	std::shared_ptr<data_t> d;
};

/**
 * Simple implementation for StreamSink.
 * Will pull data when get is called.
 * Is not a Connectable.
 */
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
