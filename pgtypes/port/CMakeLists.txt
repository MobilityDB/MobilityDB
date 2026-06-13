set(PORT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/chklocale.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_bitutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_localeconv_r.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort_arg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strerror.c
  )

# Only compile the fallback strlcpy when the platform lacks a native one
# (HAVE_DECL_STRLCPY is detected in the parent CMakeLists.txt).
if(NOT HAS_STRLCPY)
  list(APPEND PORT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/strlcpy.c)
endif()

# pgstrcasecmp.c and snprintf.c duplicate symbols from libpostgres on Windows.
# Only compile them outside the Windows-extension build.
if(NOT PGTYPES_EXT_WIN32)
  list(APPEND PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/pgstrcasecmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/snprintf.c
  )
endif()

set(PGTYPES_SOURCES ${PGTYPES_SOURCES} ${PORT_SOURCES}
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
