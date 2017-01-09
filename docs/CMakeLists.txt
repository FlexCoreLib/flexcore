find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	
	#Build cereal documentation
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../flexcore/3rdparty/cereal
		${CMAKE_CURRENT_BINARY_DIR}/cereal
	)

	#the working dir is a bit of a hack as it is force set to the flexcore directory via relative path to docs
	#this is currently necessary as the docs are only working when doxygen is executed from flexcore top level
	add_custom_target(flexcore_doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
		COMMENT "Generating flexcore API documentation with Doxygen"
		VERBATIM
	)
	
	#add cereal doc target (which is only called doc up to 1.2.1)
	add_dependencies(flexcore_doc doc)
	
endif(DOXYGEN_FOUND)