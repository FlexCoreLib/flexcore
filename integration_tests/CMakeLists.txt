CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

ENABLE_TESTING()

ADD_EXECUTABLE( dummy_executable main.cpp )
TARGET_LINK_LIBRARIES( dummy_executable PUBLIC flexcore )
ADD_TEST( dummy_main dummy_executable )

