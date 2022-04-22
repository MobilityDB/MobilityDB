# Postgres version dependant files
if (${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL 110000)
  set(FILE_015 015_time_spgist)
  set(FILE_044 044_temporal_spgist)
  if (${POSTGRESQL_VERSION_NUMBER} GREATER_EQUAL 120000)
    set(FILE_019 019_geo_constructors)
  endif()
endif()

SET(LOCAL_FILES
  000_catalog
  001_period
  003_timestampset
  005_periodset
  007_rangetypes_ext
  011_time_ops
  013_time_gist
  ${FILE_015}
  017_time_aggfuncs
  ${FILE_019}
  020_doublen
  021_tbox
  022_temporal
  023_temporal_tile
  024_tnumber_mathfuncs
  026_tbool_boolops
  027_ttext_textfuncs
  028_temporal_compops
  032_temporal_boxops
  034_temporal_posops
  036_tnumber_distance
  037_temporal_similarity
  038_temporal_aggfuncs
  040_temporal_waggfuncs
  042_temporal_gist
  ${FILE_044}
  999_tempcache
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
