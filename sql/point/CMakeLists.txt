# Postgres version dependant files

if (${PG_FULL_VERSION} GREATER_EQUAL 110000)
  set(FILE_72 72_tpoint_spgist)
endif()

SET(LOCAL_FILES
  50_stbox
  51_tpoint
  52_tpoint_in
  53_tpoint_out
  54_tpoint_compops
  55_geography_functions
  56_tpoint_spatialfuncs
  57_tpoint_tile
  58_tpoint_boxops
  60_tpoint_posops
  62_tpoint_distance
  64_tpoint_aggfuncs
  66_tpoint_spatialrels
  68_tpoint_tempspatialrels
  70_tpoint_gist
  ${FILE_72}
  74_tpoint_datagen
  76_tpoint_analytics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
