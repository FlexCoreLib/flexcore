cmake_minimum_required( VERSION 2.8 )

## Required software
SET(Boost_USE_MULTITHREADED ON)
find_package( Boost REQUIRED )

## Global sources and headers
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})


## C++ compiler options
set( CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++1y" )
set( CMAKE_CXX_FLAGS_DEBUG "-g -O0" )
set( CMAKE_CXX_FLAGS_RELEASE "-O3" )


add_subdirectory(tests)
