set(PGTYPES_SOURCES ${PGTYPES_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/findtimezone.c
  ${CMAKE_CURRENT_SOURCE_DIR}/localtime.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pgtz.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strftime.c
  # Mandatory so that the variable update is visible in the parent directory's scope
  PARENT_SCOPE
  )
