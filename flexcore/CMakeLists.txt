CMAKE_POLICY( SET CMP0028 NEW ) # Alias/Namespace targets

# creates the executable
ADD_LIBRARY( flexcore
	infrastructure.cpp
	extended/graph/graph.cpp
	utils/logging/logger.cpp
	utils/demangle.cpp
	extended/base_node.cpp
    extended/visualization/visualization.cpp
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

IF( FLEXCORE_ENABLE_COVERAGE_ANALYSIS )
	TARGET_LINK_LIBRARIES( flexcore gcov )
ENDIF()
TARGET_LINK_LIBRARIES( flexcore
	Boost::boost
	Boost::log
	Threads::Threads
	)

INCLUDE(GNUInstallDirs)
INCLUDE(CMakePackageConfigHelpers)

CONFIGURE_PACKAGE_CONFIG_FILE(flexcore-config.cmake.in flexcore-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flexcore
	PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR)

INSTALL( TARGETS flexcore
	EXPORT flexcore-targets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} )
INSTALL( EXPORT flexcore-targets
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flexcore )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/flexcore-config.cmake ../cmake/FindBoost.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flexcore )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flexcore
	FILES_MATCHING
	PATTERN "3rdparty/cereal/*" EXCLUDE
	PATTERN "*.hpp" )
INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cereal/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/flexcore/3rdparty
	FILES_MATCHING PATTERN "*.hpp"
	PATTERN "*.h" )

