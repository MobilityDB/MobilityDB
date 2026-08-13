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
# Add a threshold here when a source starts gating on one; thresholds at or
# below PG_MIN_MAJOR_VERSION are unconditionally satisfied and need no entry.
set(SQL_VERSION_THRESHOLDS 170000 180000)

macro(process_file f srcdir bindir)
  file(READ ${srcdir}/${f}.in.sql CURR_CONTENTS)

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
