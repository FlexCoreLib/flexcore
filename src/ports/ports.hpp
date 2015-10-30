#ifndef SRC_PORTS_PORTS_HPP_
#define SRC_PORTS_PORTS_HPP_

#include "event_ports.hpp"
#include "state_ports.hpp"

namespace fc
{

/// tag to specify that template uses events
struct event_tag {};
/// tag to specify that template uses states
struct state_tag {};

///template input port, tag object creates either event_in_port or state_sink
template<class data_t, class tag>
struct in_port;


template<class data_t>
struct in_port<data_t, event_tag> : public event_in_port<data_t> {};

template<class data_t>
struct in_port<data_t, state_tag> : public state_sink<data_t> {};

///template output port, tag object creates either event_out_port or state_source_call_function
template<class data_t, class tag>
struct out_port;

template<class data_t>
struct out_port<data_t, event_tag> : public event_out_port<data_t> {};

template<class data_t>
struct out_port<data_t, state_tag> : public state_source_call_function<data_t> {};

}

#endif /* SRC_PORTS_PORTS_HPP_ */
