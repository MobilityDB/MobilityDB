set(CBUFFER_SRCS
  cbuffer.c
  tcbuffer.c
  # tcbuffer_aggfuncs.c
  tcbuffer_boxops.c
  tcbuffer_compops.c
  tcbuffer_distance.c
  tcbuffer_spatialfuncs.c
  tcbuffer_spatialrels.c
  tcbuffer_tempspatialrels.c
)

if(MEOS)
  list(APPEND CBUFFER_SRCS
  cbufferset_meos.c
)
endif()

add_library(cbuffer OBJECT ${CBUFFER_SRCS})
