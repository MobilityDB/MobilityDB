SET(LOCAL_FILES
  450_jsonbset
  451_jsonbset_jsonfuncs
  452_tjsonb
  454_tjsonb_jsonfuncs
  456_tjsonb_op
  458_tjsonb_compops
  460_tjsonb_topops
  462_tjsonb_posops
  464_tjsonb_aggfuncs
  466_tjsonb_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
