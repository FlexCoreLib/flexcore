CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

ENABLE_TESTING()

# These four flags enable use of gcov
#SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
#SET(GCC_COVERAGE_LINK_FLAGS "-lgcov")

#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

ADD_EXECUTABLE( test_executable 
	Runner.cpp )

TARGET_INCLUDE_DIRECTORIES( test_executable 
    PRIVATE ${BOOST_INCLUDE_DIRS}
    "../src" 
    "." )

#TARGET_COMPILE_OPTIONS( flexcore  
#    PUBLIC "-Wall"
#    PUBLIC "-std=c++1y" )

TARGET_LINK_LIBRARIES( test_executable
    PUBLIC flexcore )

# declares a test with our executable
ADD_TEST(NAME test COMMAND test_executable)


