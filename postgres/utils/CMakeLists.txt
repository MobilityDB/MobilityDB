# if(NOT MEOS)
  # set(lwgeom_transform.c lwgeom_transform.c)
# endif()

add_library(utils OBJECT
  bool.c
  date.c
  datetime.c
  int.c
  numutils.c
  timestamp.c
  varlena.c
  )
