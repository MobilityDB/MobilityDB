# if(NOT MEOS)
  # set(lwgeom_transform.c lwgeom_transform.c)
# endif()

add_library(libpq OBJECT
  pqformat.c
  )
