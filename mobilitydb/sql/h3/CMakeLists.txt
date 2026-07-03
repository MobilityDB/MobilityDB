SET(LOCAL_FILES
  250_h3index
  251_h3indexset
  252_h3index_ops
  253_th3index
  254_th3index_compops
  255_th3index_spatialfuncs
  258_th3index_topops
  259_th3index_posops
  262_th3index_spatialrels
  272_th3index_gist
  273_th3index_spgist
  280_th3index_inspection
  281_th3index_hierarchy
  283_th3index_edges
  284_th3index_vertices
  285_th3index_traversal
  286_th3index_metrics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
