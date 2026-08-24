SET(LOCAL_FILES
  400_pcset
  410_tpcbox
  420_tpcpoint
  430_tpcpatch
  433_tpcpoint_compops
  434_tpcpatch_compops
  435_tpc_boxops
  436_tpc_topops
  437_tpc_posops
  438_tpc_distance
  439_tpc_gist
  440_tpc_spgist
  441_tpc_aggfuncs
  442_tpcpoint_spatialfuncs
  443_tpcpoint_spatialrels
  444_tpcpoint_tempspatialrels
  445_tpcpoint_analytics
  446_tpcpoint_tile
  447_tpcpoint_distance
  448_tpcpatch_tempspatialrels
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
