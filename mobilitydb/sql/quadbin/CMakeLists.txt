SET(LOCAL_FILES
  350_quadbin
  351_quadbinset
  356_quadbin_ops
  352_tquadbin
  357_tquadbin_ops
  354_tquadbin_compops
  358_tquadbin_topops
  359_tquadbin_posops
  362_tquadbin_spatialrels
  372_tquadbin_gist
  373_tquadbin_spgist
  )

append_portable_aliases()

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
