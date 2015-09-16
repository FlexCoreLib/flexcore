CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

#ENABLE_TESTING()

# test_executable

ADD_EXECUTABLE( test_executable 
	core/TestConnection.cpp
	core/TestTraits.cpp
	ports/test_stream_query_node.cpp
	ports/test_stream_state.cpp
	Runner.cpp 
	threading/TestScheduler.cpp )

TARGET_INCLUDE_DIRECTORIES( test_executable 
	PRIVATE ${BOOST_INCLUDE_DIRS}
	"../src" 
	"." )

TARGET_LINK_LIBRARIES( test_executable
	PUBLIC flexcore )

# declares a test with our executable
#ADD_TEST( NAME test WORKING_DIRECTORY "." COMMAND test_executable )


