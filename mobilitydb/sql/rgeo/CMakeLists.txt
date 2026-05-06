# No Postgres version dependant files

SET(LOCAL_FILES
  250_trgeo
  252_trgeo_compops
  253_trgeo_spatialfuncs
  254_trgeo_spatialrels
  255_trgeo_tempspatialrels
  256_trgeo_topops
  257_trgeo_posops
  259_trgeo_aggfuncs
  261_trgeo_distance
  262_trgeo_indexes
  263_trgeo_vclip
  264_trgeo_geom_clip
  265_trgeo_tile
  266_trgeo_bbox_split
  267_trgeo_analytics
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
