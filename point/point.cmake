target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("point/include")

set(SRCPOINT
point/src/stbox.c
point/src/tpoint_aggfuncs.c
point/src/tpoint_boxops.c
point/src/tpoint_datagen.c
point/src/tpoint_parser.c
point/src/tpoint_posops.c
point/src/tpoint_gist.c
point/src/tpoint_spgist.c
point/src/tpoint_tile.c
point/src/projection_gk.c
point/src/geography_functions.c
point/src/tpoint_spatialfuncs.c
point/src/tpoint_distance.c
point/src/tpoint_spatialrels.c
point/src/tpoint.c
point/src/tpoint_in.c
point/src/tpoint_out.c
point/src/tpoint_analyze.c
point/src/tpoint_selfuncs.c
point/src/tpoint_tempspatialrels.c
point/src/tpoint_analytics.c
)

set(SQLPOINT
sql/point/50_stbox.in.sql
sql/point/51_tpoint.in.sql
sql/point/52_tpoint_in.in.sql
sql/point/53_tpoint_out.in.sql
sql/point/54_tpoint_compops.in.sql
sql/point/55_geography_functions.in.sql
sql/point/56_tpoint_spatialfuncs.in.sql
sql/point/57_tpoint_tile.in.sql
sql/point/58_tpoint_boxops.in.sql
sql/point/60_tpoint_posops.in.sql
sql/point/62_tpoint_distance.in.sql
sql/point/64_tpoint_aggfuncs.in.sql
sql/point/66_tpoint_spatialrels.in.sql
sql/point/68_tpoint_tempspatialrels.in.sql
sql/point/70_tpoint_gist.in.sql
sql/point/72_tpoint_spgist.in.sql
sql/point/74_tpoint_datagen.in.sql
sql/point/76_tpoint_analytics.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

