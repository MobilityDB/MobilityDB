SET(LOCAL_FILES
  600_s2cell
  601_s2cellset
  602_s2cell_ops
  603_ts2cell
  604_ts2cell_compops
  605_ts2cell_spatialfuncs
  608_ts2cell_topops
  609_ts2cell_posops
  612_ts2cell_spatialrels
  614_ts2cell_tempspatialrels
  622_ts2cell_gist
  623_ts2cell_spgist
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
