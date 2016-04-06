# Generate list of relevant flexcore include directories

MESSAGE( WARNING "Including this file is deprecated. "
	"Please see ${CMAKE_CURRENT_LIST_DIR}/docs/USING for instructions on including flexcore." )
SET (FLEXCORE_INCLUDE_DIRECTORIES
     ${FLEXCORE_DIR}/src
     ${FLEXCORE_DIR}/src/3rdparty/cereal/include )

