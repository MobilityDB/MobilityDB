SET(LOCAL_FILES
  150_cbuffer
  151_cbufferset
  152_tcbuffer
  # 153_tcbuffer_inout
  154_tcbuffer_compops
  155_tcbuffer_spatialfuncs
  158_tcbuffer_topops
  159_tcbuffer_posops
  160_tcbuffer_distance
  161_tcbuffer_aggfuncs
  162_tcbuffer_spatialrels
  164_tcbuffer_tempspatialrels
  166_tcbuffer_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
