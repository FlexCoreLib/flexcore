/*
 * port_utils.hpp
 *
 *  Created on: Feb 15, 2016
 *      Author: jpiotrowski
 */
#ifndef SRC_PORTS_PORT_UTILS_HPP_
#define SRC_PORTS_PORT_UTILS_HPP_

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <vector>

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

/// Policy for circuit breaker where only one handler is connected at any given time.
template <class handler_t>
struct single_handler_policy
{
public:
	single_handler_policy() : handler_hash(0) {}
	single_handler_policy(single_handler_policy&& p)
	    : handler_hash(p.handler_hash)
	{
		// libc++ doesn't correctly handle move ctr of std::function, it just copies.
		// swap takes care of that.
		swap(handlers, p.handlers);
	}

	void add_handler(const handler_t& handler_, size_t hash)
	{
		handlers = handler_;
		handler_hash = hash;
	}
	void remove_handler(size_t hash)
	{
		// Consider this test_case:
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
			handlers = {};
	}

	handler_t handlers;
	size_t handler_hash;
};

/// Policy class for circuit breaker when multiple handlers can be connected at once.
template <class handler_t>
struct multiple_handler_policy
{
public:
	/// \pre The handler corresponding to hash has been pushed_back to handlers before this
	/// call.
	void add_handler(const handler_t& handler, size_t hash)
	{
		handlers.push_back(handler);
		handler_hashes.push_back(hash);
	}
	void remove_handler(size_t hash)
	{
		assert(!handler_hashes.empty());
		assert(!handlers.empty());

		auto handler_position = find(begin(handler_hashes), end(handler_hashes), hash);
		assert(handler_position != end(handler_hashes));
		auto idx = distance(begin(handler_hashes), handler_position);
		handlers.erase(begin(handlers) + idx);
		handler_hashes.erase(begin(handler_hashes) + idx);
	}

	std::vector<handler_t> handlers;
	std::vector<size_t> handler_hashes;
};

/** \brief Register callbacks with passive port.
 *
 * \tparam handler_t type of handler used by active port.
 * \tparam storage_policy policy class that handles the number of
 *         handlers used in active port.
 * \invariant callback != nullptr
 * \invariant *callback is always valid function object.
 */
template <class handler_t, template <class> class storage_policy>
struct active_port_base
{
public:
	active_port_base()
		: callback(std::make_shared<std::function<void(size_t)>>(
				[this](size_t hash)
				 {
					 storage.remove_handler(hash);
				 }))
	{
		assert(callback);
		assert(*callback);
	}
	active_port_base(active_port_base&& p)
	    : storage(std::move(p.storage)), callback(std::move(p.callback))
	{
		assert(callback);
		*callback = [this](size_t hash) { storage.remove_handler(hash); };
	}

	/** \brief Register a callback with sink, that breaks the connection to source.
	 * \pre sink_t supports registering callbacks.
	 *
	 * \param handler is used by the active side to store the connection.
	 * Needs to be compatible with what the policy expects.
	 */
	template <class sink_t, std::enable_if_t<fc::has_register_function<sink_t>(0), int> = 0>
	void add_handler(handler_t handler, sink_t& sink)
	{
		storage.add_handler(std::move(handler), std::hash<sink_t*>{}(&sink));
		sink.register_callback(callback);
	}
	/// Do-nothing when sink does not support registering callbacks.
	template <class sink_t, std::enable_if_t<!fc::has_register_function<sink_t>(0), int> = 0>
	void add_handler(handler_t handler, sink_t& sink)
	{
		storage.add_handler(std::move(handler), std::hash<sink_t*>{}(&sink));
	}

	storage_policy<handler_t> storage;
private:
	/// Callback from connected passive port to *this that deletes the connection when invoked.
	std::shared_ptr<std::function<void(size_t)>> callback;
};

} //namespace detail
} //namespace fc

#endif /* SRC_PORTS_PORT_UTILS_HPP_ */
