CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

# dummy_executable

ADD_EXECUTABLE( dummy_executable 
	main.cpp )

TARGET_LINK_LIBRARIES( dummy_executable
	PUBLIC flexcore )

