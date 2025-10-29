
set(UTILS_SRCS
  bool.c
  dynahash.c
  int.c
  int8.c
  numeric.c
  date.c
  datetime.c
  float.c
  # format_type.c
  formatting.c
  # numeric.c
  numutils.c
  pg_error.c
  pg_locale.c
  pg_locale_builtin.c
  pg_locale_libc.c
  pg_locale_icu.c
  timestamp.c
  varlena.c
  )

if(JSON)
  list(APPEND UTILS_SRCS
    json.c
    jsonb.c
    jsonb_op.c
    jsonb_util.c
    jsonfuncs.c
)
endif(JSON)

add_subdirectory("mb")

add_library(utils OBJECT ${UTILS_SRCS})
