# No Postgres version dependant files

SET(LOCAL_FILES
  150_trgeo
  152_trgeo_compops
  153_trgeo_spatialfuncs
  154_trgeo_spatialrels
  155_trgeo_tempspatialrels
  156_trgeo_topops
  157_trgeo_posops
  158_trgeo_tile
  161_trgeo_distance
  161_trgeo_vclip
  162_trgeo_indexes
  163_trgeo_geom_clip
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
