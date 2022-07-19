SET(LOCAL_FILES
  001_span
  003_timestampset
  005_periodset
  007_span_ops
  009_time_ops
  011_span_indexes
  013_time_indexes
  015_span_aggfuncs
  017_time_aggfuncs
  019_geo_constructors
  021_tbox
  022_temporal
  023_temporal_inout
  025_temporal_tile
  026_tnumber_mathfuncs
  028_tbool_boolops
  029_ttext_textfuncs
  030_temporal_compops
  032_temporal_boxops
  034_temporal_posops
  036_tnumber_distance
  037_temporal_similarity
  038_temporal_aggfuncs
  040_temporal_waggfuncs
  042_temporal_gist
  044_temporal_spgist
  999_temporal_cache
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/MobilityDB/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
