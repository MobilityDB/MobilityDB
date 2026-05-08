# No Postgres version dependant files

SET(LOCAL_FILES
  300_npoint
  301_npointset
  302_tnpoint
  304_tnpoint_compops
  306_tnpoint_spatialfuncs
  308_tnpoint_topops
  309_tnpoint_posops
  310_tnpoint_routeops
  311_tnpoint_gin
  312_tnpoint_distance
  314_tnpoint_aggfuncs
  317_tnpoint_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
