set(GEO_SRCS
  geoset.c
  pgis_types.c
  stbox.c
  stbox_index.c
  tgeo_aggfuncs.c
  tgeo_boxops.c
  tgeo_compops.c
  tgeo_distance.c
  tgeo_out.c
  tgeo_parser.c
  tgeo_restrict.c
  tgeo_spatialfuncs.c
  tgeo_spatialrels.c
  tgeo_tempspatialrels.c
  tgeo_tile.c
  tpoint_datagen.c
  tpoint_spatialfuncs.c
  tspatial.c
  tspatial_srid.c
)

if(MEOS)
  list(APPEND GEO_SRCS
  tspatial_posops_meos.c
  tspatial_rtree.c
  tspatial_topops_meos.c
)
endif()

add_library(geo OBJECT ${GEO_SRCS})


