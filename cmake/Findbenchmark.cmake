# FindBenchmark
# ---------
#
# Find google benchmark include dirs and libraries
#
# The following variables are optionally searched for defaults
# benchmark_ROOT_DIR:  Base directory where all benchmark components are found
#
# Once done this will define
# benchmark_FOUND - System has benchmark
# benchmark_INCLUDE_DIRS - The benchmark include directories
# benchmark_LIBRARIES - The libraries needed to use benchmark

find_path(benchmark_INCLUDE_DIR "benchmark/benchmark.h"
	PATHS ${benchmark_ROOT_DIR}
	PATH_SUFFIXES include
	NO_DEFAULT_PATH)
find_path(benchmark_INCLUDE_DIR "benchmark/benchmark.h")

find_library(benchmark_LIBRARY NAMES "benchmark"
	PATHS ${benchmark_ROOT_DIR}
	PATH_SUFFIXES lib lib64
	NO_DEFAULT_PATH)
find_library(benchmark_LIBRARY NAMES "benchmark")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set benchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(benchmark FOUND_VAR benchmark_FOUND
	REQUIRED_VARS benchmark_LIBRARY
	benchmark_INCLUDE_DIR)

if (benchmark_FOUND AND NOT TARGET benchmark)
	add_library(benchmark UNKNOWN IMPORTED)
	set_target_properties(benchmark PROPERTIES
		IMPORTED_LOCATION "${benchmark_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${benchmark_INCLUDE_DIR}"
	)
endif()
