# No Postgres version dependant files

# 168_trgeo_aggfuncs has no build entry upstream and is not built; left unchanged here.
SET(LOCAL_FILES
  150_trgeo
  154_trgeo_compops
  156_trgeo_spatialfuncs
  158_trgeo_tile
  160_trgeo_boxops
  161_trgeo_topops
  162_trgeo_posops
  164_trgeo_distance
  166_trgeo_similarity
  170_trgeo_spatialrels
  172_trgeo_tempspatialrels
  173_trgeo_indexes
  175_trgeo_geom_clip
  176_trgeo_vclip
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
