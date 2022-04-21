add_library(point OBJECT
  geography_functions.c
  projection_gk.c
  stbox.c
  tpoint.c
  # tpoint_meos.c
  tpoint_aggfuncs.c
  tpoint_analytics.c
  tpoint_analyze.c
  tpoint_boxops.c
  # tpoint_boxops_meos.c
  tpoint_datagen.c
  tpoint_distance.c
  tpoint_gist.c
  tpoint_in.c
  tpoint_out.c
  tpoint_parser.c
  # tpoint_posops_meos.c
  tpoint_posops.c
  tpoint_selfuncs.c
  tpoint_spatialfuncs.c
  tpoint_spatialrels.c
  # tpoint_spatialrels_meos.c
  tpoint_spgist.c
  tpoint_tempspatialrels.c
  # tpoint_tempspatialrels_meos.c
  tpoint_tile.c
  )
