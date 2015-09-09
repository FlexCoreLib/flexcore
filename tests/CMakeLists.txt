CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

PROJECT(tests)
ENABLE_TESTING()

# indicates the location of the boost installation tree.
# hard-coded for our simple example.
SET(BOOST_INCLUDE_DIRS /usr/include/boost)

INCLUDE_DIRECTORIES(../src)

SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

ADD_DEFINITIONS(-std=c++1y)

# creates the executable
ADD_EXECUTABLE(test_executable 
	core/Runner.cpp
	core/TestConnection.cpp)

# indicates the include paths
TARGET_INCLUDE_DIRECTORIES(test_executable PRIVATE ${BOOST_INCLUDE_DIRS})

# declares a test with our executable
ADD_TEST(NAME test COMMAND test_executable)
