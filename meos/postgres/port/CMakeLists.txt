add_library(port OBJECT
  pg_bitutils.c
  pgstrcasecmp.c
  qsort.c
  qsort_arg.c
  snprintf.c
  )

set_property(TARGET liblwgeom PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET liblwgeom PROPERTY POSITION_INDEPENDENT_CODE ON)
