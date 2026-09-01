SET(LOCAL_FILES
  550_posechain
  551_posechainset
  552_tposechain
  554_tposechain_compops
  555_tposechain_spatialfuncs
  561_tposechain_aggfuncs
  558_tposechain_topops
  559_tposechain_posops
  564_tposechain_tempspatialrels
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
