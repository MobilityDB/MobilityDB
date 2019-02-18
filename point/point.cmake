target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("point/include")

file(GLOB SRCPOINT "point/src/*.c")
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCPOINT})

file(GLOB SQLPOINT "point/src/sql/*.in.sql")
list(SORT SQLPOINT)
set(SQL "${SQL};${SQLPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in")

include("point/test/scripts/test.cmake")
