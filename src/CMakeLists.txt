CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

# creates the executable
ADD_LIBRARY( flexcore
    threading/Scheduler.cpp )

TARGET_INCLUDE_DIRECTORIES( flexcore
    PUBLIC "." )
 
# TODO only use coverage flags in debug mode
TARGET_LINK_LIBRARIES( flexcore $<$<BOOL:ENABLE_COVERAGE_ANALYSIS>:-lgcov> )


 	
 	
 	
 	
 	