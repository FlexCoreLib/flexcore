#ifndef SRC_PORTS_PORTS_HPP_
#define SRC_PORTS_PORTS_HPP_

#include <ports/node_aware.hpp>
#include <ports/token_tags.hpp>
#include "pure_ports.hpp"

namespace fc
{

// === default mixins ===

template<class port_t>
using default_mixin = node_aware<port_t>;

// -- event sinks --

template<class data_t>
using event_sink = default_mixin<pure::event_sink<data_t>>;

template<class lambda_t>
using event_sink_tmpl = default_mixin<pure::event_sink_tmpl<lambda_t>>;

template<class lambda_t>
auto make_event_sink_tmpl(lambda_t h) { return event_sink_tmpl<lambda_t>{h}; }

// -- event sources --

template<class data_t>
using event_source = default_mixin<pure::event_source<data_t>>;

// -- state sinks --

template<class data_t>
using state_sink = default_mixin<pure::state_sink<data_t>>;

// -- state sources --

template<class data_t>
using state_source = default_mixin<pure::state_source<data_t>>;

template<class lambda_t>
using state_source_tmpl = default_mixin<pure::state_source_tmpl<lambda_t>>;

template<class lambda_t>
auto make_state_source_tmpl(lambda_t h) { return state_source_tmpl<lambda_t>{h}; }

// -- dispatch --

/// template input port, tag object creates either event_sink or state_sink
template<class data_t, class tag>
struct in_port
{
	typedef default_mixin<typename pure::in_port<data_t, tag>::type> type;
};

/// template output port, tag object creates either event_source or state_source
template<class data_t, class tag>
struct out_port
{
	typedef default_mixin<typename pure::out_port<data_t, tag>::type> type;
};

} // namespace fc

#endif /* SRC_PORTS_PORTS_HPP_ */
