add_library(npoint OBJECT
  tnpoint.c
  tnpoint_pg.c
  tnpoint_aggfuncs_pg.c
  tnpoint_analyze_pg.c
  tnpoint_boxops.c
  tnpoint_boxops_pg.c
  tnpoint_distance.c
  tnpoint_distance_pg.c
  tnpoint_indexes_pg.c
  tnpoint_parser.c
  tnpoint_posops_pg.c
  tnpoint_selfuncs_pg.c
  tnpoint_spatialfuncs.c
  tnpoint_spatialfuncs_pg.c
  tnpoint_spatialrels.c
  tnpoint_spatialrels_pg.c
  tnpoint_static.c
  tnpoint_static_pg.c
  tnpoint_tempspatialrels.c
  tnpoint_tempspatialrels_pg.c
  # tnpoint_topoops_meos.c
  # tnpoint_posops_meos.c
  # tnpoint_tempspatialrels_meos.c
  )
