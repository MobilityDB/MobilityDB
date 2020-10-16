add_test(
	NAME load_geom_tables
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_passfail ${CMAKE_BINARY_DIR} load_geom_tables "../point/test/scripts/load.sql.xz"
)

set_tests_properties(load_geom_tables PROPERTIES FIXTURES_SETUP DBGEO)
set_tests_properties(load_geom_tables PROPERTIES FIXTURES_REQUIRED DB)
set_tests_properties(load_geom_tables PROPERTIES DEPENDS create_extension)

file(GLOB geom_testfiles "point/test/queries/*.sql")
list(SORT geom_testfiles)

foreach(file ${geom_testfiles})
	get_filename_component(TESTNAME ${file} NAME_WE)
	set(DOTEST TRUE)
	if(${TESTNAME} MATCHES "_pg([0-9]+)")
		if(${PG_MAJOR_VERSION} LESS ${CMAKE_MATCH_1})
			message("Disabling test ${TESTNAME}")
			set(DOTEST FALSE)
		endif()
	endif()
	if(DOTEST)
		add_test(
			NAME ${TESTNAME} 
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
			COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_compare ${CMAKE_BINARY_DIR} ${TESTNAME} ${file} 
		)
		set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED DBGEO)
		set_tests_properties(${TESTNAME} PROPERTIES RESOURCE_LOCK DBLOCK)
	endif()
endforeach()

