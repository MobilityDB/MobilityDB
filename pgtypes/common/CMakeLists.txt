set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/d2s.c
  ${CMAKE_CURRENT_SOURCE_DIR}/encnames.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fe_memutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/f2s.c
  ${CMAKE_CURRENT_SOURCE_DIR}/hashfn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/jsonapi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pgfnames.c
  ${CMAKE_CURRENT_SOURCE_DIR}/psprintf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/string.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stringinfo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_case.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_category.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_norm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/wchar.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
