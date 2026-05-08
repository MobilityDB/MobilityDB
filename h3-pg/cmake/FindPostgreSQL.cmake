# Based on:
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html#find-modules
# ----------------------------------------------------------------------------

# https://www.postgresql.org/support/versioning/
set(PostgreSQL_SUPPORTED_VERSIONS ${PostgreSQL_ADDITIONAL_VERSIONS}
  "18" "17" "16" "15" "14" "13")

# Use `FIND_VERSION` to locate specific version of pg_config, or fallback to known versions
if(PostgreSQL_FIND_VERSION)
  list(PREPEND PostgreSQL_SUPPORTED_VERSIONS "${PostgreSQL_FIND_VERSION_MAJOR}")
endif()

foreach(suffix ${PostgreSQL_SUPPORTED_VERSIONS})
  if(WIN32)
    list(APPEND PostgreSQL_CONFIG_PATH_SUFFIXES
      "PostgreSQL/${suffix}/bin")
  endif()
  if(UNIX)
    list(APPEND PostgreSQL_CONFIG_HINTS
      "/usr/lib/postgresql/${suffix}/bin")
  endif()
endforeach()

# Configuration will be based on values gathered from `pg_config`
find_program(PostgreSQL_CONFIG pg_config REQUIRED
  HINTS ${PostgreSQL_CONFIG_HINTS}
  PATH_SUFFIXES ${PostgreSQL_CONFIG_PATH_SUFFIXES}
)

# Grab information about the installed version of PostgreSQL
execute_process(COMMAND ${PostgreSQL_CONFIG} --version           OUTPUT_VARIABLE PostgreSQL_VERSION            OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --bindir            OUTPUT_VARIABLE PostgreSQL_BIN_DIR            OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --sharedir          OUTPUT_VARIABLE PostgreSQL_SHARE_DIR          OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --includedir        OUTPUT_VARIABLE PostgreSQL_INCLUDE_DIR        OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --pkgincludedir     OUTPUT_VARIABLE PostgreSQL_PKG_INCLUDE_DIR    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --includedir-server OUTPUT_VARIABLE PostgreSQL_SERVER_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --libdir            OUTPUT_VARIABLE PostgreSQL_LIBRARY_DIR        OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${PostgreSQL_CONFIG} --pkglibdir         OUTPUT_VARIABLE PostgreSQL_PKG_LIBRARY_DIR    OUTPUT_STRIP_TRAILING_WHITESPACE)

# @TODO: Figure out if we need _INCLUDE_DIR and/or _PKG_INCLUDE_DIR

# Create include dirs list
list(APPEND PostgreSQL_INCLUDE_DIRS "${PostgreSQL_INCLUDE_DIR}" "${PostgreSQL_PKG_INCLUDE_DIR}" "${PostgreSQL_SERVER_INCLUDE_DIR}")
list(APPEND PostgreSQL_LIBRARY_DIRS "${PostgreSQL_LIBRARY_DIR}")

# Set library to search for (which is different on WIN32)
set(FIND_LIBRARY pq)

# Platform fixes: Windows
# https://wiki.postgresql.org/wiki/Building_and_Installing_PostgreSQL_Extension_Modules
if(WIN32)
  set(FIND_LIBRARY postgres)
  list(APPEND PostgreSQL_INCLUDE_DIRS "${PostgreSQL_SERVER_INCLUDE_DIR}/port/win32")
  if(MSVC)
    list(APPEND PostgreSQL_INCLUDE_DIRS "${PostgreSQL_SERVER_INCLUDE_DIR}/port/win32_msvc")
  endif(MSVC)
endif(WIN32)

# Platform fixes: MacOS
# https://github.com/postgres/postgres/blob/master/src/makefiles/Makefile.darwin
set(PostgreSQL_LINK_FLAGS "")
# @TODO: Find out if this can be done in a more cmakey way
if(APPLE)
  find_program(PostgreSQL_EXECUTABLE postgres REQUIRED NO_DEFAULT_PATH PATHS ${PostgreSQL_BIN_DIR})
  set(PostgreSQL_LINK_FLAGS "-bundle_loader ${PostgreSQL_EXECUTABLE}")
endif()

# ----------------------------------------------------------------------------
# Now we need to find the libraries and include files
# if the library is available with multiple configurations, you can use SelectLibraryConfigurations
# to automatically set the _LIBRARY variable:

# @TODO: Find out if we can actually find any debug libraries on each platform

find_library(PostgreSQL_LIBRARY_RELEASE
  NAMES ${FIND_LIBRARY}
  PATHS ${PostgreSQL_LIBRARY_DIRS}
)

find_library(PostgreSQL_LIBRARY_DEBUG
  NAMES ${FIND_LIBRARY}d # <-- debug
  PATHS ${PostgreSQL_LIBRARY_DIRS}
)

include(SelectLibraryConfigurations)
select_library_configurations(PostgreSQL)

# ----------------------------------------------------------------------------
# If you have a good way of getting the version (from a header file, for example),
# you can use that information to set Foo_VERSION (although note that find modules have
# traditionally used Foo_VERSION_STRING, so you may want to set both).

