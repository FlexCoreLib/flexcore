#ifndef SRC_PORTS_TOKEN_TAGS_HPP_
#define SRC_PORTS_TOKEN_TAGS_HPP_

#include "pure_ports.hpp"

namespace fc
{

/// tag to specify that template uses events
struct event_tag {};
/// tag to specify that template uses states
struct state_tag {};

namespace pure
{

/// template input port, tag object creates either event_sink or state_sink
template<class data_t, class tag>
struct in_port;

template<class data_t>
struct in_port<data_t, event_tag>
{
	typedef event_sink<data_t> type;
};

template<class data_t>
struct in_port<data_t, state_tag>
{
	typedef state_sink<data_t> type;
};

/// template output port, tag object creates either event_source or state_source_call_function
template<class data_t, class tag>
struct out_port;

template<class data_t>
struct out_port<data_t, event_tag>
{
	typedef event_source<data_t> type;
};
template<class data_t>
struct out_port<data_t, state_tag>
{
	typedef state_source_call_function<data_t> type;
};

} // namespace pure
} // namespace fc

#endif /* SRC_PORTS_TOKEN_TAGS_HPP_ */
