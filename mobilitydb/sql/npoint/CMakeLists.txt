# No Postgres version dependant files

SET(LOCAL_FILES
  081_tnpoint_static
  082_npointset
  083_tnpoint
  085_tnpoint_compops
  087_tnpoint_spatialfuncs
  089_tnpoint_boxops
  090_tnpoint_posops
  091_tnpoint_routeops
  092_tnpoint_gin
  093_tnpoint_distance
  095_tnpoint_aggfuncs
  096_tnpoint_spatialrels
  097_tnpoint_tempspatialrels
  098_tnpoint_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
