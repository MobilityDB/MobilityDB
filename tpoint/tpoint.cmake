target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("tpoint/include")

set(SRCPOINT
tpoint/src/stbox.c
tpoint/src/tpoint_aggfuncs.c
tpoint/src/tpoint_boxops.c
tpoint/src/tpoint_parser.c
tpoint/src/tpoint_posops.c
tpoint/src/tpoint_gist.c
tpoint/src/tpoint_spgist.c
tpoint/src/projection_gk.c
tpoint/src/tpoint_spatialfuncs.c
tpoint/src/tpoint_spatialrels.c
tpoint/src/tpoint_distance.c
tpoint/src/tpoint.c
tpoint/src/tpoint_in.c
tpoint/src/tpoint_out.c
tpoint/src/tpoint_analyze.c
tpoint/src/tpoint_selfuncs.c
tpoint/src/tpoint_tempspatialrels.c
)

set(SQLPOINT
tpoint/src/sql/50_stbox.in.sql
tpoint/src/sql/51_tpoint.in.sql
tpoint/src/sql/52_tpoint_in.in.sql
tpoint/src/sql/53_tpoint_out.in.sql
tpoint/src/sql/54_tpoint_compops.in.sql
tpoint/src/sql/56_tpoint_spatialfuncs.in.sql
tpoint/src/sql/58_tpoint_boxops.in.sql
tpoint/src/sql/60_tpoint_posops.in.sql
tpoint/src/sql/62_tpoint_distance.in.sql
tpoint/src/sql/64_tpoint_aggfuncs.in.sql
tpoint/src/sql/66_tpoint_spatialrels.in.sql
tpoint/src/sql/68_tpoint_tempspatialrels.in.sql
tpoint/src/sql/70_tpoint_gist.in.sql
tpoint/src/sql/72_tpoint_spgist.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};tpoint/control.in")

include("tpoint/test/scripts/test.cmake")
