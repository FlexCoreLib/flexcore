# Common Pitfalls

This page contains a list of common pitfalls, which might be hard to detect.

1. Ports which have been connected cannot be moved.
If a port is moved after it has been connected, the partner of the connection is left with a dangling reference. This will most likely result in a segfault with a corrupted stack.
If one wants to move a node after it's ports are connected, keep the ports by unique_ptr.

2. Connectables which are lvalues and have been connected to ports can not be moved after connecting. This would lead to a dangling reference inside the port.
If this is a problem, connectables should either be copied or allocated on the heap. A local copy helper can be of use
```cpp
auto source = [] { return 1; };
auto sink = [](int i) {};
auto copy = [](auto c) { return c; };
copy(source) >> copy(sink);
```
3. If a node instantiates an fc::event_sink or fc::state_source with a lambda capturing `this` or a reference to a member of the node, the node must not be moved.
Moving the node would invalidate the reference or pointer.
As a general rule (not flexcore specific): if you capture `this` in a lambda, delete move and copy constructor.