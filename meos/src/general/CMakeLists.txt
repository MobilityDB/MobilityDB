if(MEOS)
  set(set_aggfuncs_meos.c set_aggfuncs_meos.c)
  set(tbool_boolops_meos.c tbool_boolops_meos.c)
  set(temporal_compops_meos.c temporal_compops_meos.c)
  set(temporal_meos.c temporal_meos.c)
  set(tnumber_mathfuncs_meos.c tnumber_mathfuncs_meos.c)
  set(ttext_textfuncs_meos.c ttext_textfuncs_meos.c)
endif()

add_library(general OBJECT
  basetype_inout.c
  doublen.c
  lifting.c
  meos_catalog.c
  pg_types.c
  skiplist.c
  set.c
  set_ops.c
  span.c
  span_aggfuncs.c
  span_ops.c
  spanset.c
  spanset_ops.c
  tbool_boolops.c
  tbox.c
  temporal.c
  temporal_aggfuncs.c
  temporal_boxops.c
  temporal_compops.c
  temporal_similarity.c
  temporal_tile.c
  time_aggfuncs.c
  tinstant.c
  tnumber_distance.c
  tnumber_mathfuncs.c
  tsequence.c
  tsequenceset.c
  ttext_textfuncs.c
  type_in.c
  type_out.c
  type_parser.c
  type_util.c
  ${set_aggfuncs_meos.c}
  ${tbool_boolops_meos.c}
  ${temporal_compops_meos.c}
  ${temporal_meos.c}
  ${tnumber_mathfuncs_meos.c}
  ${ttext_textfuncs_meos.c}
)

