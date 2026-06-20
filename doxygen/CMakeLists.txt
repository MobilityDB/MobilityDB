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
    # Generate phantom C decls for every SQL function so Doxygen's
    # @sqlfn alias expansion autolinks into a symbol page listing the
    # SQL signatures and the C backing function. See
    # scripts/sql_to_doxygen_stubs.py for the full rationale.
    set(SQL_STUB_DIR ${CMAKE_CURRENT_BINARY_DIR}/sql_stubs)
    set(SQL_STUB_FILE ${SQL_STUB_DIR}/sql_phantom_decls.c)
    file(GLOB_RECURSE SQL_SRC_FILES
      CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/mobilitydb/sql/*.in.sql")
    add_custom_command(
      OUTPUT ${SQL_STUB_FILE}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${SQL_STUB_DIR}
      COMMAND python3
        ${CMAKE_SOURCE_DIR}/scripts/sql_to_doxygen_stubs.py
        --sql-dir ${CMAKE_SOURCE_DIR}/mobilitydb/sql
        --collision-dir
          ${CMAKE_SOURCE_DIR}/meos/include
          ${CMAKE_SOURCE_DIR}/meos/src
          ${CMAKE_SOURCE_DIR}/mobilitydb/pg_include
          ${CMAKE_SOURCE_DIR}/mobilitydb/src
        --out-dir ${SQL_STUB_DIR}
      DEPENDS
        ${CMAKE_SOURCE_DIR}/scripts/sql_to_doxygen_stubs.py
        ${SQL_SRC_FILES}
      COMMENT "Generating SQL phantom declarations for Doxygen autolink"
      VERBATIM
    )
    add_custom_target(sql_stubs DEPENDS ${SQL_STUB_FILE})

    configure_file(${CMAKE_SOURCE_DIR}/doxygen/Doxyfile
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc_dev
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS sql_stubs
      COMMENT "Generating the developer's documentation with Doxygen" VERBATIM
    )
  endif()

endif()
