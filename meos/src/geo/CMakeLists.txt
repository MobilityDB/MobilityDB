set(GEO_SRCS
  pgis_types.c
  stbox.c
  stbox_index.c
  tgeo_boxops.c
  tgeo_compops.c
  tgeo_distance.c
  tgeo_out.c
  tgeo_parser.c
  tgeo_restrict.c
  tgeo_spatialfuncs.c
  tgeo_spatialrels.c
  tgeo_tempspatialrels.c
  tpoint_aggfuncs.c
  tpoint_datagen.c
  tpoint_tile.c
  tspatial.c
  tspatial_posops_meos.c
  tspatial_topops_meos.c
  tspatial_srid.c
)

if(MEOS)
  list(APPEND GEO_SRCS tspatial_rtree.c)
endif()

add_library(geo OBJECT ${GEO_SRCS})


