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
)

set(SQLPOINT
point/src/sql/50_stbox.in.sql
point/src/sql/51_tpoint.in.sql
point/src/sql/52_tpoint_in.in.sql
point/src/sql/53_tpoint_out.in.sql
point/src/sql/54_tpoint_compops.in.sql
point/src/sql/55_geography_functions.in.sql
point/src/sql/56_tpoint_spatialfuncs.in.sql
point/src/sql/58_tpoint_boxops.in.sql
point/src/sql/60_tpoint_posops.in.sql
point/src/sql/62_tpoint_distance.in.sql
point/src/sql/64_tpoint_aggfuncs.in.sql
point/src/sql/66_tpoint_spatialrels.in.sql
point/src/sql/68_tpoint_tempspatialrels.in.sql
point/src/sql/70_tpoint_gist.in.sql
point/src/sql/72_tpoint_spgist.in.sql
point/src/sql/74_tpoint_datagen.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

include("point/test/scripts/test.cmake")
