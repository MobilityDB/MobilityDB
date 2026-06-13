set(COMMON_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/d2s.c
  ${CMAKE_CURRENT_SOURCE_DIR}/encnames.c
  ${CMAKE_CURRENT_SOURCE_DIR}/f2s.c
  ${CMAKE_CURRENT_SOURCE_DIR}/hashfn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/jsonapi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pgfnames.c
  ${CMAKE_CURRENT_SOURCE_DIR}/psprintf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/string.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_case.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_category.c
  ${CMAKE_CURRENT_SOURCE_DIR}/unicode_norm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/wchar.c
  )

# fe_memutils.c and stringinfo.c duplicate symbols from libpostgres on Windows.
# Only compile them outside the Windows-extension build.
if(NOT PGTYPES_EXT_WIN32)
  list(APPEND COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/fe_memutils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stringinfo.c
  )
endif()

set(PGTYPES_SOURCES ${PGTYPES_SOURCES} ${COMMON_SOURCES}
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
