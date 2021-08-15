#
# Based on:
# https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
#

option(DOXY_DOC
  "Set ON|OFF (default=OFF) to build the developer's documentation" OFF)

if(DOXY_DOC)
  message(STATUS "Build the developer's documentation.")
  find_package(Doxygen ${DOXYGEN_MINIMUM_VERSION})

  if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doxy
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating the developer's documentation with Doxygen" VERBATIM
    )
  endif()

endif()
