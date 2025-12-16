set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/scansup.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
