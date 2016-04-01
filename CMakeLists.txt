CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

PROJECT( flexcore )

IF( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
	SET( STANDALONE TRUE )
ELSE()
	SET( STANDALONE FALSE )
ENDIF()

OPTION( ENABLE_COVERAGE_ANALYSIS "activate gcov based coverage anlysis" OFF )
OPTION( ENABLE_TESTS "build unit tests" ${STANDALONE} )

ADD_COMPILE_OPTIONS(  
    "-Wall"
    "-Wextra"
    "-pedantic"
    $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-fprofile-arcs>
    $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-ftest-coverage>
    )

IF( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
	ADD_COMPILE_OPTIONS(
	    "-Wno-unused-local-typedef"
	    )
ENDIF()

FIND_PACKAGE( Threads REQUIRED )
FIND_PACKAGE( Boost 1.54 REQUIRED COMPONENTS log thread system)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

SET ( FLEXCORE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "flexcore project path" )

INCLUDE( include_directories.cmake )

ADD_SUBDIRECTORY( src )
IF( ENABLE_TESTS )
	ADD_SUBDIRECTORY( tests )
	ADD_SUBDIRECTORY( integration_tests )
ENDIF()

