target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("point/include")

set(SRCPOINT
point/src/STbox.c
point/src/GeoAggFuncs.c
point/src/GeoBoundBoxOps.c
point/src/GeoParser.c
point/src/GeoRelativePosOps.c
point/src/IndexGistTempPoint.c
point/src/IndexSpgistTempPoint.c
point/src/mfjson.c
point/src/ProjectionGK.c
point/src/SpatialFuncs.c
point/src/SpatialRels.c
point/src/TempDistance.c
point/src/TempPoint.c
point/src/TempPointAnalyze.c
point/src/TempPointSelFuncs.c
point/src/TempSpatialRels.c
)

set(SQLPOINT
point/src/sql/50_STbox.in.sql
point/src/sql/52_TempPoint.in.sql
point/src/sql/54_ComparisonOps.in.sql
point/src/sql/56_SpatialFuncs.in.sql
point/src/sql/58_GeoBoundBoxOps.in.sql
point/src/sql/60_GeoRelativePosOps.in.sql
point/src/sql/62_TempDistance.in.sql
point/src/sql/64_GeoAggFuncs.in.sql
point/src/sql/66_SpatialRels.in.sql
point/src/sql/68_TempSpatialRels.in.sql
point/src/sql/70_IndexGistTempPoint.in.sql
point/src/sql/72_IndexSpgistTempPoint.in.sql
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

include("point/test/scripts/test.cmake")
