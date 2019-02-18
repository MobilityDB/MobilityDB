target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("point/include")

set(SRCPOINT
point/src/BoundBoxOps.c
point/src/Gbox.c
point/src/GeoAggFuncs.c
point/src/GeoEstimate.c
point/src/IndexGistTPoint.c
point/src/IndexSpgistTPoint.c
point/src/Parser.c
point/src/ProjectionGK.c
point/src/RelativePosOpsM.c
point/src/SpatialRels.c
point/src/TempDistance.c
point/src/TemporalGeo.c
point/src/TemporalPoint.c
point/src/TempSpatialRels.c
)

set(SQLPOINT
point/src/sql/20_Gbox.in.sql
point/src/sql/21_TemporalPoint.in.sql
point/src/sql/22_ComparisonOps.in.sql
point/src/sql/23_TemporalGeo.in.sql
point/src/sql/24_RelativePosOpsG.in.sql
point/src/sql/24_RelativePosOpsM.in.sql
point/src/sql/25_TempDistance.in.sql
point/src/sql/26_GeoAggFuncs.in.sql
point/src/sql/27_BoundBoxOpsG.in.sql
point/src/sql/27_BoundBoxOpsM.in.sql
point/src/sql/28_SpatialRelsG.in.sql
point/src/sql/28_SpatialRelsM.in.sql
point/src/sql/29_TempSpatialRelsG.in.sql
point/src/sql/29_TempSpatialRelsM.in.sql
point/src/sql/30_IndexGistTPointM.in.sql
point/src/sql/30_IndexSpgistTPointM.in.sql
point/src/sql/31_IndexGistTPointG.in.sql
point/src/sql/31_IndexSpgistTPointG.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

include("point/test/scripts/test.cmake")
