SET(LOCAL_FILES
  200_cbuffer
  201_cbufferset
  202_tcbuffer
  # 203_tcbuffer_inout
  204_tcbuffer_compops
  205_tcbuffer_spatialfuncs
  208_tcbuffer_topops
  209_tcbuffer_posops
  210_tcbuffer_distance
  211_tcbuffer_aggfuncs
  212_tcbuffer_spatialrels
  214_tcbuffer_tempspatialrels
  216_tcbuffer_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
