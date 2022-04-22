if(MEOS)
  set(tnpoint_topoops_meos.c tnpoint_topoops_meos.c)
  set(tnpoint_posops_meos.c tnpoint_posops_meos.c)
  set(tnpoint_tempspatialrels_meos.c tnpoint_tempspatialrels_meos.c)
else()
  set(tnpoint_aggfuncs.c tnpoint_aggfuncs.c)
  set(tnpoint_analyze.c tnpoint_analyze.c)
  set(tnpoint_indexes.c tnpoint_indexes.c)
  set(tnpoint_selfuncs.c tnpoint_selfuncs.c)
  set(tnpoint_posops.c tnpoint_posops.c)
endif()

add_library(npoint OBJECT
  tnpoint.c
  ${tnpoint_aggfuncs.c}
  ${tnpoint_analyze.c}
  tnpoint_boxops.c
  tnpoint_distance.c
  ${tnpoint_indexes.c}
  tnpoint_parser.c
  ${tnpoint_posops.c}
  ${tnpoint_selfuncs.c}
  tnpoint_spatialfuncs.c
  tnpoint_spatialrels.c
  tnpoint_static.c
  tnpoint_tempspatialrels.c
  ${tnpoint_topoops_meos.c}
  ${tnpoint_posops_meos.c}
  ${tnpoint_tempspatialrels_meos.c}
  )
