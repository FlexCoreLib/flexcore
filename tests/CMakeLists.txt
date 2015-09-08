CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

PROJECT(tests)
ENABLE_TESTING()

INCLUDE_DIRECTORIES(../src)

ADD_DEFINITIONS(-std=c++1y)

## Find all source files to be compiled into object files
file( GLOB SOURCES core/Test*.cpp )

# creates the executable
ADD_EXECUTABLE(test_executable 
	core/Runner.cpp
	${SOURCES})

# indicates the include paths
# TARGET_INCLUDE_DIRECTORIES(test_executable PRIVATE ${BOOST_INCLUDE_DIRS})

# Link against core
TARGET_LINK_LIBRARIES (test_executable LINK_PUBLIC core)

# declares a test with our executable
ADD_TEST(NAME test COMMAND test_executable)
