set(NPOINT_SOURCES
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
  list(APPEND NPOINT_SOURCES
  npointset_meos.c
  ways_meos.c
  )
else()
  list(APPEND NPOINT_SOURCES
  ways.c
  )
endif()

add_library(npoint OBJECT ${NPOINT_SOURCES})
