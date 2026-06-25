SET(LOCAL_FILES
  300_quadbin
  301_quadbinset
  306_quadbin_ops
  302_tquadbin
  307_tquadbin_ops
  304_tquadbin_compops
  308_tquadbin_topops
  309_tquadbin_posops
  312_tquadbin_spatialrels
  322_tquadbin_gist
  323_tquadbin_spgist
  )

append_portable_aliases()

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
