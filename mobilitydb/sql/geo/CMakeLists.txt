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

if(GEO)
  list(APPEND LOCAL_FILES 
    # The numbers should be changed, e.g., from 252 to 052 when GEO becomes mandatory
    252_tgeo
    253_tgeo_inout
    254_tgeo_compops
    256_tgeo_spatialfuncs
    # 258_tgeo_tile
    260_tgeo_boxops
    262_tgeo_posops
    264_tgeo_distance
    268_tgeo_aggfuncs
    270_tgeo_spatialrels
    272_tgeo_tempspatialrels
    273_tgeo_gist
    274_tgeo_spgist
    276_tgeo_analytics
    )
endif()

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
