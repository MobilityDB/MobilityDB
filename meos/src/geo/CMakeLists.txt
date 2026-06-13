set(GEO_SOURCES
  geo_round.c
  postgis_funcs.c
  stbox.c
  stbox_index.c
  tgeo.c
  tgeo_aggfuncs.c
  tgeo_boxops.c
  tgeo_compops.c
  tgeo_distance.c
  tgeo_restrict.c
  tgeo_spatialfuncs.c
  tgeo_spatialrels.c
  tgeo_tile.c
  tpoint_datagen.c
  tpoint_spatialfuncs.c
  tspatial.c
  tspatial_parser.c
  tspatial_tempspatialrels.c
  tspatial_srid.c
)

if(MEOS)
  list(APPEND GEO_SOURCES
  geoset_meos.c
  tgeo_meos.c
  tspatial_transform_meos.c
  tspatial_posops_meos.c
  # tspatial_rtree.c
  tspatial_topops_meos.c
)
endif()

add_library(geo OBJECT ${GEO_SOURCES})