string(REGEX MATCHALL "[0-9]+" PostgreSQL_VERSION_LIST "${PostgreSQL_VERSION}")
list(GET PostgreSQL_VERSION_LIST 0 PostgreSQL_VERSION_MAJOR)
list(GET PostgreSQL_VERSION_LIST 1 PostgreSQL_VERSION_MINOR)
set(PostgreSQL_VERSION "${PostgreSQL_VERSION_MAJOR}.${PostgreSQL_VERSION_MINOR}")

# ----------------------------------------------------------------------------
# Handle additional components

if("PostGIS" IN_LIST PostgreSQL_FIND_COMPONENTS)
  find_library(
    POSTGIS
    NAMES postgis postgis-3 postgis-3.so
    PATHS ${PostgreSQL_PKG_LIBRARY_DIR}
  )

  set(PostgreSQL_PostGIS_FOUND TRUE)
  if(NOT POSTGIS)
    set(PostgreSQL_PostGIS_FOUND FALSE)
  endif()
endif()

# ----------------------------------------------------------------------------
# Now we can use FindPackageHandleStandardArgs to do
# most of the rest of the work for us
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PostgreSQL
  REQUIRED_VARS
    PostgreSQL_LIBRARY
    PostgreSQL_INCLUDE_DIRS
  HANDLE_COMPONENTS
  VERSION_VAR PostgreSQL_VERSION
)

# ----------------------------------------------------------------------------
# At this point, we have to provide a way for users of the find module to link to the library or libraries that were found.
#
# If the library is available with multiple configurations, the IMPORTED_CONFIGURATIONS target property should also be populated:

if(PostgreSQL_FOUND)
  # When providing imported targets, these should be namespaced (hence the Foo:: prefix);
  # CMake will recognize that values passed to target_link_libraries() that contain :: in their name
  # are supposed to be imported targets (rather than just library names),
  if (NOT TARGET PostgreSQL::PostgreSQL)
    add_library(PostgreSQL::PostgreSQL UNKNOWN IMPORTED)
  endif()
  
  # The RELEASE variant should be listed first in the property so that the variant is chosen
  # if the user uses a configuration which is not an exact match for any listed IMPORTED_CONFIGURATIONS.
  if (PostgreSQL_LIBRARY_RELEASE)
    set_property(TARGET PostgreSQL::PostgreSQL APPEND PROPERTY
      IMPORTED_CONFIGURATIONS RELEASE
    )
    set_target_properties(PostgreSQL::PostgreSQL PROPERTIES
      IMPORTED_LOCATION_RELEASE "${PostgreSQL_LIBRARY_RELEASE}"
    )
  endif()

  if (PostgreSQL_LIBRARY_DEBUG)
    set_property(TARGET PostgreSQL::PostgreSQL APPEND PROPERTY
      IMPORTED_CONFIGURATIONS DEBUG
    )
    set_target_properties(PostgreSQL::PostgreSQL PROPERTIES
      IMPORTED_LOCATION_DEBUG "${PostgreSQL_LIBRARY_DEBUG}"
    )
  endif()

  set_target_properties(PostgreSQL::PostgreSQL PROPERTIES
    #INTERFACE_COMPILE_OPTIONS "${PostgreSQL_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${PostgreSQL_INCLUDE_DIRS}"
  )
endif()

# Most of the cache variables should be hidden in the `ccmake` interface unless the user explicitly asks to edit them.
mark_as_advanced(
  PostgreSQL_INCLUDE_DIRS
  PostgreSQL_LIBRARY
)

# ----------------------------------------------------------------------------

# Traditional variables (from pkg-config):
#
# _FOUND            has the package been found
# _LIBRARIES        ?
# _LINK_LIBRARIES   ?
# _LIBRARY_DIRS     ?
# _LDFLAGS          ?
# _LDFLAGS_OTHER    ?
# _INCLUDE_DIRS     ?
# _CFLAGS           ?
# _CFLAGS_OTHER     required non-include-dir CFLAGS to stdout
# 
# _VERSION          full version of found package
# _PREFIX           ?
# _INCLUDEDIR       ?
# _LIBDIR           ?

# debug
message(STATUS "Find version:  ${PostgreSQL_FIND_VERSION}")
message(STATUS "Find major:    ${PostgreSQL_FIND_VERSION_MAJOR}")
message(STATUS "Find minor:    ${PostgreSQL_FIND_VERSION_MINOR}")

message(STATUS "Found version: ${PostgreSQL_VERSION}")
message(STATUS "Found major:   ${PostgreSQL_VERSION_MAJOR}")
message(STATUS "Found minor:   ${PostgreSQL_VERSION_MINOR}")

# Get all variables
GET_CMAKE_PROPERTY(vars VARIABLES)
FOREACH(var ${vars})
  STRING(REGEX MATCH "^PostgreSQL_" item ${var})
  IF(item)
    message(STATUS "${var} = ${${var}}")
  ENDIF(item)
ENDFOREACH(var)
