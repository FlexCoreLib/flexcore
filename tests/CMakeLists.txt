CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#ENABLE_TESTING()

# test_executable

ADD_EXECUTABLE( test_executable 
	core/test_connection.cpp
	core/test_traits.cpp
	clock/TestClock.cpp
	ports/test_node_aware.cpp
 	ports/events/test_events.cpp
	ports/states/test_node_with_lambda.cpp
	ports/states/test_state_source_call_function.cpp
	ports/states/test_state_source_with_setter.cpp
	runner.cpp 
	threading/test_parallel_region.cpp
	threading/test_scheduler.cpp )

TARGET_INCLUDE_DIRECTORIES( test_executable 
	PRIVATE ${BOOST_INCLUDE_DIRS}
	"../src/" 
	"." )

TARGET_LINK_LIBRARIES( test_executable
	PUBLIC flexcore )

# declares a test with our executable
#ADD_TEST( NAME test WORKING_DIRECTORY "." COMMAND test_executable )


