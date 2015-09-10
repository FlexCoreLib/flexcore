CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

PROJECT( flexcore )

SET(ENABLE_COVERAGE_ANALYSIS FALSE CACHE BOOL "")

# TODO: use profiling args only in debug mode
ADD_COMPILE_OPTIONS(  
    "-Wall"
    "-std=c++1y" 
    $<$<BOOL:ENABLE_COVERAGE_ANALYSIS>:-fprofile-arcs>
    $<$<BOOL:ENABLE_COVERAGE_ANALYSIS>:-ftest-coverage> 
    )

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( tests )



