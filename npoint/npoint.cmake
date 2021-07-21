target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
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

set(SQLNPOINT
sql/npoint/81_tnpoint_static.in.sql
sql/npoint/83_tnpoint.in.sql
sql/npoint/85_tnpoint_compops.in.sql
sql/npoint/87_tnpoint_spatialfuncs.in.sql
sql/npoint/89_tnpoint_boxops.in.sql
sql/npoint/91_tnpoint_posops.in.sql
sql/npoint/93_tnpoint_distance.in.sql
sql/npoint/95_tnpoint_aggfuncs.in.sql
sql/npoint/96_tnpoint_spatialrels.in.sql
sql/npoint/97_tnpoint_tempspatialrels.in.sql
sql/npoint/98_tnpoint_indexes.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCNPOINT})

set(SQL "${SQL};${SQLNPOINT}")
set(CONTROLIN "${CONTROLIN};npoint/control.in")
