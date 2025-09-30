add_library(port OBJECT
  pgstrcasecmp.c
  qsort.c
  qsort_arg.c
  snprintf.c
  )

set_property(TARGET port PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET port PROPERTY POSITION_INDEPENDENT_CODE ON)
