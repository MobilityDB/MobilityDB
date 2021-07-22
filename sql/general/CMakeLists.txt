# Postgres version dependant files
if (${PG_FULL_VERSION} GREATER_EQUAL 110000)
  set(FILE_15 15_time_spgist)
  set(FILE_44 44_temporal_spgist)
  if (${PG_FULL_VERSION} GREATER_EQUAL 120000)
    set(FILE_19 19_geo_constructors)
  endif()
endif()

SET(LOCAL_FILES
  00_catalog
  01_period
  03_timestampset
  05_periodset
  07_rangetypes_ext
  11_timeops
  13_time_gist
  ${FILE_15}
  17_time_aggfuncs
  ${FILE_19}
  20_doublen
  21_tbox
  22_temporal
  23_temporal_tile
  24_tnumber_mathfuncs
  26_tbool_boolops
  27_ttext_textfuncs
  28_temporal_compops
  32_temporal_boxops
  34_temporal_posops
  36_tnumber_distance
  38_temporal_aggfuncs
  40_temporal_waggfuncs
  42_temporal_gist
  ${FILE_44}
  99_tempcache
  )

foreach (f ${LOCAL_FILES})
  process_file(${f} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_BINARY_DIR}/${f}.sql.in ${CMAKE_BINARY_DIR}/sql/${f}.sql @ONLY)
  list(APPEND PACKAGE_SQL_FILES  ${CMAKE_BINARY_DIR}/sql/${f}.sql)
endforeach()

set(PROJECT_SQL_FILES ${PROJECT_SQL_FILES} ${PACKAGE_SQL_FILES} PARENT_SCOPE)
