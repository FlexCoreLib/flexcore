# Glossary

Definitions of the concepts and terms used in flexcore. The main objective is
to create a common nomenclature to avoid misunderstandings.

* **Dataflow Programming:** Programming paradigm, where software structure is modelled as a directed graph with data flowing between nodes. See [dataflow][].
* **flexcore:** This is what all the fuzz is about. A C++ library for dataflow programming. The name flexcore is also used for the development project.

### Connectables
* **Node:** Nodes are connected to form the graph of the dataflow program.  Functionality is written in the code of the node. Nodes can again consist of several other nodes. This is then called a **compound node**. Nodes communicate using connections established between ports.
* [Parallel Region:](md_docs_ParallelRegion.html) A subgraph of the dataflow graph which allows all computations inside to be executed in parallel to all computations outside is called a parallel region. Parallel Regions can consist of single nodes or compound nodes.
* [Port:](md_docs_ports.html) Ports are the end points of connections, and are accesible to nodes. Ports can be **input ports** (sinks) or **output ports** (sources) for state or events.  Nodes can have explicitly declared ports. Ports of a node can also be implicit, when the node is very simple.
* **Pure:** A node that does not have side effects is called pure. A pure node always produces the same output, when given the same inputs. See [pure][] The term must not be confused with the  flexcore namespace _pure_, which is used to distinguish low level classes, that are essential for the libraries dataflow concept, and higher order classes, which are primarily extensions to the core concept and are more convenient to use in most applications.
* **Sink:** A sink is a port, which receives tokens over one or more connections.  A sink can be seen as a source of data for a node. Sinks can either pull tokens over **states** or receive **events**.
* **Source:** A source is a port, which sends tokens. A source can either be pulled through a state, or send events on its own.
* **Unary:** A node that has exactly one input port and exactly one output port is called unary.

### Connections
* **Connection:** Connections are the edges of the dataflow graph. All interaction between nodes is done through **tokens**, which travel over connections.
* **Graph:** The structure of a program written in flexcore is a graph. It is a directed graph, which might, but doesn't need to be acyclic. The graph consists of nodes and edges (called **connection** in flexcore), see [directed-graph][].
* **active:** In a connection, the port that calls through the connection is called active.
* **passive:** The port, that is being called through a connection is called passive. Each connection connects an active port with a passive port.

### Data
* **Token:** Tokens are pieces of data travelling over connections between nodes. Tokens can contain arbitrary (but finite) amounts of data. Tokens can be sent as **events** or be pulled as **states**. Whether a Token is a state or an event is determined by the connection the token flows through. This information is not stored in the token itself, but in the ports trough which a node sends and receives the token.
* **Event:** A token, which is pushed through a connection. flexcore uses Events when one node wants to send a message to other nodes. When an event is sent, the source is **active** and the sink is **passive**.  Events in flexcore correspond to the general concept in event driven design. See [event-computing][].
* **State:** A token that can be pulled through a connection. States are used to model continuous data and communication of state between nodes. When tokens travel as state, the sink is **active** and the source is **passive**.

[dataflow]: https://en.wikipedia.org/wiki/Dataflow_programming
[pure]: https://en.wikipedia.org/wiki/Pure_function
[event-computing]: https://en.wikipedia.org/wiki/Event_%28computing%29
[directed-graph]: https://en.wikipedia.org/wiki/Directed_graph
