# Parallel Regions

Parallel Regions form the basis for parallel executions of sub-graphs of a flexcore program.

All nodes inside the region are guaranteed to be executed by a single thread.
All calculations within the region might be performed in a different thread than calculations outside the region.

Connections entering or leaving the region go through buffers.
These buffers are use double buffering to allow asynchronous access to from within and without the region.
The region controls the buffers and switches their content at the beginning of each cycle.

Buffers are created automatically and inserted into the connection, if the connection is between ports from two different regions.

Client code specifies which region a port belongs to as a constructor parameter to the port.

![2015-11-02_ParallelRegion](/uploads/181d3535896a9a8d0a03057132b0a44a/2015-11-02_ParallelRegion.png)

Each Region has two ticks, which trigger all operations:
1. The switch tick triggers switching of all buffers of the region.
2. The work tick triggers the actual calculations inside the nodes.

These ticks are controlled by the [Parallelscheduler](Parallelscheduler).
