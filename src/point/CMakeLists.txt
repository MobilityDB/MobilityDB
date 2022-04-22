# Files for building the MEOS library
if(MEOS)
  set(tpoint_meos.c tpoint_meos.c)
  set(tpoint_boxops_meos.c tpoint_boxops_meos.c)
  set(tpoint_posops_meos.c tpoint_posops_meos.c)
  set(tpoint_spatialrels_meos.c tpoint_spatialrels_meos.c)
  set(tpoint_tempspatialrels_meos.c tpoint_tempspatialrels_meos.c)
else()
  set(tpoint_aggfuncs.c tpoint_aggfuncs.c)
  set(tpoint_analytics.c tpoint_analytics.c)
  set(tpoint_analyze.c tpoint_analyze.c)
  set(tpoint_datagen.c tpoint_datagen.c)
  set(tpoint_gist.c tpoint_gist.c)
  set(tpoint_posops.c tpoint_posops.c)
  set(tpoint_selfuncs.c tpoint_selfuncs.c)
  set(tpoint_spgist.c tpoint_spgist.c)
endif()

add_library(point OBJECT
  geography_functions.c
  projection_gk.c
  stbox.c
  tpoint.c
  ${tpoint_aggfuncs.c}
  ${tpoint_analytics.c}
  ${tpoint_analyze.c}
  tpoint_boxops.c
  ${tpoint_datagen.c}
  tpoint_distance.c
  ${tpoint_gist.c}
  tpoint_in.c
  tpoint_out.c
  tpoint_parser.c
  ${tpoint_posops.c}
  ${tpoint_selfuncs.c}
  tpoint_spatialfuncs.c
  tpoint_spatialrels.c
  ${tpoint_spgist.c}
  tpoint_tempspatialrels.c
  tpoint_tile.c
  ${tpoint_meos.c}
  ${tpoint_boxops_meos.c}
  ${tpoint_posops_meos.c}
  ${tpoint_spatialrels_meos.c}
  ${tpoint_tempspatialrels_meos.c}
  )
