# Searches for RBDL includes and library files
#
# Sets the variables
#   RBDL_FOUND
#   RBDL_INCLUDE_DIR
#   RBDL_LIBRARY

SET (RBDL_FOUND FALSE)

FIND_PATH (RBDL_INCLUDE_DIR rbdl/rbdl.h
	/usr/include
	/usr/local/include
	$ENV{HOME}/local/include
	$ENV{RBDL_PATH}/src
	$ENV{RBDL_PATH}/include
	$ENV{RBDL_INCLUDE_PATH}
	)

FIND_LIBRARY (RBDL_LIBRARY NAMES rbdl	PATHS
	/usr/lib
	/usr/local/lib
	$ENV{HOME}/local/lib
	$ENV{RBDL_PATH}
	$ENV{RBDL_LIBRARY_PATH}
	)

FIND_LIBRARY (RBDL_LUAMODEL_LIBRARY NAMES rbdl_luamodel	PATHS
	/usr/lib
	/usr/local/lib
	$ENV{HOME}/local/lib
	$ENV{RBDL_PATH}
	$ENV{RBDL_LIBRARY_PATH}
	)

IF (NOT RBDL_LIBRARY)
	MESSAGE (ERROR "Could not find RBDL library")
ENDIF (NOT RBDL_LIBRARY)

IF (NOT RBDL_LUAMODEL_LIBRARY)
	MESSAGE (ERROR "Could not find LuaModel Addon for RBDL")
ENDIF (NOT RBDL_LUAMODEL_LIBRARY)

IF (RBDL_INCLUDE_DIR AND RBDL_LIBRARY AND RBDL_LUAMODEL_LIBRARY)
	SET (RBDL_FOUND TRUE)
ENDIF (RBDL_INCLUDE_DIR AND RBDL_LIBRARY AND RBDL_LUAMODEL_LIBRARY)

IF (RBDL_FOUND)
   IF (NOT RBDL_FIND_QUIETLY)
      MESSAGE(STATUS "Found RBDL: ${RBDL_LIBRARY}")
			MESSAGE(STATUS "Found RBDL LuaModel: ${RBDL_LUAMODEL_LIBRARY}")
   ENDIF (NOT RBDL_FIND_QUIETLY)
ELSE (RBDL_FOUND)
   IF (RBDL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find RBDL")
   ENDIF (RBDL_FIND_REQUIRED)
ENDIF (RBDL_FOUND)

MARK_AS_ADVANCED (
	RBDL_INCLUDE_DIR
	RBDL_LIBRARY
	RBDL_LUAMODEL_LIBRARY
	)
