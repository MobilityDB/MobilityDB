#
# Based on:
# https://majewsky.wordpress.com/2010/08/14/tip-of-the-day-cmake-and-doxygen/
#

option(DOC_DEV
  "Set ON|OFF (default=OFF) to build the developer's documentation" OFF)

if(DOC_DEV)
  message(STATUS "Build the developer's documentation.")
  find_package(Doxygen ${DOXYGEN_MINIMUM_VERSION})

  if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/doxygen/Doxyfile
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc_dev
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating the developer's documentation with Doxygen" VERBATIM
    )
  endif()

endif()
