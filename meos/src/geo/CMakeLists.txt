set(GEO_SRCS
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
  geo_poly_clip.c
  tpoint_datagen.c
  tpoint_spatialfuncs.c
  tspatial.c
  tspatial_parser.c
  tspatial_tempspatialrels.c
  tspatial_srid.c
)

if(MEOS)
  list(APPEND GEO_SRCS
  geoset_meos.c
  tgeo_meos.c
  tspatial_transform_meos.c
  tspatial_posops_meos.c
  # tspatial_rtree.c
  tspatial_topops_meos.c
)
endif()

add_library(geo OBJECT ${GEO_SRCS})

# Vendored Clipper2 + C++ polygon-Boolean adapter
add_subdirectory(${CMAKE_SOURCE_DIR}/clipper2
  ${CMAKE_BINARY_DIR}/clipper2)
add_library(clip_clipper2 OBJECT clip_clipper2.cpp)
target_include_directories(clip_clipper2 PRIVATE
  ${CMAKE_SOURCE_DIR}/meos/include
  ${CMAKE_SOURCE_DIR}/clipper2/include
  ${POSTGRESQL_INCLUDE_DIR}
  ${POSTGIS_LIBLWGEOM_DIR}
)
set_target_properties(clip_clipper2 PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  POSITION_INDEPENDENT_CODE ON
)


