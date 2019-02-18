target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM}) # postgis)
add_definitions(-DWITH_POSTGIS)

include_directories("npoint/include")

file(GLOB SRCNPOINT "npoint/src/*.c")
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${SRCNPOINT})

file(GLOB SQLNPOINT "npoint/src/sql/*.in.sql")
list(SORT SQLNPOINT)
set(SQL "${SQL};${SQLNPOINT}")
set(CONTROLIN "${CONTROLIN};point/control.in;npoint/control.in")
