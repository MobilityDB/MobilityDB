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
  set(skiplist.c skiplist.c)
  set(span_aggfuncs.c span_aggfuncs.c)
  set(span_analyze.c span_analyze.c)
  set(span_gist.c span_gist.c)
  set(span_selfuncs.c span_selfuncs.c)
  set(span_spgist.c span_spgist.c)
  set(temporal_aggfuncs.c temporal_aggfuncs.c)
  set(temporal_analyze.c temporal_analyze.c)
  set(temporal_gist.c temporal_gist.c)
  set(temporal_posops.c temporal_posops.c)
  set(temporal_selfuncs.c temporal_selfuncs.c)
  set(temporal_spgist.c temporal_spgist.c)
  set(temporal_supportfn.c temporal_supportfn.c)
  set(temporal_tile.c temporal_tile.c)
  set(temporal_waggfuncs.c temporal_waggfuncs.c)
  set(time_aggfuncs.c time_aggfuncs.c)
  set(tnumber_gist.c tnumber_gist.c)
  set(tnumber_selfuncs.c tnumber_selfuncs.c)
  set(tnumber_spgist.c tnumber_spgist.c)
endif()

add_library(general OBJECT
  basetype_inout.c
  doublen.c
  ${geo_constructors.c}
  lifting.c
  periodset.c
  pg_call.c
  ${skiplist.c}
  span.c
  ${span_aggfuncs.c}
  ${span_analyze.c}
  ${span_gist.c}
  ${span_spgist.c}
  span_ops.c
  ${span_selfuncs.c}
  tbool_boolops.c
  tbox.c
  temporal.c
  ${temporal_aggfuncs.c}
  ${temporal_analyze.c}
  temporal_boxops.c
  temporal_catalog.c
  temporal_compops.c
  ${temporal_gist.c}
  temporal_parser.c
  ${temporal_posops.c}
  ${temporal_selfuncs.c}
  temporal_similarity.c
  ${temporal_spgist.c}
  ${temporal_supportfn.c}
  ${temporal_tile.c}
  temporal_util.c
  ${temporal_waggfuncs.c}
  temporal_in.c
  temporal_out.c
  ${time_aggfuncs.c}
  time_ops.c
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
