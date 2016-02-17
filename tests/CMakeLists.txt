CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#ENABLE_TESTING()

# test_executable
ADD_EXECUTABLE( test_executable 
	core/test_connection.cpp
	core/test_connectables.cpp
	core/test_traits.cpp
	clock/TestClock.cpp
	graph/test_graph.cpp
	nodes/test_base_node.cpp
	nodes/test_buffer.cpp
	nodes/test_list_manipulation.cpp
	nodes/test_generic.cpp
	nodes/test_event_nodes.cpp
	nodes/test_state_nodes.cpp
	ports/test_event_buffer.cpp
	ports/test_node_aware.cpp
	ports/test_state_buffer.cpp
 	ports/events/test_events.cpp
 	ports/states/test_state_sinks.cpp
	ports/states/test_node_with_lambda.cpp
	ports/states/test_state_source_call_function.cpp
	ports/states/test_state_source_with_setter.cpp
	ports/states/test_state_source_tmpl.cpp
	range/test_range.cpp
	runner.cpp 
	threading/test_cyclecontrol.cpp
	threading/test_parallel_region.cpp
	threading/test_minimalscheduler.cpp
	threading/test_parallelscheduler.cpp
	tokens/test_moving.cpp )

TARGET_INCLUDE_DIRECTORIES( test_executable 
	PRIVATE "." )

TARGET_LINK_LIBRARIES( test_executable
	PUBLIC flexcore )

# declares a test with our executable
#ADD_TEST( NAME test WORKING_DIRECTORY "." COMMAND test_executable )
