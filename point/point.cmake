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
point/src/sql/50_Gbox.in.sql
point/src/sql/52_TemporalPoint.in.sql
point/src/sql/54_ComparisonOps.in.sql
point/src/sql/56_TemporalGeo.in.sql
point/src/sql/58_BoundBoxOps.in.sql
point/src/sql/60_RelativePosOps.in.sql
point/src/sql/62_TempDistance.in.sql
point/src/sql/64_GeoAggFuncs.in.sql
point/src/sql/66_SpatialRels.in.sql
point/src/sql/68_TempSpatialRels.in.sql
point/src/sql/70_IndexGistTPoint.in.sql
point/src/sql/72_IndexSpgistTPoint.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

include("point/test/scripts/test.cmake")
