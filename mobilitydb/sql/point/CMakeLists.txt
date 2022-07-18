# Postgres version dependant files

if (${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL 110000)
  set(072_tpoint_spgist 072_tpoint_spgist)
endif()

SET(LOCAL_FILES
  050_stbox
  051_tpoint
  052_tpoint_inout
  054_tpoint_compops
  055_geography_functions
  056_tpoint_spatialfuncs
  057_tpoint_tile
  058_tpoint_boxops
  060_tpoint_posops
  062_tpoint_distance
  063_tpoint_similarity
  064_tpoint_aggfuncs
  066_tpoint_spatialrels
  068_tpoint_tempspatialrels
  070_tpoint_gist
  ${072_tpoint_spgist}
  074_tpoint_datagen
  076_tpoint_analytics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
