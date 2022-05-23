# if(NOT MEOS)
  # set(lwgeom_transform.c lwgeom_transform.c)
# endif()

add_library(port OBJECT
  pgstrcasecmp.c
  qsort.c
  qsort_arg.c
  snprintf.c
  )
