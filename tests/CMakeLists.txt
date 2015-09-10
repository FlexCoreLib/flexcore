CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

PROJECT(tests)
ENABLE_TESTING()

INCLUDE_DIRECTORIES(../src)

# These four flags enable use of gcov
#SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
#SET(GCC_COVERAGE_LINK_FLAGS "-lgcov")

#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

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