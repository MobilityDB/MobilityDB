add_definitions(-DWITH_POSTGIS)

include_directories("npoint/include")

set(SRCNPOINT
npoint/src/tnpoint.c
npoint/src/tnpoint_aggfuncs.c
npoint/src/tnpoint_analyze.c
npoint/src/tnpoint_boxops.c
npoint/src/tnpoint_distance.c
npoint/src/tnpoint_indexes.c
npoint/src/tnpoint_parser.c
npoint/src/tnpoint_posops.c
npoint/src/tnpoint_selfuncs.c
npoint/src/tnpoint_spatialfuncs.c
npoint/src/tnpoint_spatialrels.c
npoint/src/tnpoint_static.c
npoint/src/tnpoint_tempspatialrels.c
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCNPOINT})
set(CONTROLIN "${CONTROLIN};npoint/control.in")
