target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("npoint/include")

set(SRCNPOINT
npoint/src/BoundBoxOps.c
npoint/src/IndexGistTNPoint.c
npoint/src/Parser.c
npoint/src/SpatialRels.c
npoint/src/StaticObjects.c
npoint/src/TempDistance.c
npoint/src/TemporalGeo.c
npoint/src/TemporalNPoint.c
npoint/src/TempSpatialRels.c
)

set(SQLNPOINT
npoint/src/sql/41_StaticObjects.in.sql
npoint/src/sql/42_TemporalNpoint.in.sql
npoint/src/sql/43_ComparisonOps.in.sql
npoint/src/sql/44_TemporalGeo.in.sql
npoint/src/sql/45_RelativePosOps.in.sql
npoint/src/sql/46_TempDistance.in.sql
npoint/src/sql/47_GeoAggFuncs.in.sql
npoint/src/sql/48_BoundBoxOps.in.sql
npoint/src/sql/49_SpatialRels.in.sql
npoint/src/sql/50_TempSpatialRels.in.sql
npoint/src/sql/51_IndexGistTNPoint.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCNPOINT})

set(SQL "${SQL};${SQLNPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in;npoint/control.in")
