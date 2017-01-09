# Concepts

## Connection Concepts

### Connectable
There is one main concept which forms the foundation of the FlexCore library:
**connectable**. Every type, which fulfils this concept can be connected.
Every edge in the dataflow graph connects to connectable objects.

The concept **connectable** extends two concepts from the standard library: 

- **callable**, described at http://en.cppreference.com/w/cpp/concept/Callable
- **copyable**, described at: http://en.cppreference.com/w/cpp/types/is_copy_constructible.

Examples for connectable types are: Lambdas, Function Pointers, std::function,
functors etc.

There is one exception to the **copyable** requirement: [lvalues][lvalue]. Callable objects which are connected as named variables need not be copyable. They will be wrapped in a `std::ref` object to establish the connection. As a consequence, lvalue connectables are **not** allowed to change address after the connection is established.

### Connections
Two connectables are connected by calling the free template function **connect**,
which takes two connectables as parameters.

This returns an object of type **connection**, which contains both arguments of
connect. A connection object is itself connectable.
~~~{.cpp}
template <class L, class R> //L and R need to fulfil connectable
connection<L, R> connect(L lhs, R rhs);
connectable_a >> connectable_b;
~~~

Users should not call `connect` directly but instead use the overloaded
`operator>>` which performs type validation and then calls `connect`.
~~~{.cpp}
using fc::operator>>;
connectable_a >> connectable_b;
~~~

The following UML class diagram shows the interaction between these concepts:

![CoreClassDiagram_v3](CoreClassDiagram_v3.png)

By calling connect on connectables and connections repeatedly we can
build **chains** of connections.

### Restrictions: Active- And Passive-Connectable
Two additional concepts complete the design.  These are **active_connectable**
and **passive_connectable**. active_connectable and passive_connectable serve
as end points of chains of connections. Types which are active_connectable do
not need to be callable unlike passive_connectable and connectable.  connectable
types are both active_connectable and passive_connectable.

Any type which is neither connectable nor passive_connectable or
active_connectable is called **not_connectable**. This is not a separate
concept, as it is defined purely as the negation of the three other.

Both active and passive connectable types can appear on the left and right hand
side of a connection. Since they form the end points of a chain of connections,
active_connectable and passive_connectable restrict how connect can be called on
them:

~~~{.cpp}
// for push-logic, i.e. sending events (source is active (pusher), sink is passive)
connect(active_connectable, connectable);         // returns an active_connectable.
connect(connectable, passive_connectable);        // returns a passive_connectable.
connect(active_connectable, passive_connectable); // connection is complete, returns a non_connectable connection.

// for pull-logic, i.e. receiving states  (source is passive, sink is active (puller))
connect(connectable, active_connectable);         // returns an active_connectable.
connect(passive_connectable, connectable);        // returns a passive_connectable.
connect(passive_connectable, active_connectable); // connection is complete, returns a non_connectable connection.

connect(connectable, connectable);                // returns a connectable as defined above.

connect(active_connectable, active_connectable);                   // illegal.
connect(passive_connectable, passive_connectable);                 // illegal.
~~~

![CoreClassDiagram_v4](CoreClassDiagram_v4.png)

[lvalue]: http://en.cppreference.com/w/cpp/language/value_category
