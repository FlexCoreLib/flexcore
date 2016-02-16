CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

PROJECT( flexcore )

SET( ENABLE_COVERAGE_ANALYSIS FALSE CACHE BOOL "" )

# TODO: use profiling args only in debug mode
ADD_COMPILE_OPTIONS(  
    "-Wall"
    "-Wextra"
    "-std=c++1y" 
    "-pthread"
    $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-fprofile-arcs>
    $<$<BOOL:${ENABLE_COVERAGE_ANALYSIS}>:-ftest-coverage>
    )
FIND_PACKAGE( Boost 1.54 REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

SET ( FLEXCORE_DIR "." CACHE PATH "flexcore project path" )

INCLUDE( include_directories.cmake )

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( tests )
ADD_SUBDIRECTORY( dummy_main )

