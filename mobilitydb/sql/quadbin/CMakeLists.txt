SET(LOCAL_FILES
  350_quadbin
  351_quadbinset
  352_quadbin_ops
  353_tquadbin
  354_tquadbin_compops
  355_tquadbin_spatialfuncs
  357_tquadbin_boxops
  358_tquadbin_topops
  359_tquadbin_posops
  361_tquadbin_aggfuncs
  362_tquadbin_spatialrels
  364_tquadbin_tempspatialrels
  372_tquadbin_gist
  373_tquadbin_spgist
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/mobilitydb/sql)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/mobilitydb/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
