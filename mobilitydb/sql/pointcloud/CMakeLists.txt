SET(LOCAL_FILES
  400_pcset
  410_tpcbox
  411_tpcbox_gist
  412_tpcbox_spgist
  420_tpcpoint
  430_tpcpatch
  435_tpc_topops
  436_tpc_posops
  437_tpc_gist
  438_tpc_spgist
  439_tpc_distance
  440_tpc_aggfuncs
  441_tpcpoint_spatialfuncs
  442_tpcpoint_spatialrels
  443_tpcpoint_tempspatialrels
  444_tpcpoint_analytics
  445_tpcpoint_tile
  446_tpcpoint_distance
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
