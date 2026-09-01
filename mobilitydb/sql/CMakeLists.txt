include(AssembleExtensionSql)

set(MOBILITYDB_MODULE_PATHNAME "$libdir/lib${MOBILITYDB_LIB_NAME}")

configure_file(mobilitydb.in.control ${CMAKE_BINARY_DIR}/mobilitydb.control)

# Version-gating markers a .in.sql source may use to include or exclude a block
# of SQL for a given PostgreSQL version. A source writes
#
#   #if POSTGRESQL_VERSION_NUMBER >= 170000
#   CREATE FUNCTION only_on_pg17 ...
#   #endif //POSTGRESQL_VERSION_NUMBER >= 170000
#
# and process_file turns the markers into plain SQL comments when the predicate
# holds, or into a /* ... */ block that comments the body out when it does not.
macro(process_file f srcdir bindir)
  file(READ ${srcdir}/${f}.in.sql CURR_CONTENTS)

  # The thresholds are the ones the file itself gates on.
  string(REGEX MATCHALL "#if POSTGRESQL_VERSION_NUMBER [<>=]+ [0-9]+"
    SQL_VERSION_GATES "${CURR_CONTENTS}")
  set(SQL_VERSION_THRESHOLDS)
  foreach(g ${SQL_VERSION_GATES})
    string(REGEX REPLACE "^.* ([0-9]+)$" "\\1" v "${g}")
    list(APPEND SQL_VERSION_THRESHOLDS ${v})
  endforeach()
  list(REMOVE_DUPLICATES SQL_VERSION_THRESHOLDS)

  foreach(v ${SQL_VERSION_THRESHOLDS})
    if(${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL ${v})
      set(KEEP ">=")
      set(DROP "<")
    else()
      set(KEEP "<")
      set(DROP ">=")
    endif()

    # Predicate holds: comment out the markers, leaving the body active.
    string(REPLACE "#if POSTGRESQL_VERSION_NUMBER ${KEEP} ${v}"
      "-- if POSTGRESQL_VERSION_NUMBER ${KEEP} ${v}" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REPLACE "#endif //POSTGRESQL_VERSION_NUMBER ${KEEP} ${v}"
      "-- endif POSTGRESQL_VERSION_NUMBER ${KEEP} ${v}" CURR_CONTENTS "${CURR_CONTENTS}")

    # Predicate fails: open a C-style comment at #if and close it at #endif so
    # that the body between them is commented out.
    string(REPLACE "#if POSTGRESQL_VERSION_NUMBER ${DROP} ${v}"
      "/* -- if POSTGRESQL_VERSION_NUMBER ${DROP} ${v}" CURR_CONTENTS "${CURR_CONTENTS}")
    string(REPLACE "#endif //POSTGRESQL_VERSION_NUMBER ${DROP} ${v}"
      "-- endif POSTGRESQL_VERSION_NUMBER ${DROP} ${v} */" CURR_CONTENTS "${CURR_CONTENTS}")
  endforeach()

  # >= and < span every integer comparison, so a marker spelled any other way
  # is one the loop above left in place, and `#if` is not SQL.
  string(REGEX MATCH "#(if|endif //)POSTGRESQL_VERSION_NUMBER[^\n]*" LEFTOVER
    "${CURR_CONTENTS}")
  if(LEFTOVER)
    message(FATAL_ERROR
      "${srcdir}/${f}.in.sql: unhandled version marker '${LEFTOVER}'. "
      "Gate on '>=' or '<': '> N' is '>= N+1' and '<= N' is '< N+1'.")
  endif()

  file(WRITE ${bindir}/${f}.sql "${CURR_CONTENTS}")
  add_sql_configure_depends(${srcdir}/${f}.in.sql)
endmacro()

add_subdirectory(temporal)
add_subdirectory(geo)
if(CBUFFER)
  add_subdirectory(cbuffer)
endif()
if(H3)
  add_subdirectory(h3)
endif()
if(JSON)
  add_subdirectory(json)
endif()
if(NPOINT)
  add_subdirectory(npoint)
endif()
if(POINTCLOUD)
  add_subdirectory(pointcloud)
endif()
if(POSE)
  add_subdirectory(pose)
  add_subdirectory(posechain)
endif()
if(QUADBIN)
  add_subdirectory(quadbin)
endif()
if(RASTER)
  add_subdirectory(raster)
endif()
if(RGEO)
  add_subdirectory(rgeo)
endif()
if(S2CELL)
  add_subdirectory(s2cell)
endif()

# Load order is significant: types must be created before the operators over
# them, and 999_oid_cache must come last. It is encoded in the numeric filename
# prefixes, with a disjoint range per module, and every module stages into the
# same directory, so sorting the paths orders the sources by that prefix.
list(SORT PROJECT_SQL_FILES)

if (PROJECT_VERBOSE)
  message(STATUS "PROJECT_SQL_FILES=${PROJECT_SQL_FILES}")
endif()

configure_file(header.in.sql ${CMAKE_CURRENT_BINARY_DIR}/header.sql @ONLY)

assemble_extension_sql(
  OUTPUT ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}
  HEADER ${CMAKE_CURRENT_BINARY_DIR}/header.sql
  BASE_DIR ${CMAKE_BINARY_DIR}/mobilitydb/sql
  SOURCES ${PROJECT_SQL_FILES})
