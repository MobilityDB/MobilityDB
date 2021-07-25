# - Find json-c
# Find the PostgreSQL includes and client library
# This module defines
#  JSON_INCLUDE_DIR
#  JSON_LIBRARIES
#
# Copyright (c) 2021, Vicky Vergara <vicky@georepublic.org>

find_package (PkgConfig)
pkg_check_modules(JSONC_PKG json-c)

if (JSONC_PKG_FOUND)
    set (JSONC_LIBRARIES ${JSONC_PKG_LIBRARIES})
    set (JSONC_INCLUDE_DIRS ${JSONC_PKG_INCLUDE_DIRS})
else ()
    find_library(JSONC_LIBRARIES
        NAMES json-c json
        HINTS /lib /lib64 /usr/lib /usr/lib64
    )

    find_path(JSONC_INCLUDE_DIRS
        NAMES json.h
        HINTS /usr/include/json /usr/include/json-c
        DOC "json-c headers"
    )
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(json-c
    DEFAULT_MSG JSONC_LIBRARIES JSONC_INCLUDE_DIRS
)
mark_as_advanced(JSONC_INCLUDE_DIRS JSONC_LIBRARIES)
