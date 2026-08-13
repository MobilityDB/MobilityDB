SET(LOCAL_FILES
  100_pose
  101_poseset
  102_tpose
  104_tpose_compops
  105_tpose_spatialfuncs
  107_tpose_boxops
  108_tpose_topops
  109_tpose_posops
  110_tpose_distance
  111_tpose_aggfuncs
  112_tpose_spatialrels
  114_tpose_tempspatialrels
  116_tpose_indexes
  117_tpose_analytics
  119_tpose_tile
  120_geopose_frames
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
