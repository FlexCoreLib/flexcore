/*
 * terminal.hpp
 *
 *  Created on: Apr 7, 2016
 *      Author: ckielwein
 */

#ifndef FLEXCORE_EXTENDED_NODES_TERMINAL_HPP_
#define FLEXCORE_EXTENDED_NODES_TERMINAL_HPP_

#include <flexcore/extended/base_node.hpp>

namespace fc
{

/**
 * \brief Node that relays state from input port to output port.
 *
 * A state_terminal is useful to form the border of external connections
 * and internal connections in a compound node which nests other nodes.
 *
 * \tparam T type of data transmitted through terminal node.
 * \tparam base_node base_node to either include or exclude this node from forest.
 * Instantiate terminal with either pure_node or tree_base_node.
 * \ingroup nodes
 */
template <class T, class base_node = tree_base_node>
class state_terminal : public base_node
{
public:
	/**
	 * \brief Constructor which forwards arguments to base_node
	 * \param args Arguments for Base node. In the default case this is:
	 * \code {std::shared_ptr<fc::parallel_region> r, const std::string& name}
	 */
	template<class... base_args>
	explicit state_terminal(base_args&&... args)
		: base_node(std::forward<base_args>(args)...)
		, in_state(this)
		, out_state(this, [this](){ return in_state.get(); })
	{
	}

	/// State sink of type T
	auto& in() { return in_state; }
	/// State source of type T
	auto& out() { return out_state; }

private:
	typename base_node::template state_sink<T> in_state;
	typename base_node::template state_source<T> out_state;
};

/**
 * \brief Node that relays events from input port to output port.
 *
 * An event_terminal is useful to form the border of external connections
 * and internal connections in a compound node which nests other nodes.
 *
 * \tparam T type of data transmitted through terminal node.
 * \tparam base_node base_node to either include or exclude this node from forest.
 * Instantiate terminal with either pure_node or tree_base_node.
 * \ingroup nodes
 */
template <class T, class base_node = tree_base_node>
class event_terminal : public base_node
{
public:
	/**
	 * \brief Constructor which forwards arguments to base_node
	 * \param args Arguments for Base node. In the default case this is:
	 * \code {std::shared_ptr<fc::parallel_region> r, const std::string& name}
	 */
	template<class... base_args>
	explicit event_terminal(base_args&&... args)
		: base_node(std::forward<base_args>(args)...)
		, in_event(this, [this](T in){ out_event.fire(std::move(in));})
		, out_event(this)
	{
	}

	/// Event sink of type T
	auto& in() { return in_event; }
	/// Event source of type T
	auto& out() { return out_event; }

private:
	typename base_node::template event_sink<T> in_event;
	typename base_node::template event_source<T> out_event;
};

}  // namespace fc

#endif /* FLEXCORE_EXTENDED_NODES_TERMINAL_HPP_ */
