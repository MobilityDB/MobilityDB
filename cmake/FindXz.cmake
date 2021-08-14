# This module looks for xz. This module defines the following variables:
#
#  XZ_EXECUTABLE
#  XZCAT_EXECUTABLE
#  XZ_FOUND
#
# Copyright (c) 2021 Esteban Zimanyi
#
# Derived from
# https://github.com/WebKit/webkit/blob/main/Source/cmake/FindXz.cmake
#

include(FindCygwin)
# include(FindMsys.cmake) # Available in cmake version 3.21

find_program(XZ_EXECUTABLE
  xz
  ${CYGWIN_INSTALL_PATH}/bin
  # ${MSYS_INSTALL_PATH}/usr/bin # Available in cmake version 3.21
)
find_program(XZCAT_EXECUTABLE
  xzcat
  ${CYGWIN_INSTALL_PATH}/bin
  # ${MSYS_INSTALL_PATH}/usr/bin # Available in cmake version 3.21
)

# Handle the QUIETLY and REQUIRED arguments and set XZ_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Xz DEFAULT_MSG XZ_EXECUTABLE XZCAT_EXECUTABLE)

mark_as_advanced(XZ_EXECUTABLE XZCAT_EXECUTABLE)

message(STATUS "XZ_EXECUTABLE: ${XZ_EXECUTABLE}")
message(STATUS "XZCAT_EXECUTABLE: ${XZCAT_EXECUTABLE}")
