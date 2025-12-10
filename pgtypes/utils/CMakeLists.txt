
# Add subdirectory first
add_subdirectory("mb")

# Local files
set(UTILS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/bool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/dynahash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/int.c
  ${CMAKE_CURRENT_SOURCE_DIR}/int8.c
  ${CMAKE_CURRENT_SOURCE_DIR}/numeric.c
  ${CMAKE_CURRENT_SOURCE_DIR}/date.c
  ${CMAKE_CURRENT_SOURCE_DIR}/datetime.c
  ${CMAKE_CURRENT_SOURCE_DIR}/float.c
  # ${CMAKE_CURRENT_SOURCE_DIR}/format_type.c
  ${CMAKE_CURRENT_SOURCE_DIR}/formatting.c
  # ${CMAKE_CURRENT_SOURCE_DIR}/levenshtein.c
  # ${CMAKE_CURRENT_SOURCE_DIR}/numeric.c
  ${CMAKE_CURRENT_SOURCE_DIR}/numutils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_locale.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_locale_builtin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_locale_libc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pg_locale_icu.c
  ${CMAKE_CURRENT_SOURCE_DIR}/timestamp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/varlena.c
  # )

# if(JSON)
  # set(UTILS_SOURCES ${UTILS_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/json.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonb_op.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonb_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/jsonfuncs.c
)
# endif(JSON)

# Aggregate with child contributions
set(ALL_SOURCES
  ${PGTYPES_SOURCES}  # contains mb/ and mb/conversion files
  ${UTILS_SOURCES}
)

# Push once to parent
set(PGTYPES_SOURCES ${ALL_SOURCES} PARENT_SCOPE)
