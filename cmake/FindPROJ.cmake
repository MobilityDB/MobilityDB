# Find Proj
# ~~~~~~~~~
# Copyright (c) 2021, Vicky Vergara <vicky at georepublic dot de>
# Adjustments for MobilityDB
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for Proj library
#
# If it's found it sets PROJ_FOUND to TRUE
# and following variables are set:
#    PROJ_INCLUDE_DIRS
#    PROJ_LIBRARIES

# FIND_PATH and FIND_LIBRARY normally search standard locations
# before the specified paths. To search non-standard paths first,
# FIND_* is invoked first with specified paths and NO_DEFAULT_PATH
# and then again with no specified paths to search the default
# locations. When an earlier FIND_* succeeds, subsequent FIND_*s
# searching for the same item do nothing.

# try to use framework on mac
# want clean framework path, not unix compatibility path
if(APPLE)
  if(CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
      OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
      OR NOT CMAKE_FIND_FRAMEWORK)
    SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
    SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
    #FIND_PATH(PROJ_INCLUDE_DIRS PROJ/proj_api.h)
    FIND_LIBRARY(PROJ_LIBRARIES PROJ)
    if(PROJ_LIBRARIES)
      # FIND_PATH doesn't add "Headers" for a framework
      SET (PROJ_INCLUDE_DIRS ${PROJ_LIBRARIES}/Headers CACHE PATH "Path to a file.")
    endif()
    SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
  endif()
endif()

FIND_PATH(PROJ_INCLUDE_DIRS proj_api.h
  "$ENV{INCLUDE}"
  "$ENV{LIB_DIR}/include"
  )

if(NOT PROJ_INCLUDE_DIRS)
  FIND_PATH(PROJ_INCLUDE_DIRS proj.h
    "$ENV{INCLUDE}"
    "$ENV{LIB_DIR}/include"
    )
endif()

FIND_LIBRARY(PROJ_LIBRARIES NAMES proj_i proj PATHS
  "$ENV{LIB}"
  "$ENV{LIB_DIR}/lib"
  )

if(PROJ_INCLUDE_DIRS)
   if(EXISTS ${PROJ_INCLUDE_DIRS}/proj.h AND EXISTS ${PROJ_INCLUDE_DIRS}/proj_experimental.h)
     file(READ ${PROJ_INCLUDE_DIRS}/proj.h proj_version)
     string(REGEX REPLACE "^.*PROJ_VERSION_MAJOR +([0-9]+).*$" "\\1" PROJ_VERSION_MAJOR "${proj_version}")
     string(REGEX REPLACE "^.*PROJ_VERSION_MINOR +([0-9]+).*$" "\\1" PROJ_VERSION_MINOR "${proj_version}")
     string(REGEX REPLACE "^.*PROJ_VERSION_PATCH +([0-9]+).*$" "\\1" PROJ_VERSION_PATCH "${proj_version}")
     string(CONCAT PROJ_VERSION_STR "(" ${PROJ_VERSION_MAJOR} "." ${PROJ_VERSION_MINOR} "." ${PROJ_VERSION_PATCH} ")")
   else()
     file(READ ${PROJ_INCLUDE_DIRS}/proj_api.h proj_version)
     string(REGEX REPLACE "^.*PJ_VERSION ([0-9]+).*$" "\\1" PJ_VERSION "${proj_version}")

     # This will break if 4.10.0 ever will be released (highly unlikely)
     string(REGEX REPLACE "([0-9])([0-9])([0-9])" "\\1" PROJ_VERSION_MAJOR "${PJ_VERSION}")
     string(REGEX REPLACE "([0-9])([0-9])([0-9])" "\\2" PROJ_VERSION_MINOR "${PJ_VERSION}")
     string(REGEX REPLACE "([0-9])([0-9])([0-9])" "\\3" PROJ_VERSION_PATCH "${PJ_VERSION}")
     string(CONCAT PROJ_VERSION_STR "(" ${PROJ_VERSION_MAJOR} "." ${PROJ_VERSION_MINOR} "." ${PROJ_VERSION_PATCH} ")")
   endif()
 endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PROJ
  FOUND_VAR PROJ_FOUND
  REQUIRED_VARS PROJ_INCLUDE_DIRS PROJ_LIBRARIES
  VERSION_VAR PROJ_VERSION_STR
  FAIL_MESSAGE "Could NOT find proj")

if (PROJ_FOUND)
  MESSAGE(STATUS "Found PROJ: ${PROJ_LIBRARIES} (${PROJ_VERSION_MAJOR}.${PROJ_VERSION_MINOR}.${PROJ_VERSION_PATCH})")
  MESSAGE(STATUS "PROJ_INCLUDE_DIRS: ${PROJ_INCLUDE_DIRS}")
  mark_as_advanced(PROJ_INCLUDE_DIRS PROJ_LIBRARIES)
endif()
