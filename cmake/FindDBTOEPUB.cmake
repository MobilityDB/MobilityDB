#
# (c)2021 Esteban Zimanyi
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

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args(DBTOEPUB
  FOUND_VAR DBTOEPUB_FOUND
  REQUIRED_VARS DBTOEPUB_COMMAND )
  # VERSION_VAR DBTOEPUB_VERSION )
