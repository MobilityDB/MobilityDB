set(RGEO_SOURCES
  trgeo.c
  trgeo_boxops.c
  trgeo_compops.c
  trgeo_distance.c
  trgeo_inst.c
  trgeo_parser.c
  trgeo_seq.c
  trgeo_seqset.c
  # trgeo_spatialrels.c
  trgeo_utils.c
  trgeo_vclip.c
  )

add_library(rgeo OBJECT ${RGEO_SOURCES})