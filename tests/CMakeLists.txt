CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

PROJECT(tests)
ENABLE_TESTING()

# indicates the location of the boost installation tree.
# hard-coded for our simple example.
SET(BOOST_INCLUDE_DIRS /usr/include/boost)

INCLUDE_DIRECTORIES(../src)

ADD_DEFINITIONS(-std=c++1y)

# creates the executable
ADD_EXECUTABLE(test_executable 
	core/TestConnection.cpp)

# indicates the include paths
TARGET_INCLUDE_DIRECTORIES(test_executable PRIVATE ${BOOST_INCLUDE_DIRS})

# declares a test with our executable
ADD_TEST(NAME test COMMAND test_executable)
