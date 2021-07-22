# No Postgres version dependant files

SET(LOCAL_FILES
  81_tnpoint_static
  83_tnpoint
  85_tnpoint_compops
  87_tnpoint_spatialfuncs
  89_tnpoint_boxops
  91_tnpoint_posops
  93_tnpoint_distance
  95_tnpoint_aggfuncs
  96_tnpoint_spatialrels
  97_tnpoint_tempspatialrels
  98_tnpoint_indexes
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
