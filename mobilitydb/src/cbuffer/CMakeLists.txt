add_library(pg_cbuffer OBJECT
  cbuffer.c
  tcbuffer.c
  tcbuffer_inout.c
  # tcbuffer_aggfuncs.c
  tcbuffer_boxops.c
  tcbuffer_compops.c
  # tcbuffer_distance.c
  tcbuffer_spatialfuncs.c
  tcbuffer_spatialrels.c
  # tcbuffer_tempspatialrels.c
  )
