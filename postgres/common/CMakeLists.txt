# if(NOT MEOS)
  # set(lwgeom_transform.c lwgeom_transform.c)
# endif()

add_library(common OBJECT
  d2s.c
  hashfn.c
  )
