CMAKE_MINIMUM_REQUIRED(VERSION 3.0.)

FIND_PACKAGE(benchmark)
IF (NOT benchmark_FOUND)
	message(WARNING "Disabling benchmarks")
	return()
ENDIF()

ADD_EXECUTABLE(flexcore_benchmark
	benchmarkfunctions.cpp
	range_benchmarks.cpp
	port_benchmarks.cpp
)

set_property(TARGET flexcore_benchmark PROPERTY CXX_STANDARD 14)

#TARGET_COMPILE_OPTIONS( flexcore_benchmark PUBLIC
#	"-march=native"
#	)

TARGET_LINK_LIBRARIES( flexcore_benchmark PUBLIC  
	benchmark
	pthread
	flexcore
	)