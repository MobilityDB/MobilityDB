set(QUADBIN_SOURCES
  quadbin.c
  quadbin_geo.c
  quadbinset.c
  tquadbin_ops.c
  tquadbin.c
  tquadbin_boxops.c
  tquadbin_compops.c
)

add_library(quadbin OBJECT ${QUADBIN_SOURCES})
