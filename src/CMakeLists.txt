CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

# creates the executable
ADD_LIBRARY( flexcore
	infrastructure.cpp
	extended/graph/graph.cpp
	utils/logging/logger.cpp
	extended/base_node.cpp
	scheduler/clock.cpp
	scheduler/cyclecontrol.cpp
	scheduler/parallelregion.cpp
	scheduler/parallelscheduler.cpp
	scheduler/serialschedulers.cpp )

TARGET_COMPILE_OPTIONS( flexcore
	PUBLIC "-std=c++1y" )
TARGET_INCLUDE_DIRECTORIES( flexcore
    PUBLIC ${FLEXCORE_INCLUDE_DIRECTORIES} )

# TODO only use coverage flags in debug mode
TARGET_LINK_LIBRARIES( flexcore $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-lgcov>
	${CMAKE_THREAD_LIBS_INIT}
	${Boost_LIBRARIES})
