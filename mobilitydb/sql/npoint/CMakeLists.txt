# No Postgres version dependant files

SET(LOCAL_FILES
  200_npoint
  201_npointset
  202_tnpoint
  204_tnpoint_compops
  206_tnpoint_spatialfuncs
  208_tnpoint_topops
  209_tnpoint_posops
  210_tnpoint_routeops
  211_tnpoint_gin
  212_tnpoint_distance
  214_tnpoint_aggfuncs
  217_tnpoint_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
