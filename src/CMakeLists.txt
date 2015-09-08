CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(core)

## Find all source files to be compiled into object files
file( GLOB SOURCES core/*.cpp )

ADD_LIBRARY(core ${SOURCES})
