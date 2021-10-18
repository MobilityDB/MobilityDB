#
# (c)2015 KiCad Developers
# (c)2015 Brian Sidebotham <brian.sidebotham@gmail.com>
#
# CMake module to find dblatex. NOTE: on some systems dblatex is available relative to the python
# executable and there's no direct binary that we can find, so we also try to find python and
# discover the dblatex "script" from there
#
# Variables generated:
#
# DBLATEX_FOUND     true when DBLATEX_COMMAND and DBLATEX_VERSION have valid entries
# DBLATEX_COMMAND   The command to run dblatex (may be a list including an interpreter)
# DBLATEX_VERSION   The DBLATEX version that has been found
#

# Have a go at finding a dblatex executable (valid for POSIX systems, not for WIN systems)
find_program( DBLATEX_COMMAND dblatex )

# If nothing could be found, test to see if we can just find the file, that we'll then run with the
# python interpreter
if( NOT DBLATEX_COMMAND )
    find_file( DBLATEX_COMMAND dblatex )
    message( STATUS "find_file( DBLATEX_COMMAND dblatex ): ${DBLATEX_COMMAND}" )
endif()

# find_program finds the dblatex python script on Windows (bad CMake!) and defines it as an
# executable. This is not true and so we must test the executable by running it and testing the
# output
execute_process(
    COMMAND ${DBLATEX_COMMAND} --version
    OUTPUT_VARIABLE _OUT
    ERROR_VARIABLE _ERR
    RESULT_VARIABLE _RES
    OUTPUT_STRIP_TRAILING_WHITESPACE )

# Check to see if the program is executable. If it isn't we'll attempt to run it via the python
# interpreter
if( NOT _RES MATCHES 0 )

    # Find the python interpreter quietly
    if( NOT PYTHONINTERP_FOUND )
        find_package( PYTHONINTERP QUIET )
    endif()

    if( NOT PYTHONINTERP_FOUND )
        # Python's not available so can't find dblatex...
        set( DBLATEX_COMMAND "" )
    else()
        # Try running the program with the python interpreter
        set( DBLATEX_COMMAND ${PYTHON_EXECUTABLE} ${DBLATEX_COMMAND} )

        execute_process(
            COMMAND ${DBLATEX_COMMAND} --version
            OUTPUT_VARIABLE _OUT
            ERROR_VARIABLE _ERR
            RESULT_VARIABLE _RES
            OUTPUT_STRIP_TRAILING_WHITESPACE )

        # If it still can't be run, attempt to find dblatex relative to
        # the python interpreter (Works on Windows, but no need to guard)
        if( NOT _RES MATCHES 0 )
            get_filename_component( PYTHON_DIR ${PYTHON_EXECUTABLE} DIRECTORY )
            set( DBLATEX_COMMAND ${PYTHON_EXECUTABLE} ${PYTHON_DIR}/Scripts/dblatex )
            execute_process(
                COMMAND ${DBLATEX_COMMAND} --version
                OUTPUT_VARIABLE _OUT
                ERROR_VARIABLE _ERR
                RESULT_VARIABLE _RES
                OUTPUT_STRIP_TRAILING_WHITESPACE )
            if( NOT _RES MATCHES 0 )
                set( DBLATEX_COMMAND "" )
            endif()
        endif()
    endif()
endif()

# If we've found an executable dblatex, check the version
if( DBLATEX_COMMAND )
    string(REGEX REPLACE ".*version[^0-9.]*\([0-9.]+\).*" "\\1" DBLATEX_VERSION "${_OUT}")
endif()

# Generate the DBLATEX_FOUND as necessary, etc.
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(
    DBLATEX
    REQUIRED_VARS DBLATEX_COMMAND
    VERSION_VAR DBLATEX_VERSION )
