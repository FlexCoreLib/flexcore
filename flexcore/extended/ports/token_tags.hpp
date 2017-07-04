#ifndef SRC_PORTS_TOKEN_TAGS_HPP_
#define SRC_PORTS_TOKEN_TAGS_HPP_

#include <flexcore/pure/pure_ports.hpp>

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

///in_port for events is event_sink
template<class data_t>
struct in_port<data_t, event_tag>
{
	using type = pure::event_sink<data_t>;
};

/// in_port for state is state_sink
template<class data_t>
struct in_port<data_t, state_tag>
{
	using type = pure::state_sink<data_t>;
};

/// template output port, tag object creates either event_source or state_source
template<class data_t, class tag>
struct out_port;

/// out_port for events is event_source
template<class data_t>
struct out_port<data_t, event_tag>
{
	using type = pure::event_source<data_t>;
};

/// out_port for state is state_source
template<class data_t>
struct out_port<data_t, state_tag>
{
	using type = pure::state_source<data_t>;
};

} // namespace pure
} // namespace fc

#endif /* SRC_PORTS_TOKEN_TAGS_HPP_ */
