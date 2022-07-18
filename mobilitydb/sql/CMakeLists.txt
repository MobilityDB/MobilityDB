set(MOBILITYDB_MODULE_PATHNAME "$libdir/lib${MOBILITYDB_LIB_NAME}")
set(extschema "@extschema@")

configure_file(mobilitydb.control ${CMAKE_BINARY_DIR}/mobilitydb.control)

# one word definitions
if (${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL 110000)
  set(JOIN_LE "scalarlejoinsel")
  set(JOIN_GE "scalargejoinsel")

  set(SCALAR_LE "scalarlesel")
  set(SCALAR_GE "scalargesel")

else()
  set(JOIN_LE "scalarltjoinsel")
  set(JOIN_GE "scalargtjoinsel")

  set(SCALAR_LE "scalarltsel")
  set(SCALAR_GE "scalargtsel")
endif()

macro(process_file f srcdir bindir)
  file(READ ${srcdir}/${f}.in.sql CURR_CONTENTS)

  if (${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL 120000)
    string(REGEX REPLACE "#if POSTGRESQL_VERSION_NUMBER >= 120000" "-- if POSTGRESQL_VERSION_NUMBER >= 120000" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REGEX REPLACE "#endif //POSTGRESQL_VERSION_NUMBER >= 120000" "-- endif POSTGRESQL_VERSION_NUMBER >= 120000" CURR_CONTENTS "${CURR_CONTENTS}")

    string(REGEX REPLACE "#if POSTGRESQL_VERSION_NUMBER < 120000" "/* --if POSTGRESQL_VERSION_NUMBER < 120000" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REGEX REPLACE "#endif //POSTGRESQL_VERSION_NUMBER < 120000" "-- endif POSTGRESQL_VERSION_NUMBER < 120000 */" CURR_CONTENTS "${CURR_CONTENTS}")
  else()
    string(REGEX REPLACE "#if POSTGRESQL_VERSION_NUMBER >= 120000" "/* -- if POSTGRESQL_VERSION_NUMBER >= 120000" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REGEX REPLACE "#endif //POSTGRESQL_VERSION_NUMBER >= 120000" "-- endif POSTGRESQL_VERSION_NUMBER >= 120000 */" CURR_CONTENTS "${CURR_CONTENTS}")

    string(REGEX REPLACE "#if POSTGRESQL_VERSION_NUMBER < 120000" " -- if POSTGRESQL_VERSION_NUMBER >= 120000" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REGEX REPLACE "#endif //POSTGRESQL_VERSION_NUMBER < 120000" "-- endif POSTGRESQL_VERSION_NUMBER >= 120000" CURR_CONTENTS "${CURR_CONTENTS}")
  endif()

  file(WRITE ${bindir}/${f}.sql.in "${CURR_CONTENTS}")
endmacro()


add_subdirectory(general)
add_subdirectory(point)
if(NPOINT)
  add_subdirectory(npoint)
endif()

list(SORT PROJECT_SQL_FILES)

add_custom_target(all_sql_files ALL
  DEPENDS ${PROJECT_SQL_FILES})

if (PROJECT_VERBOSE)
  message(STATUS "PROJECT_SQL_FILES=${PROJECT_SQL_FILES}")
endif()

foreach (f ${PROJECT_SQL_FILES})
  file(READ ${f} CURR_CONTENTS)
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}.in "${CURR_CONTENTS}")
endforeach()

configure_file(${CMAKE_CURRENT_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}.in ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE} COPYONLY)

file(READ "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}" sql_commands)
string(REGEX REPLACE "MODULE_PATHNAME" "${CMAKE_BINARY_DIR}/lib${MOBILITYDB_LIB_NAME}.so" sql_commands "${sql_commands}")
message(STATUS "Substitution: ${CMAKE_BINARY_DIR}/lib${MOBILITYDB_LIB_NAME}.so")
string(REGEX REPLACE "@extschema@" "public" sql_commands "${sql_commands}")
file(WRITE ${MOBILITYDB_TEST_EXTENSION_FILE}.in "${sql_commands}")
configure_file(${MOBILITYDB_TEST_EXTENSION_FILE}.in ${MOBILITYDB_TEST_EXTENSION_FILE} COPYONLY)
