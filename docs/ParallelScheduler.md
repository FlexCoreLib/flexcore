# Scheduler and Periodic Tasks

Calculations on nodes of the dataflow graph can generally be done in parallel. (See [Parallelregion](Parallelregion) )

To efficiently distribute these calculations a Scheduler is used.
The current implementation is based on a thread-pool together with a task-queue. (See [Thread Pool](https://en.wikipedia.org/wiki/Thread_pool_pattern) for an explanation.

The task queue of the scheduler is fed with cyclic task by a master thread (called Cyclecontrol) which makes sure that a cycle is executed once and only once in the duration of its cycle time.

Cyclecontrol goes through the following steps each cycle.
1. Advance virtual clock by minimal cycle duration
2. call switch tick of all parallel regions whose calculations are due. (The cycle duration of the region matches the current time.)
3. Add Work tick of these regions to worker threads of scheduler
4. Wait Remaining duration of cycle
5. Goto 1.

The switch tick serves as the synchronization point and the work tick does the actual calculations.

![2015-11-10_Scheduler_sequence](/uploads/cd71e525c58508c2505c2183585a24d5/2015-11-10_Scheduler_sequence.png)

![2015-09-25_scheduler_class_v2ck](/uploads/b9ea75aaa753d104817d9c83db3e8117/2015-09-25_scheduler_class_v2ck.png)
