cmake_minimum_required(VERSION 3.1) 
project(MobilityDB VERSION 0.0.1)

include(CTest)
enable_testing()

#find_package(PostgreSQL 11 REQUIRED) # the cmake PG module is beyond broken
set(PG_REQUIRED_VERSION "PostgreSQL 11")

find_program(PGCONFIG pg_config)
if(NOT PGCONFIG)
	message(FATAL_ERROR "Could not find pg_config")
endif()
execute_process(COMMAND ${PGCONFIG} --version OUTPUT_VARIABLE PGVERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT "${PGVERSION}" MATCHES "${PG_REQUIRED_VERSION}")
	message(FATAL_ERROR "Wrong PostgreSQL version: found ${PGVERSION}, required ${PG_REQUIRED_VERSION}")
endif()

execute_process(COMMAND ${PGCONFIG} --includedir --includedir-server OUTPUT_VARIABLE PostgreSQL_ACTUAL_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(${PostgreSQL_ACTUAL_INCLUDE_DIR})
include_directories("include")

#link_directories(${PostgreSQL_LIBRARY_DIRS})
#link_directories(${PostgreSQL_EXTLIB_DIR})

add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
if(CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-Wno-misleading-indentation)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

string(TOLOWER ${CMAKE_PROJECT_NAME} LCNAME)

file(GLOB SRCS "src/*.c")
file(GLOB SQL "src/sql/*.in.sql")

set(CONTROLIN "control.in")
set(CONTROLOUT "${LCNAME}.control")
set(SQLOUT "${LCNAME}--${PROJECT_VERSION}.sql")

execute_process(COMMAND ${PGCONFIG} --sharedir OUTPUT_VARIABLE PostgreSQL_SHARE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE) # FIXME: I don't think this work on Windows
execute_process(COMMAND ${PGCONFIG} --pkglibdir OUTPUT_VARIABLE PostgreSQL_EXTLIB_DIR OUTPUT_STRIP_TRAILING_WHITESPACE) # FIXME: I don't think this work on Windows

add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${SQLOUT})
add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/${CONTROLOUT})

add_library(${CMAKE_PROJECT_NAME} MODULE ${SRCS})

if(APPLE)
	SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader /usr/local/bin/postgres")
endif()


find_library(HAS_LWGEOM lwgeom)

option(WITH_POSTGIS "Include PostGIS support" ON)
if(WITH_POSTGIS)
	if(HAS_LWGEOM)
		MESSAGE(STATUS "Found LWGEOM at ${HAS_LWGEOM} - compiling with PostGIS support")
	else()
		MESSAGE(STATUS "LWGEOM not found - compiling without PostGIS support")
		set(WITH_POSTGIS OFF)
	endif()
endif()

include("test/scripts/test.cmake")

if(WITH_POSTGIS)
		include("point/point.cmake")
	    include("npoint/npoint.cmake")
endif()

add_custom_command(
	OUTPUT ${SQLOUT}
	COMMAND mkdir -p ${CMAKE_BINARY_DIR}/sqlin
	COMMAND cp ${SQL} ${CMAKE_BINARY_DIR}/sqlin/
	COMMAND cd ${CMAKE_BINARY_DIR}/sqlin/ && find -type f | sort | xargs cat > ${CMAKE_BINARY_DIR}/${SQLOUT}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${SQL})

add_custom_command(
	OUTPUT ${CONTROLOUT}
	COMMAND cat ${CONTROLIN} | sed -e's/LIBNAME/${CMAKE_PROJECT_NAME}/'  > ${CMAKE_BINARY_DIR}/${CONTROLOUT}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${CONTROLIN})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${PostgreSQL_EXTLIB_DIR}")
install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")
install(FILES "${CMAKE_BINARY_DIR}/${SQLOUT}" DESTINATION "${PostgreSQL_SHARE_DIR}/extension")

add_custom_target(install_restart
                  $(MAKE) install
                  COMMAND systemctl restart postgresql
                  DEPENDS ${PROJECT_NAME}
                  COMMENT "Restarting PostgreSQL")


