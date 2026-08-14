set(JSON_SOURCES
  jsonbset.c
  jsonbset_jsonfuncs.c
  # jsonpath_exec.c
  tjsonb.c
  tjsonb_jsonfuncs.c
  # tjsonb_jsonpath.c
  tjsonb_compops.c
)

if(MEOS)
  list(APPEND JSON_SOURCES
  jsonbset_meos.c
)
endif()

add_library(json OBJECT ${JSON_SOURCES})
