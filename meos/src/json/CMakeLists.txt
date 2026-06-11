set(JSON_SOURCES
  jsonbset.c
  jsonbset_jsonfuncs.c
  # jsonpath_exec.c
  tjsonb.c
  tjsonb_jsonfuncs.c
  # tjsonb_jsonpath.c
  tjsonb_compops.c
)

add_library(json OBJECT ${JSON_SOURCES})
