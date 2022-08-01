# Find GEOS
# ~~~~~~~~~
# Taken from QGIS and made minimal modifications for MobilityDB,
# e.g., to allow 'dev' suffix in GEOS version
# Copyright (c) 2021, Esteban Zimanyi
#
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
#    GEOS_LIBRARY
#

IF(APPLE)
  FUNCTION (GET_VERSION_PLIST PLISTFILE OUTVAR)
    SET (PVERSION "")
    IF (EXISTS ${PLISTFILE})
      FILE (READ "${PLISTFILE}" info_plist)
      STRING (REGEX REPLACE "\n" "" info_plist "${info_plist}")
      STRING (REGEX MATCH "<key>CFBundleShortVersionString</key>[ \t]*<string>([0-9\\.]*)</string>" PLISTVERSION "${info_plist}")
      STRING (REGEX REPLACE "<key>CFBundleShortVersionString</key>[ \t]*<string>([0-9\\.]*)</string>" "\\1" PVERSION "${PLISTVERSION}")
    ENDIF (EXISTS ${PLISTFILE})
    SET (${OUTVAR} ${PVERSION} PARENT_SCOPE)
  ENDFUNCTION (GET_VERSION_PLIST)
ENDIF(APPLE)

IF(WIN32)

  IF (MINGW)
    FIND_PATH(GEOS_INCLUDE_DIR geos_c.h "$ENV{LIB_DIR}/include" /usr/local/include /usr/include c:/msys/local/include)
    FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c PATHS "$ENV{LIB_DIR}/lib" /usr/local/lib /usr/lib c:/msys/local/lib)
  ENDIF (MINGW)

  IF (MSVC)
    FIND_PATH(GEOS_INCLUDE_DIR geos_c.h $ENV{LIB_DIR}/include $ENV{INCLUDE})
    FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c_i geos_c PATHS
      "$ENV{LIB_DIR}/lib"
      $ENV{LIB}
      )
  ENDIF (MSVC)

ELSEIF(APPLE AND QGIS_MAC_DEPS_DIR)

    FIND_PATH(GEOS_INCLUDE_DIR geos_c.h "$ENV{LIB_DIR}/include" )
    FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c PATHS "$ENV{LIB_DIR}/lib" )

