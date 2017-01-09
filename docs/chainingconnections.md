# Chaining of Connections

FlexCore allows connections to be chained.
This means that more then two connectables can be used in a linear connection.

The following line of code chains two connectables (Lambdas in this case) which increment their input in the connection between `source` and `sink`. `sink` will receive the output of source +2.
~~~{.cpp}
source >> [](int in){ return ++in; } >> [](int in){ return ++in; } >> sink;
~~~

It is not required that the data type of the token travelling through the connection stays the same.
Connectables in between nodes are especially useful to convert data types if the sink requires a different type than the source provides.

Chains of connectables are built at compile time and can have arbitrary length.
They are made possible by the fact the connections are connectable themselves. See [Core Concepts](CoreConcept).
This means that chains of connections can be optimized very well by the compiler.