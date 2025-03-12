SET(LOCAL_FILES
  050_geoset
  051_stbox
  052_tpoint
  053_tpoint_inout
  054_tpoint_compops
  056_tpoint_spatialfuncs
  058_tpoint_tile
  060_tpoint_boxops
  062_tpoint_posops
  064_tpoint_distance
  066_tpoint_similarity
  068_tpoint_aggfuncs
  070_tpoint_spatialrels
  072_tpoint_tempspatialrels
  073_tpoint_gist
  074_tpoint_spgist
  076_tpoint_analytics
  078_tpoint_datagen
  )

# Files added for temporal gemetries
# TODO These files should be added to the list above
list(APPEND LOCAL_FILES 
  052_tgeo
  053_tgeo_inout
  054_tgeo_compops
  056_tgeo_spatialfuncs
  058_tgeo_tile
  060_tgeo_boxops
  062_tgeo_posops
  064_tgeo_distance
  068_tgeo_aggfuncs
  070_tgeo_spatialrels
  072_tgeo_tempspatialrels
  073_tgeo_gist
  074_tgeo_spgist
  076_tgeo_analytics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
