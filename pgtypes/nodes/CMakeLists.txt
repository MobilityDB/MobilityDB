set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/copyfuncs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/equalfuncs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/nodeFuncs.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
