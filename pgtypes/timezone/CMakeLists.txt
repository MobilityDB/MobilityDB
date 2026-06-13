set(TZ_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/findtimezone.c
  ${CMAKE_CURRENT_SOURCE_DIR}/localtime.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strftime.c
  )

# pgtz.c defines `session_timezone` (and pg_tzset). On Windows the import lib
# from PG also exports them, causing multiply-defined errors; on macOS the
# dylib's local NULL definition would shadow the backend's pointer, crashing
# every timestamp parse. Skip pgtz.c for those platforms; localtime.c et al.
# are still needed because they hold helpers unique to pgtypes (e.g.
# pg_timezone_abbrev_is_known) that the backend does not export.
if(NOT PGTYPES_EXT_NO_BACKEND_DUPS)
  list(APPEND TZ_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pgtz.c)
endif()

set(PGTYPES_SOURCES ${PGTYPES_SOURCES} ${TZ_SOURCES}
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
