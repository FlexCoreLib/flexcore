Getting started
===============
# Basics {#basics}

Flexcore is a library for dataflow programming. To make good use of it, you need
to figure out how to map your problem onto the constructs available in it.

There are 3 basic building-blocks in flexcore:

  - [ports](#ports)
  - [regions](#regions)
  - [nodes](#nodes)

Ports are used for exchanging information. Nodes perform calculations and
request or send information through ports. Regions are all about the
dependencies between nodes. If nodes are independent put them in different
regions. Being independent does not mean nodes from different regions can't
communicate.

## ports {#ports}

There are two kinds of information that are passed around flexcore: states and events.
States model information that is always available. Think temperature sensor.
Events model information that is sent around. Think button press.

Communication through ports is unidirectional - always from source to sink.
There is always an initiating side to signal flow - in the case of events it is
the [event_source](@ref fc::event_source) that sends the event. For states it is
the [state_sink](@ref fc::state_sink) that requests a state from the source.

All ports are templatized and can be used to transfer any data type.
Connections are established using `operator >>`.

~~~{.cpp}
#include <flexcore/pure/pure_ports.hpp>
#include <iostream>
using namespace fc;

void print_int(int v) { std::cout << v << std::endl; }

int main()
{
	pure::event_source<int> src;
	pure::event_sink<int> sink{print_int};
	src >> sink;
	src.fire(42);
}
~~~

Ports are most useful when coupled with C++11 lambdas. Event sinks and state
sources need to have access to a functor that can handle the event/provide the
state.

~~~{.cpp}
#include <flexcore/pure/pure_ports.hpp>
#include <iostream>
#include <cassert>
#include <string>
using namespace fc;

std::string var;

int main()
{
	pure::state_source<std::string> src{[] { return var; }};
	pure::state_sink<std::string> sink;
	src >> sink;
	var = "Hello!";
	assert(sink.get() == "Hello!");
}
~~~

Ports in the [pure namespace](@ref fc::pure) are a raw communication primitive.
More advanced use cases are meant to be served by the enhanced ports in the
[fc namespace](@ref fc).

## nodes {#nodes}
While it is possible to work directly using ports, it is much more practical to
aggregate inputs/outputs in a class. In flexcore this is called a node. If
possible, nodes should derive from [tree_base_node](@ref fc::tree_base_node).
This allows nodes to work with the infrastructure available in flexcore.

A simple node could look like the following.

~~~{.cpp}
#include <flexcore/extended/base_node.hpp>
#include <flexcore/infrastructure.hpp>
#include <iostream>

int get_temperature() { return 42; }
void print_temperature(int T)
{
       std::cout << "Current temperature is: " << T << std::endl;
}

struct my_node : fc::tree_base_node
{
       my_node(std::shared_ptr<fc::parallel_region> reg, std::string name)
           : tree_base_node(reg, name)
               , trigger(this, [this] { this->work(); })
               , curr_temp(this)
               , temp_out(this, get_temperature)
       {}

       void work()
       {
               curr_temp.fire(temp_out());
       }
       fc::event_sink<void> trigger;
       fc::event_source<int> curr_temp;
       fc::state_source<int> temp_out;
};


int main()
{
       fc::pure::event_source<void> go_signal;
       fc::pure::event_sink<int> sink{print_temperature};

       fc::infrastructure infra;
       my_node* node = infra.node_owner().make_child_named<my_node>("my node");

       go_signal >> node->trigger;
       node->curr_temp >> sink;

       go_signal.fire();
}
~~~

The ports used in the above example are enhanced ports, and so require a
pointer to a fc::tree_base_node as the first constructor parameter.

## regions {#regions}

As more and more nodes are added to the dataflow graph, dependencies start to
form. Some nodes may access a shared resource that shouldn't be accessed
concurrently. Regions are used to express these kinds of relationships in
flexcore.

Regions are meant to be used with the flexcore infrastructure. The assumption
is that the dataflow is cycle driven - periodic tasks that need to be executed
at given intervals.

~~~{.cpp}
#include <flexcore/infrastructure.hpp>
#include <iostream>
#include <sstream>

struct my_region_node : fc::owning_base_node
{
	 my_region_node(std::shared_ptr<fc::parallel_region> reg, std::string name, forest_t* f)
		: owning_base_node(reg, name, f)
		, i(0)
		, src(this, [this] { return i++; })
		, sink(this)
		, do_work(this, [this,name] { work(name); })
	{}

	void work(const std::string& name)
	{
		std::stringstream ss;
		ss << name << " " << sink.get() << std::endl;
		std::cout << ss.str();
	}
	int i;
	fc::state_source<int> src;
	fc::state_sink<int> sink;
	fc::event_sink<void> do_work;
};

int main()
{
	fc::infrastructure infra;
	auto region_a = infra.add_region("region a", fc::thread::cycle_control::medium_tick);
	auto region_b = infra.add_region("region b", fc::thread::cycle_control::fast_tick);

	my_region_node* node_a = infra.node_owner()
	                              .make_child_named<my_region_node>(region_a, "node a");
	my_region_node* node_b = infra.node_owner()
	                              .make_child_named<my_region_node>(region_b, "node b");

	node_a->src >> node_b->sink;
	node_b->src >> node_a->sink;
	region_a->work_tick() >> node_a->do_work;
	region_b->work_tick() >> node_b->do_work;

	infra.start_scheduler();
	infra.iterate_main_loop();
	infra.stop_scheduler();
}
~~~

In the above example two nodes are created with their own regions. They send
each other information in the form of integers. The important thing to know is
that the two regions will be executed in parallel, but nodes within each region
will be sent work_tick events serially. Information flowing between the regions
is buffered to remove the need for synchronization.

Regions also have event_source ports. Regions automatically send out events
responsible for moving data through the buffers between regions, but need to be
manually connected to the nodes that are to perform periodic tasks (via
[`work_tick()`](fc::parallel_region::work_tick)).

## Where to go from here

Now that you have seen some of the building blocks available in flexcore you
can use them to model your scenario. Start by drawing up the flow of
information and processing steps. The processing steps will most likely be
nodes, ports will be at the end of lines connecting nodes. Group things using
regions as necessary to enable safe parallelism.
