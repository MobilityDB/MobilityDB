set(CTEST_PARALLEL_LEVEL 1)

add_test(
	NAME build
	COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR}
)

add_test(
	NAME setup
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh setup ${CMAKE_BINARY_DIR}
)
add_test(
	NAME create_extension
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh create_ext ${CMAKE_BINARY_DIR} 
)
add_test(
	NAME load_tables
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh run_passfail ${CMAKE_BINARY_DIR} load_tables "scripts/load.sql.xz"
)
add_test(
	NAME teardown
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
	COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/test.sh teardown ${CMAKE_BINARY_DIR} 
)

set_tests_properties(setup PROPERTIES FIXTURES_SETUP DBSETUP)
set_tests_properties(setup PROPERTIES RESOURCE_LOCK DBLOCK)
set_tests_properties(create_extension PROPERTIES DEPENDS setup)
set_tests_properties(create_extension PROPERTIES FIXTURES_SETUP DBEXT)
set_tests_properties(create_extension PROPERTIES RESOURCE_LOCK DBLOCK)
set_tests_properties(create_extension PROPERTIES FIXTURES_REQUIRED DBSETUP)
set_tests_properties(load_tables PROPERTIES DEPENDS create_extension)
set_tests_properties(load_tables PROPERTIES FIXTURES_SETUP DB)
set_tests_properties(load_tables PROPERTIES RESOURCE_LOCK DBLOCK)
set_tests_properties(load_tables PROPERTIES FIXTURES_REQUIRED DBEXT)
set_tests_properties(teardown PROPERTIES FIXTURES_CLEANUP DB;DBEXT;DBSETUP)
set_tests_properties(teardown PROPERTIES RESOURCE_LOCK DBLOCK)

file(GLOB testfiles "test/queries/*.sql")
list(SORT testfiles)

foreach(file ${testfiles})
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
		set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED DB)
		set_tests_properties(${TESTNAME} PROPERTIES RESOURCE_LOCK DBLOCK)
	endif()
endforeach()

