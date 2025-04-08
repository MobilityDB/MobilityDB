set(NPOINT_SRCS
  npoint.c
  tnpoint.c
  tnpoint_aggfuncs.c
  tnpoint_boxops.c
  tnpoint_compops.c
  tnpoint_distance.c
  tnpoint_routeops.c
  tnpoint_spatialfuncs.c
)

if(MEOS)
  list(APPEND NPOINT_SRCS
  npointset_meos.c
)
endif()

add_library(npoint OBJECT ${NPOINT_SRCS})
