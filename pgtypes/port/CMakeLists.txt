set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/chklocale.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_localeconv_r.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pgstrcasecmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort.c
  ${CMAKE_CURRENT_SOURCE_DIR}/qsort_arg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/snprintf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strerror.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strlcpy.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
