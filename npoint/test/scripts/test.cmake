add_test(
	NAME load_npoint_tables
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_passfail ${CMAKE_BINARY_DIR} load_npoint_tables "../npoint/test/scripts/load.sql.xz"
)

set_tests_properties(load_npoint_tables PROPERTIES FIXTURES_SETUP DB)
set_tests_properties(load_npoint_tables PROPERTIES DEPENDS create_extension)

file(GLOB geom_testfiles "npoint/test/queries/*.sql")
foreach(file ${geom_testfiles})
	get_filename_component(TESTNAME ${file} NAME_WE)
	add_test(
		NAME ${TESTNAME} 
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
		COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_compare ${CMAKE_BINARY_DIR} ${TESTNAME} ${file} 
	)
	set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED DB)
	set_tests_properties(${TESTNAME} PROPERTIES RESOURCE_LOCK DBLOCK)
endforeach()

