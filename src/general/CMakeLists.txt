add_library(general OBJECT
  doublen.c
  doublen_pg.c
  geo_constructors.c
  lifting.c
  period.c
  period_pg.c
  periodset.c
  periodset_pg.c
  rangetypes_ext.c
  skiplist.c
  tbool_boolops.c
  tbool_boolops_pg.c
  tbool_boolops_meos.c
  tbox.c
  tbox_pg.c
  tempcache.c
  temporal.c
  # temporal_meos.c
  temporal_pg.c
  temporal_aggfuncs.c
  temporal_analyze.c
  temporal_boxops.c
  # temporal_boxops_meos.c
  temporal_boxops_pg.c
  temporal_compops.c
  # temporal_compops_meos.c
  temporal_compops_pg.c
  temporal_gist.c
  temporal_parser.c
  # temporal_posops_meos.c
  temporal_posops_pg.c
  temporal_selfuncs.c
  temporal_similarity.c
  temporal_similarity_pg.c
  temporal_spgist.c
  temporal_supportfn.c
  temporal_tile.c
  temporal_util.c
  temporal_waggfuncs.c
  time_aggfuncs.c
  time_analyze.c
  time_gist.c
  time_ops.c
  time_ops_pg.c
  time_selfuncs.c
  time_spgist.c
  timestampset.c
  timestampset_pg.c
  tinstant.c
  tinstantset.c
  tnumber_distance.c
  tnumber_distance_pg.c
  tnumber_gist.c
  tnumber_mathfuncs.c
  # tnumber_mathfuncs_meos.c
  tnumber_mathfuncs_pg.c
  tnumber_selfuncs.c
  tnumber_spgist.c
  tsequence.c
  tsequenceset.c
  ttext_textfuncs.c
  # ttext_textfuncs_meos.c
  ttext_textfuncs_pg.c
  )
