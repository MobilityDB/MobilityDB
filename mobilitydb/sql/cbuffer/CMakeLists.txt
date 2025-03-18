SET(LOCAL_FILES
  120_cbuffer
  121_cbufferset
  122_tcbuffer
  # 123_tcbuffer_inout
  124_tcbuffer_compops
  125_tcbuffer_spatialfuncs
  128_tcbuffer_topops
  129_tcbuffer_posops
  # 130_tcbuffer_distance
  131_tcbuffer_aggfuncs
  132_tcbuffer_spatialrels
  134_tcbuffer_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
