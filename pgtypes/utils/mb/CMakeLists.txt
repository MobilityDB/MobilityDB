
# Add subdirectory first
add_subdirectory("conversion_procs")

# Local files
set(MB_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/conv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/mbutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stringinfo_mb.c
  ${CMAKE_CURRENT_SOURCE_DIR}/wstrcmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/wstrncmp.c
)

# Aggregate with child contributions
set(ALL_SOURCES
  ${PGTYPES_SOURCES}  # empty at this level
  ${MB_SOURCES}
)

# Push once to parent
set(PGTYPES_SOURCES ${ALL_SOURCES} PARENT_SCOPE)