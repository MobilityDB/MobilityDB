if(MEOS)
  set(tpoint_meos.c tpoint_meos.c)
  set(tpoint_spatialrels_meos.c tpoint_spatialrels_meos.c)
  set(tpoint_spatialrels_meos.c tpoint_spatialrels_meos.c)
endif()

add_library(point OBJECT
  geography_functions.c
  pgis_call.c
  stbox.c
  tpoint.c
  tpoint_aggfuncs.c
  tpoint_analytics.c
  tpoint_boxops.c
  tpoint_distance.c
  tpoint_out.c
  tpoint_parser.c
  tpoint_spatialfuncs.c
  tpoint_spatialrels.c
  tpoint_tempspatialrels.c
  tpoint_tile.c
  ${tpoint_meos.c}
  ${tpoint_spatialrels_meos.c}
  ${tpoint_tempspatialrels_meos.c}
)
