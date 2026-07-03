SET(LOCAL_FILES
  200_jsonbset
  201_jsonbset_jsonfuncs
  202_tjsonb
  204_tjsonb_jsonfuncs
  206_tjsonb_op
  208_tjsonb_compops
  210_tjsonb_topops
  212_tjsonb_posops
  214_tjsonb_aggfuncs
  216_tjsonb_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