ELSE(WIN32)

 IF(UNIX)
    # try to use framework on mac
    # want clean framework path, not unix compatibility path
    IF (APPLE)
      IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
          OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
          OR NOT CMAKE_FIND_FRAMEWORK)
        SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
        SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
        FIND_LIBRARY(GEOS_LIBRARY GEOS)
        IF (GEOS_LIBRARY)
          # they're all the same in a framework
          SET (GEOS_INCLUDE_DIR ${GEOS_LIBRARY}/Headers CACHE PATH "Path to a file.")
          # set GEOS_CONFIG to make later test happy, not used here, may not exist
          SET (GEOS_CONFIG ${GEOS_LIBRARY}/unix/bin/geos-config CACHE FILEPATH "Path to a program.")
          # version in info.plist
          GET_VERSION_PLIST (${GEOS_LIBRARY}/Resources/Info.plist GEOS_VERSION)
          IF (NOT GEOS_VERSION)
            MESSAGE (FATAL_ERROR "Could not determine GEOS version from framework.")
          ENDIF (NOT GEOS_VERSION)
          STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\1" GEOS_VERSION_MAJOR "${GEOS_VERSION}")
          STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\2" GEOS_VERSION_MINOR "${GEOS_VERSION}")
          IF (GEOS_VERSION_MAJOR LESS 3)
            MESSAGE (FATAL_ERROR "GEOS version is too old (${GEOS_VERSION}). Use 3.0.0 or higher.")
          ENDIF (GEOS_VERSION_MAJOR LESS 3)
        ENDIF (GEOS_LIBRARY)
        SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
      ENDIF ()
    ENDIF (APPLE)

    IF(CYGWIN)
      FIND_LIBRARY(GEOS_LIBRARY NAMES geos_c PATHS /usr/lib /usr/local/lib)
    ENDIF(CYGWIN)

    IF (NOT GEOS_INCLUDE_DIR OR NOT GEOS_LIBRARY OR NOT GEOS_CONFIG)
      # didn't find OS X framework, and was not set by user
      SET(GEOS_CONFIG_PREFER_PATH "$ENV{GEOS_HOME}/bin" CACHE STRING "preferred path to GEOS (geos-config)")
      FIND_PROGRAM(GEOS_CONFIG geos-config
          ${GEOS_CONFIG_PREFER_PATH}
          $ENV{LIB_DIR}/bin
          /usr/local/bin/
          /usr/bin/
          )
      #MESSAGE("DBG GEOS_CONFIG ${GEOS_CONFIG}")

      IF (GEOS_CONFIG)

        EXEC_PROGRAM(${GEOS_CONFIG}
            ARGS --version
            OUTPUT_VARIABLE GEOS_VERSION)
        STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\1" GEOS_VERSION_MAJOR "${GEOS_VERSION}")
        STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\2" GEOS_VERSION_MINOR "${GEOS_VERSION}")

        IF (GEOS_VERSION_MAJOR LESS 3 OR (GEOS_VERSION_MAJOR EQUAL 3 AND GEOS_VERSION_MINOR LESS 3) )
          MESSAGE (FATAL_ERROR "GEOS version is too old (${GEOS_VERSION}). Use 3.3.0 or higher.")
        ENDIF (GEOS_VERSION_MAJOR LESS 3 OR (GEOS_VERSION_MAJOR EQUAL 3 AND GEOS_VERSION_MINOR LESS 3) )

        # set INCLUDE_DIR to prefix+include
        EXEC_PROGRAM(${GEOS_CONFIG}
            ARGS --prefix
            OUTPUT_VARIABLE GEOS_PREFIX)

        FIND_PATH(GEOS_INCLUDE_DIR
            geos_c.h
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
        #MESSAGE("DBG  GEOS_LINK_DIRECTORIES_WITH_PREFIX=${GEOS_LINK_DIRECTORIES_WITH_PREFIX}")

        ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES

        IF (GEOS_LINK_DIRECTORIES_WITH_PREFIX)
          STRING(REGEX REPLACE "[-][L]" "" GEOS_LINK_DIRECTORIES ${GEOS_LINK_DIRECTORIES_WITH_PREFIX} )
        ENDIF (GEOS_LINK_DIRECTORIES_WITH_PREFIX)

        ### XXX - mloskot: geos-config --libs does not return -lgeos_c, so set it manually
        ## split off the name
        ## use regular expression to match wildcard equivalent "-l*<endchar>"
        ## with <endchar> is a space or a semicolon
        #STRING(REGEX MATCHALL "[-][l]([^ ;])+"
        #  GEOS_LIB_NAME_WITH_PREFIX
        #  "${GEOS_CONFIG_LIBS}" )
        #MESSAGE("DBG  GEOS_CONFIG_LIBS=${GEOS_CONFIG_LIBS}")
        #MESSAGE("DBG  GEOS_LIB_NAME_WITH_PREFIX=${GEOS_LIB_NAME_WITH_PREFIX}")
        SET(GEOS_LIB_NAME_WITH_PREFIX -lgeos_c CACHE STRING INTERNAL)

        ## remove prefix -l because we need the pure name

        IF (GEOS_LIB_NAME_WITH_PREFIX)
          STRING(REGEX REPLACE "[-][l]" "" GEOS_LIB_NAME ${GEOS_LIB_NAME_WITH_PREFIX} )
        ENDIF (GEOS_LIB_NAME_WITH_PREFIX)
        #MESSAGE("DBG  GEOS_LIB_NAME=${GEOS_LIB_NAME}")

        IF (APPLE)
          IF (NOT GEOS_LIBRARY)
            # work around empty GEOS_LIBRARY left by framework check
            # while still preserving user setting if given
            # ***FIXME*** need to improve framework check so below not needed
            SET(GEOS_LIBRARY ${GEOS_LINK_DIRECTORIES}/lib${GEOS_LIB_NAME}.dylib CACHE STRING INTERNAL FORCE)
          ENDIF (NOT GEOS_LIBRARY)
        ELSE (APPLE)
          FIND_LIBRARY(GEOS_LIBRARY NAMES ${GEOS_LIB_NAME} PATHS ${GEOS_LIB_DIRECTORIES}/lib)
        ENDIF (APPLE)
        #MESSAGE("DBG  GEOS_LIBRARY=${GEOS_LIBRARY}")

      ELSE(GEOS_CONFIG)
        MESSAGE("FindGEOS.cmake: geos-config not found. Please set it manually. GEOS_CONFIG=${GEOS_CONFIG}")
      ENDIF(GEOS_CONFIG)
    ENDIF(NOT GEOS_INCLUDE_DIR OR NOT GEOS_LIBRARY OR NOT GEOS_CONFIG)
  ENDIF(UNIX)
ENDIF(WIN32)

IF(GEOS_INCLUDE_DIR AND NOT GEOS_VERSION)
  FILE(READ ${GEOS_INCLUDE_DIR}/geos_c.h VERSIONFILE)
  STRING(REGEX MATCH "#define GEOS_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+" GEOS_VERSION ${VERSIONFILE})
  STRING(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" GEOS_VERSION ${GEOS_VERSION})
  STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\1" GEOS_VERSION_MAJOR "${GEOS_VERSION}")
  STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)(dev)?" "\\2" GEOS_VERSION_MINOR "${GEOS_VERSION}")
ENDIF(GEOS_INCLUDE_DIR AND NOT GEOS_VERSION)

IF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)
   SET(GEOS_FOUND TRUE)
ENDIF (GEOS_INCLUDE_DIR AND GEOS_LIBRARY)

IF (GEOS_FOUND)

   IF (NOT GEOS_FIND_QUIETLY)
      MESSAGE(STATUS "Found GEOS: ${GEOS_LIBRARY} (${GEOS_VERSION})")
   ENDIF (NOT GEOS_FIND_QUIETLY)

ELSE (GEOS_FOUND)

   MESSAGE(GEOS_INCLUDE_DIR=${GEOS_INCLUDE_DIR})
   MESSAGE(GEOS_LIBRARY=${GEOS_LIBRARY})
   MESSAGE(FATAL_ERROR "Could not find GEOS")

ENDIF (GEOS_FOUND)

message(STATUS "GEOS_INCLUDE_DIR: ${GEOS_INCLUDE_DIR}")
message(STATUS "GEOS_LIBRARY: ${GEOS_LIBRARY}")
message(STATUS "GEOS_VERSION: ${GEOS_VERSION}")
message(STATUS "GEOS_VERSION_MAJOR: ${GEOS_VERSION_MAJOR}")
message(STATUS "GEOS_VERSION_MINOR: ${GEOS_VERSION_MINOR}")
