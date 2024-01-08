add_library(point OBJECT
  geography_functions.c
  pgis_call.c
  stbox.c
  tpoint.c
  tpoint_aggfuncs.c
  tpoint_boxops.c
  tpoint_boxops_meos.c
  tpoint_compops.c
  tpoint_distance.c
  tpoint_out.c
  tpoint_parser.c
  tpoint_posops_meos.c
  tpoint_restrfuncs.c
  tpoint_spatialfuncs.c
  tpoint_spatialrels.c
  tpoint_tempspatialrels.c
  tpoint_tile.c
)

