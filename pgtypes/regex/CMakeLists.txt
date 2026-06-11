set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/regcomp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/regerror.c
  ${CMAKE_CURRENT_SOURCE_DIR}/regexec.c
  ${CMAKE_CURRENT_SOURCE_DIR}/regexport.c
  ${CMAKE_CURRENT_SOURCE_DIR}/regfree.c
  ${CMAKE_CURRENT_SOURCE_DIR}/regprefix.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
