# if(NOT MEOS)
  # set(lwgeom_transform.c lwgeom_transform.c)
# endif()

add_library(utils OBJECT
  bool.c
  date.c
  int.c
  numutils.c
  timestamp.c
  varlena.c
  )
