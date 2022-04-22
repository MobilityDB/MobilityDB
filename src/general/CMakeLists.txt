if(MEOS)
  set(tbool_boolops_meos.c tbool_boolops_meos.c)
  set(temporal_meos.c temporal_meos.c)
  set(temporal_boxops_meos.c temporal_boxops_meos.c)
  set(temporal_compops_meos.c temporal_compops_meos.c)
  set(temporal_posops_meos.c temporal_posops_meos.c)
  set(tnumber_mathfuncs_meos.c tnumber_mathfuncs_meos.c)
  set(ttext_textfuncs_meos.c ttext_textfuncs_meos.c)
else()
  set(geo_constructors.c geo_constructors.c)
  set(temporal_aggfuncs.c temporal_aggfuncs.c)
  set(temporal_analyze.c temporal_analyze.c)
  set(temporal_gist.c temporal_gist.c)
  set(temporal_posops.c temporal_posops.c)
  set(temporal_selfuncs.c temporal_selfuncs.c)
  set(temporal_spgist.c temporal_spgist.c)
  set(temporal_supportfn.c temporal_supportfn.c)
  set(temporal_waggfuncs.c temporal_waggfuncs.c)
  set(time_aggfuncs.c time_aggfuncs.c)
  set(time_analyze.c time_analyze.c)
  set(time_gist.c time_gist.c)
  set(time_selfuncs.c time_selfuncs.c)
  set(time_spgist.c time_spgist.c)
  set(tnumber_gist.c tnumber_gist.c)
  set(tnumber_selfuncs.c tnumber_selfuncs.c)
  set(tnumber_spgist.c tnumber_spgist.c)
endif()

add_library(general OBJECT
  doublen.c
  ${geo_constructors.c}
  lifting.c
  period.c
  periodset.c
  rangetypes_ext.c
  skiplist.c
  tbool_boolops.c
  tbox.c
  tempcache.c
  temporal.c
  ${temporal_aggfuncs.c}
  ${temporal_analyze.c}
  temporal_boxops.c
  temporal_compops.c
  ${temporal_gist.c}
  temporal_parser.c
  ${temporal_posops.c}
  ${temporal_selfuncs.c}
  temporal_similarity.c
  ${temporal_spgist.c}
  ${temporal_supportfn.c}
  temporal_tile.c
  temporal_util.c
  ${temporal_waggfuncs.c}
  ${time_aggfuncs.c}
  ${time_analyze.c}
  ${time_gist.c}
  time_ops.c
  ${time_selfuncs.c}
  ${time_spgist.c}
  timestampset.c
  tinstant.c
  tinstantset.c
  tnumber_distance.c
  ${tnumber_gist.c}
  tnumber_mathfuncs.c
  ${tnumber_selfuncs.c}
  ${tnumber_spgist.c}
  tsequence.c
  tsequenceset.c
  ttext_textfuncs.c
  ${tbool_boolops_meos.c}
  ${temporal_meos.c}
  ${temporal_boxops_meos.c}
  ${temporal_compops_meos.c}
  ${temporal_posops_meos.c}
  ${tnumber_mathfuncs_meos.c}
  ${ttext_textfuncs_meos.c}
  )
