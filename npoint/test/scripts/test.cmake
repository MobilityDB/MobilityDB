add_test(
	NAME load_npoint_tables
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_passfail ${CMAKE_BINARY_DIR} load_npoint_tables "../npoint/test/scripts/load.sql.xz"
)

set_tests_properties(load_npoint_tables PROPERTIES FIXTURES_SETUP DBNPOINT)
set_tests_properties(load_npoint_tables PROPERTIES FIXTURES_REQUIRED DBGEO)
set_tests_properties(load_npoint_tables PROPERTIES DEPENDS create_extension)

file(GLOB npoint_testfiles "npoint/test/queries/*.sql")
list(SORT npoint_testfiles)

foreach(file ${npoint_testfiles})
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
		set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED DBNPOINT)
		set_tests_properties(${TESTNAME} PROPERTIES RESOURCE_LOCK DBLOCK)
	endif()
endforeach()
