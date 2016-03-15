/*
 * port_utils.hpp
 *
 *  Created on: Feb 15, 2016
 *      Author: jpiotrowski
 */
#ifndef SRC_PORTS_PORT_UTILS_HPP_
#define SRC_PORTS_PORT_UTILS_HPP_

#include <functional>
#include <algorithm>
#include <vector>
#include <iterator>

namespace fc
{
namespace detail
{

// std::function requires that the stored object be copyable. Once we arrive at the active port's
// connect member function, the argument is either an rvalue copyable object or an lvalue
// potentially move only type. So for lvalues it is necessary to construct a copyable wrapper
// (forwarding lambda or std::ref will do).
template <class conn_t>
auto handler_wrapper(conn_t& c)
{
	return std::ref(c);
}
template <class conn_t, class enable = std::enable_if_t<std::is_rvalue_reference<conn_t&&>{}>>
decltype(auto) handler_wrapper(conn_t&& c)
{
	return std::move(c);
}

template <class handler_t>
class single_handler_policy
{
public:
	single_handler_policy(handler_t* handler) : handler(handler)
	{
		assert(handler);
	}
	void add_handler(size_t hash) { handler_hash = hash; }
	void remove_handler(size_t hash)
	{
		// TODO: investigate this test_case
		//
		//     state_sink sink;
		//     state_source source1, source2;
		//     source1 >> sink;
		//     source2 >> sink;
		//
		// When the destructor of source1 runs, it will attempt to remove the connection, but the
		// connection has already been broken when source2 was connected. That's why this is a check
		// and not an assert.
		if (hash == handler_hash)
			*handler = {};
	}

private:
	handler_t* handler;
	size_t handler_hash;
};

template <class handler_t>
class multiple_handler_policy
{
public:
	multiple_handler_policy(std::vector<handler_t>* handlers) : handlers(handlers)
	{
		assert(handlers);
	}
	void add_handler(size_t hash) { handler_hashes.push_back(hash); }
	void remove_handler(size_t hash)
	{
		auto handler_position = find(begin(handler_hashes), end(handler_hashes), hash);
		assert(handler_position != end(handler_hashes));
		auto idx = distance(begin(handler_hashes), handler_position);
		handlers->erase(begin(*handlers) + idx);
		handler_hashes.erase(begin(handler_hashes) + idx);
	}

private:
	std::vector<handler_t>* handlers;
	std::vector<size_t> handler_hashes;
};

template <class handler_t, template <class> class handler_storage_policy>
class connection_breaker : handler_storage_policy<handler_t>
{
public:
	template <class handler_storage>
	connection_breaker(handler_storage& handlers)
	    : handler_storage_policy<handler_t>(&handlers),
	      sink_callback(std::make_shared<std::function<void(size_t)>>([this](size_t hash)
	                                                                  {
		                                                                  this->remove_handler(
		                                                                      hash);
		                                                              }))
	{
	}

	template <class sink_t>
	void add_circuit_breaker(sink_t& sink, std::true_type)
	{
		this->add_handler(std::hash<sink_t*>{}(&sink));
		sink.register_callback(sink_callback);
	}
	template <class sink_t>
	void add_circuit_breaker(sink_t&, std::false_type) {}

	void remove_handler(size_t hash) { handler_storage_policy<handler_t>::remove_handler(hash); }

private:
	// Callback from connected sinks to *this that delete the connection corresponding to a hash
	// when invoked.
	std::shared_ptr<std::function<void(size_t)>> sink_callback;
};

} //namespace detail
} //namespace fc

#endif /* SRC_PORTS_PORT_UTILS_HPP_ */
