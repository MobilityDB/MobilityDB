add_library(point OBJECT
  geography_functions.c
  projection_gk.c
  stbox.c
  stbox_pg.c
  tpoint.c
  # tpoint_meos.c
  tpoint_pg.c
  tpoint_aggfuncs.c
  tpoint_analytics.c
  tpoint_analytics_pg.c
  tpoint_analyze.c
  tpoint_boxops.c
  # tpoint_boxops_meos.c
  tpoint_boxops_pg.c
  tpoint_datagen.c
  tpoint_distance.c
  tpoint_distance_pg.c
  tpoint_gist.c
  tpoint_in.c
  tpoint_in_pg.c
  tpoint_out.c
  tpoint_out_pg.c
  tpoint_parser.c
  # tpoint_posops_meos.c
  tpoint_posops_pg.c
  tpoint_selfuncs.c
  tpoint_spatialfuncs.c
  tpoint_spatialfuncs_pg.c
  tpoint_spatialrels.c
  # tpoint_spatialrels_meos.c
  tpoint_spatialrels_pg.c
  tpoint_spgist.c
  tpoint_tempspatialrels.c
  # tpoint_tempspatialrels_meos.c
  tpoint_tempspatialrels_pg.c
  tpoint_tile.c
  )
