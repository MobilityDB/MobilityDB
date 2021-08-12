#
# (c)2021 Esteban Zimanyi based on FindDBLATEX.cmake
# https://github.com/KiCad/kicad-doc/blob/master/CMakeModules/FindDBLATEX.cmake
#
# CMake module to find dbtoepub.
#
# Variables generated:
#
# DBTOEPUB_FOUND     true when DBTOEPUB_COMMAND and DBTOEPUB_VERSION have valid entries
# DBTOEPUB_COMMAND   The command to run dbtoepub
# DBTOEPUB_VERSION   The DBTOEPUB version that has been found -> NOT AVAILABLE
# N.B. Currently dbtoepub DOES NOT provide version number
#

# Have a go at finding a dbtoepub executable (valid for POSIX systems, not for WIN systems)
find_program( DBTOEPUB_COMMAND dbtoepub )

# If nothing could be found, test to see if we can just find the file, that we'll then run with the
# python interpreter
if( NOT DBTOEPUB_COMMAND )
    find_file( DBTOEPUB_COMMAND dbtoepub )
    message( STATUS "find_file( DBTOEPUB_COMMAND dbtoepub ): ${DBTOEPUB_COMMAND}" )
endif()

# find_program finds the dbtoepub python script on Windows (bad CMake!) and defines it as an
# executable. This is not true and so we must test the executable by running it and testing the
# output
execute_process(
    COMMAND ${DBTOEPUB_COMMAND} --version
    OUTPUT_VARIABLE _OUT
    ERROR_VARIABLE _ERR
    RESULT_VARIABLE _RES
    OUTPUT_STRIP_TRAILING_WHITESPACE )

# If we've found an executable dbtoepub, check the version
# if( DBTOEPUB_COMMAND )
    # string(REGEX REPLACE ".*version[^0-9.]*\([0-9.]+\).*" "\\1" DBTOEPUB_VERSION "${_OUT}")
# endif()

# Generate the DBTOEPUB_FOUND as necessary, etc.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    DBTOEPUB
    REQUIRED_VARS DBTOEPUB_COMMAND )
    # VERSION_VAR DBTOEPUB_VERSION )
