# Find GEOS
# ~~~~~~~~~
# Copyright (c) 2008, Mateusz Loskot <mateusz@loskot.net>
# (based on FindGDAL.cmake by Magnus Homann)
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for GEOS library
#
# If it's found it sets GEOS_FOUND to TRUE
# and following variables are set:
#    GEOS_INCLUDE_DIR
#    GEOS_INCLUDE_DIRS
#    GEOS_LIBRARY
#    GEOS_LIBRARIES

IF (NOT DEFINED GEOS_INCLUDE_DIR OR NOT DEFINED GEOS_LIBRARY OR NOT DEFINED GEOS_LIBRARIES OR NOT DEFINED GEOS_CONFIG)
  UNSET(GEOS_INCLUDE_DIR CACHE)
  UNSET(GEOS_INCLUDE_DIRS CACHE)
  UNSET(GEOS_LIBRARY CACHE)
  UNSET(GEOS_LIBRARIES CACHE)
  UNSET(GEOS_CONFIG CACHE)

  SET(GEOS_CONFIG_PREFER_PATH "$ENV{GEOS_HOME}/bin" CACHE STRING "preferred path to GEOS (geos-config)")
  FIND_PROGRAM(GEOS_CONFIG geos-config
    ${GEOS_CONFIG_PREFER_PATH}
    /usr/local/bin/
    /usr/bin/
    )

  IF (DEFINED GEOS_CONFIG)

    EXEC_PROGRAM(${GEOS_CONFIG}
      ARGS --version
      OUTPUT_VARIABLE GEOS_VERSION)
    STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" GEOS_VERSION_MAJOR "${GEOS_VERSION}")
    STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" GEOS_VERSION_MINOR "${GEOS_VERSION}")

    IF (GEOS_VERSION_MAJOR LESS 3 OR (GEOS_VERSION_MAJOR EQUAL 3 AND GEOS_VERSION_MINOR LESS 1) )
      MESSAGE (FATAL_ERROR "GEOS version is too old (${GEOS_VERSION}). Use 3.1.0 or higher.")
    ENDIF (GEOS_VERSION_MAJOR LESS 3 OR (GEOS_VERSION_MAJOR EQUAL 3 AND GEOS_VERSION_MINOR LESS 1) )

    # set INCLUDE_DIR to prefix+include
    EXEC_PROGRAM(${GEOS_CONFIG}
      ARGS --prefix
      OUTPUT_VARIABLE GEOS_PREFIX)

    FIND_PATH(GEOS_INCLUDE_DIR
      geos/geom.h
      PATHS
      ${GEOS_PREFIX}/include
      /usr/local/include
      /usr/include
      )

    ## extract link dirs for rpath
    EXEC_PROGRAM(${GEOS_CONFIG}
      ARGS --libs
      OUTPUT_VARIABLE GEOS_CONFIG_LIBS )

    ## split off the link dirs (for rpath)
    ## use regular expression to match wildcard equivalent "-L*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "[-][L]([^ ;])+"
      GEOS_LINK_DIRECTORIES_WITH_PREFIX
      "${GEOS_CONFIG_LIBS}" )


    ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES

    IF (GEOS_LINK_DIRECTORIES_WITH_PREFIX)
      STRING(REGEX REPLACE "^-L(.*)" "\\1" GEOS_LINK_DIRECTORIES ${GEOS_LINK_DIRECTORIES_WITH_PREFIX} )
    ENDIF (GEOS_LINK_DIRECTORIES_WITH_PREFIX)


    ## use regular expression to match wildcard equivalent "-l*<endchar>"
    ## with <endchar> is a space or a semicolon
    STRING(REGEX MATCHALL "(^| )-l([^ ;])+"
      GEOS_LIB_NAME_WITH_PREFIX
      "${GEOS_CONFIG_LIBS}" )

    ## remove prefix -l because we need the pure name
    IF (GEOS_LIB_NAME_WITH_PREFIX)
      STRING(REGEX REPLACE "(^ )-l(.*)" "\\2" GEOS_LIBRARY ${GEOS_LIB_NAME_WITH_PREFIX} )
    ENDIF (GEOS_LIB_NAME_WITH_PREFIX)

  ELSE(DEFINED GEOS_CONFIG)
    MESSAGE("FindGEOS.cmake: geos-config not found. Please set it manually. GEOS_CONFIG=${GEOS_CONFIG}")
  ENDIF(DEFINED GEOS_CONFIG)
ENDIF(NOT DEFINED GEOS_INCLUDE_DIR OR NOT DEFINED GEOS_LIBRARY OR NOT DEFINED GEOS_LIBRARIES OR NOT DEFINED GEOS_CONFIG)

IF(DEFINED GEOS_INCLUDE_DIR AND NOT DEFINED GEOS_VERSION)
  FILE(READ ${GEOS_INCLUDE_DIR}/geos_c.h VERSIONFILE)
  STRING(REGEX MATCH "#define GEOS_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+(dev)?\"" GEOS_VERSION ${VERSIONFILE})
  STRING(REGEX MATCH "[0-9]+\\.[0-9]\\.[0-9]+" GEOS_VERSION ${GEOS_VERSION})
ENDIF(DEFINED GEOS_INCLUDE_DIR AND NOT DEFINED GEOS_VERSION)

IF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
  SET(GEOS_FOUND TRUE)
  SET(GEOS_LIBRARIES ${GEOS_LIBRARY})
  SET(GEOS_INCLUDE_DIRS ${GEOS_INCLUDE_DIR})
ENDIF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)

IF (GEOS_FOUND)

  IF (NOT GEOS_FIND_QUIETLY)
    MESSAGE(STATUS "Found GEOS: ${GEOS_LIBRARY} (version: ${GEOS_VERSION}, includes: ${GEOS_INCLUDE_DIRS})")
  ENDIF (NOT GEOS_FIND_QUIETLY)

ELSE (GEOS_FOUND)

  MESSAGE(GEOS_INCLUDE_DIR=${GEOS_INCLUDE_DIR})
  MESSAGE(GEOS_LIBRARY=${GEOS_LIBRARY})
  MESSAGE(FATAL_ERROR "Could not find GEOS.  Perhaps try:\n$ sudo apt-get install libgeos++-dev\n")

ENDIF (GEOS_FOUND)
