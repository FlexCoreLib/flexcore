CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

CMAKE_POLICY( SET CMP0028 NEW ) # Alias/Namespace targets

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
TARGET_INCLUDE_DIRECTORIES( flexcore PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cereal/include>
	$<INSTALL_INTERFACE:include/>
	$<INSTALL_INTERFACE:include/flexcore/3rdparty>
	)

# TODO only use coverage flags in debug mode
TARGET_LINK_LIBRARIES( flexcore $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-lgcov>
	Boost::boost
	Boost::log
	Threads::Threads
	)

# install instructions
IF( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8" )
	SET( _LIB_SUFFIX 64 )
ELSEIF( "${CMAKE_SIZEOF_VOID_P}" EQUAL "4" )
	SET( _LIB_SUFFIX 32 )
ELSE()
	SET( _LIB_SUFFIX "" )
ENDIF()

SET( LIB_SUFFIX ${_LIB_SUFFIX} CACHE STRING "suffix of library install dir" )
SET( INSTALL_LIB_DIR lib${LIB_SUFFIX} )
SET( INSTALL_INCLUDE_DIR include/flexcore )
SET( INSTALL_CMAKE_DIR ${INSTALL_LIB_DIR}/cmake/flexcore )

INSTALL( TARGETS flexcore
	EXPORT flexcore-targets
	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
	ARCHIVE DESTINATION ${INSTALL_LIB_DIR} )
INSTALL( EXPORT flexcore-targets DESTINATION ${INSTALL_CMAKE_DIR} )
INSTALL( FILES flexcore-config.cmake ../cmake/FindBoost.cmake DESTINATION ${INSTALL_CMAKE_DIR} )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${INSTALL_INCLUDE_DIR}
	FILES_MATCHING
	PATTERN "3rdparty/cereal/*" EXCLUDE
	PATTERN "*.hpp" )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cereal/include/
	DESTINATION ${INSTALL_INCLUDE_DIR}/3rdparty
	FILES_MATCHING PATTERN "*.hpp"
	PATTERN "*.h" )

