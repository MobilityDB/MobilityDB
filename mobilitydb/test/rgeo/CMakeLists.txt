add_test(
  NAME load_rgeo_tables
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_passfail
    -D TEST_NAME=load_rgeo_tables -D TEST_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/load_rgeo.sql.xz
    -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
)

set_tests_properties(load_rgeo_tables PROPERTIES
  DEPENDS test_setup
  FIXTURES_SETUP DBRGEO
  FIXTURES_REQUIRED "DB;DBGEO;DBPOINT")

file(GLOB testfiles "queries/*.sql")
list(SORT testfiles)

# 122_trgeo, 124_trgeo_compops, 124_trgeo_compops_tbl were previously
# skipped because their sequence/sequenceset trgeometry-vs-geometry
# comparison queries spuriously raised "Operation on mixed SRID" even
# when the SRIDs were aligned. Root cause was in the lifting
# infrastructure: tsegment_intersection_value -> datumsegm_locate
# dispatched on the temporal value's basetype (T_POSE for trgeometry)
# and reinterpreted the right-hand GSERIALIZED bytes through
# posesegm_locate, tripping ensure_valid_pose_pose's SRID check on
# garbage data. Fixed in this change — see meos/src/temporal/lifting.c
# (cross_type guard) and meos/src/rgeo/trgeo_compops.c
# (eacomp_trgeo_geo / tcomp_geo_trgeo / tcomp_trgeo_geo set
# lfinfo.cross_type=true). Tests reactivated with regenerated expected.
set(SKIP_TESTS)

foreach(file ${testfiles})
  get_filename_component(TESTNAME ${file} NAME_WE)
  set(DOTEST TRUE)
  if(${TESTNAME} MATCHES "_pg([0-9]+)")
    if(${POSTGRESQL_VERSION_MAJOR} LESS ${CMAKE_MATCH_1})
      message("Disabling test ${TESTNAME}")
      set(DOTEST FALSE)
    endif()
  endif()
  # Skip tests whose expected output file is missing — the run_compare
  # script otherwise reports the entire actual output as a diff.
  if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/expected/${TESTNAME}.test.out")
    message("Disabling test ${TESTNAME}: missing expected/${TESTNAME}.test.out")
    set(DOTEST FALSE)
  endif()
  if(DOTEST)
    add_test(
      NAME ${TESTNAME}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND ${CMAKE_COMMAND} -D TEST_OPER=run_compare
        -D TEST_NAME=${TESTNAME} -D TEST_FILE=${file}
        -P ${CMAKE_BINARY_DIR}/mobilitydb/test/scripts/test.cmake
      )
    set_tests_properties(${TESTNAME} PROPERTIES
      FIXTURES_REQUIRED "DB;DBGEO;DBPOINT;DBRGEO"
      RESOURCE_LOCK DBLOCK)
  endif()
endforeach()
