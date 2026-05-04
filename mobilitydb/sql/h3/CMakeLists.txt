SET(LOCAL_FILES
  250_h3index
  251_h3indexset
  254_h3indexset_ops
  260_h3index_sets
  270_th3index
  271_th3index_ops
  272_th3index_gist
  273_th3index_spgist
  274_th3index_accessors
  280_th3index_inspection
  281_th3index_hierarchy
  282_th3index_latlng
  283_th3index_edges
  284_th3index_vertices
  285_th3index_traversal
  286_th3index_metrics
  287_th3index_casts
  288_th3index_compops
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
