CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

# creates the executable
ADD_LIBRARY( flexcore
	clock/clock.cpp
	graph/graph.cpp
	logging/logger.cpp
	threading/parallelregion.cpp
	threading/cyclecontrol.cpp
    threading/minimalscheduler.cpp
    threading/parallelscheduler.cpp
    threading/serialschedulers.cpp )

TARGET_INCLUDE_DIRECTORIES( flexcore
    PUBLIC ${FLEXCORE_INCLUDE_DIRECTORIES} )
 
# TODO only use coverage flags in debug mode
TARGET_LINK_LIBRARIES( flexcore $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-lgcov>
	-pthread
	${Boost_LIBRARIES})
