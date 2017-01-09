# Ports

There are four basic kinds of ports in flexcore.

  * `event_source`
  * `event_sink`
  * `state_source`
  * `state_sink`

`event_source` and `state_sink` are active ports, so they store connections and initiate the communication. `event_sink` and `state_sink` are passive ports, and are needed to complete a connection.
The basic flexcore ports are non-copyable but moveable. Beware of [moving ports that have been connected](knownpitfalls). For this reason only lvalue ports can be used in connections.

The various ports described below are part of the `fc::pure` namespace. To get these implementations use the line:
~~~{.cpp}
#include <flexcore/pure/pure_ports.hpp>
~~~
To take advantage of ports with added functionality (node-aware, graph-connectable), use the ports in the `flexcore` namespace and include the header:
~~~{.cpp}
#include <flexcore/ports.hpp>
~~~

## active ports
### event_source
~~~{.cpp}
template <class event_t>
struct event_source
{
    void fire(event_t event);
    template <class conn_t>
    port_connection<event_source<event_t>, conn_t, event>  connect(conn_t&&) &;
};
~~~
`event_t` is the type of data that this port sends; it may be void, in which case `fire` takes no arguments. `event_source` may be connected to multiple ports and will send every one of them any event it is fired with. `connect()` can only be called on an lvalue port.

### state_sink
~~~{.cpp}
template <class data_t>
class state_sink
{
    data_t get() const;
    template <class conn_t>
    port_connection<conn_t, state_sink<data_t>, data_t> connect(conn_t&&) &;
};
~~~
`data_t` is the type of data that this port provides. The `get()` method calls the connection. A `state_sink` can only be connected to a single other port. Connecting the `state_sink` more than once invokes undefined behaviour.

## passive ports
### event_sink
~~~{.cpp}
template <class event_t>
struct event_sink
{
    explicit event_sink(const std::function<void(event_t)>& handler);
    template <class T>
    void operator()(T&&); // when event_t != void and T is convertible to event_t.
    // or
    void operator()(); // when  event_t == void.
};
~~~
`event_t` is the type of event that is accepted by this port. `void` is an acceptable type for *poke* style events that don't transfer any data. An `event_sink` will accept any kind of event that is implicitly convertible to its `event_t`. 

The handler function passed to the constructor is a callback that will be called when the port receives an event. Typically this will notify the node that the port belongs to of the incoming event.

### state_source
~~~{.cpp}
template <class data_t>
class state_source
{
    explicit state_source(std::function<data_t()> f);
    data_t operator()();
};
~~~
`data_t` is the type of state that the `state_source` returns. The function `f` passed to the constructor is expected to deliver an object of type `data_t` on very call. This function will be called every time when the `state_sink` that this `state_source` is connected to needs a state.
