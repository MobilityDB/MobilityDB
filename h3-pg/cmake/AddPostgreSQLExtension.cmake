# Add pg_regress binary
find_program(PostgreSQL_REGRESS pg_regress
  HINTS
    "${PostgreSQL_PKG_LIBRARY_DIR}/pgxs/src/test/regress/"
    "${PostgreSQL_BIN_DIR}"
)

# Add pg_validate_extupgrade binary
find_program(PostgreSQL_VALIDATE_EXTUPGRADE pg_validate_extupgrade)

# Add pgindent binary
find_program(PostgreSQL_INDENT pgindent)

# Helper command to add extensions
function(PostgreSQL_add_extension LIBRARY_NAME)
  set(options RELOCATABLE)
  set(oneValueArgs NAME COMMENT VERSION COMPONENT)
  set(multiValueArgs REQUIRES SOURCES INSTALLS UPDATES)
  cmake_parse_arguments(EXTENSION "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Default extension name to same as library name
  if(NOT EXTENSION_NAME)
    set(EXTENSION_NAME ${LIBRARY_NAME})
  endif()

  # Allow extensions without sources
  if(EXTENSION_SOURCES)
    # Add extension as a dynamically linked library
    add_library(${LIBRARY_NAME} MODULE ${EXTENSION_SOURCES})

    # Link extension to PostgreSQL
    target_link_libraries(${LIBRARY_NAME} PRIVATE PostgreSQL::PostgreSQL)

    # Handle macOS specifics
    if(APPLE)
      # Fix apple missing symbols
      set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS ${PostgreSQL_LINK_FLAGS})

      # Since Postgres 16, the shared library extension on macOS is `dylib`, not `so`.
      # Ref https://github.com/postgres/postgres/commit/b55f62abb2c2e07dfae99e19a2b3d7ca9e58dc1a
      if (${PostgreSQL_VERSION_MAJOR} VERSION_GREATER_EQUAL "16")
        set_target_properties (${LIBRARY_NAME} PROPERTIES SUFFIX ".dylib")
      endif()
    endif()

    # Final touches on output file
    set_target_properties(${LIBRARY_NAME} PROPERTIES
      OUTPUT_NAME ${EXTENSION_NAME}
      INTERPROCEDURAL_OPTIMIZATION TRUE
      #C_VISIBILITY_PRESET hidden # @TODO: how to get this working?
      PREFIX "" # Avoid lib* prefix on output file
    )

    # Install .so/.dll to pkglib-dir
    install(
      TARGETS ${LIBRARY_NAME}
      LIBRARY DESTINATION "${PostgreSQL_PKG_LIBRARY_DIR}"
      COMPONENT ${EXTENSION_COMPONENT}
    )
  endif()

  # Generate .control file
  string(REPLACE ";" ", " EXTENSION_REQUIRES "${EXTENSION_REQUIRES}")
  configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/control.in
    ${EXTENSION_NAME}.control
  )

  # Generate .sql install file
  set(EXTENSION_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}--${EXTENSION_VERSION}.sql)
  file(WRITE "${EXTENSION_INSTALL}.in" "")
  foreach(file ${EXTENSION_INSTALLS})
    file(READ ${file} CONTENTS)
    file(APPEND "${EXTENSION_INSTALL}.in" "${CONTENTS}")
  endforeach()
  configure_file("${EXTENSION_INSTALL}.in" "${EXTENSION_INSTALL}" COPYONLY)

  # Install everything else into share-dir
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${EXTENSION_NAME}.control
      ${EXTENSION_INSTALL}
      ${EXTENSION_UPDATES}
    DESTINATION "${PostgreSQL_SHARE_DIR}/extension"
    COMPONENT ${EXTENSION_COMPONENT}
  )

  # Setup auto-format
  if(PostgreSQL_INDENT)
    add_custom_target("format_${EXTENSION_NAME}"
      COMMAND ${PostgreSQL_INDENT} ${EXTENSION_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Formatting ${EXTENSION_NAME} sources"
    )
    add_dependencies(${LIBRARY_NAME} "format_${EXTENSION_NAME}")
  endif()
endfunction()
