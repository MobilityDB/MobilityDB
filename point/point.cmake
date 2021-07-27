add_definitions(-DWITH_POSTGIS)
#[[
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
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})
]]

set(CONTROLIN "${CONTROLIN};point/control.in")

