cmake_minimum_required(VERSION 3.2)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
        message(FATAL_ERROR "In-source builds not allowed.
        Please make a new directory (called a build directory) and run CMake from there.
        You may need to remove 'CMakeCache.txt' and 'CMakeFiles/'.")
endif()

project(MobilityDB VERSION 1.0)
set(MOBILITYDB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
string(TOLOWER ${CMAKE_PROJECT_NAME} MOBILITYDB_LOWERCASE_NAME)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckSymbolExists)

#[[
set(SRCS
src/geo_constructors.c
src/doublen.c
src/lifting.c
src/period.c
src/periodset.c
src/rangetypes_ext.c
src/skiplist.c
src/tbool_boolops.c
src/tbox.c
src/tempcache.c
src/temporal.c
src/tinstant.c
src/tinstantset.c
src/tsequence.c
src/tsequenceset.c
src/temporal_aggfuncs.c
src/temporal_analyze.c
src/temporal_boxops.c
src/temporal_compops.c
src/temporal_gist.c
src/tnumber_mathfuncs.c
src/temporal_parser.c
src/temporal_posops.c
src/temporal_selfuncs.c
src/temporal_spgist.c
src/temporal_tile.c
src/temporal_util.c
src/temporal_waggfuncs.c
src/timeops.c
src/timestampset.c
src/time_aggfuncs.c
src/time_analyze.c
src/time_gist.c
src/time_selfuncs.c
src/time_spgist.c
src/tnumber_distance.c
src/tnumber_gist.c
src/tnumber_selfuncs.c
src/tnumber_spgist.c
src/ttext_textfuncs.c
)
]]
include(CTest)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckCCompilerFlag)
if(NOT WIN32)
    CHECK_C_COMPILER_FLAG("-fPIC" C_COMPILER_SUPPORTS_FPIC)
    if(C_COMPILER_SUPPORTS_FPIC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")
    endif()
endif()

#-------------------------------------
# Verify PostgreSQL Version
#-------------------------------------
set(PG_MIN_MAJOR_VERSION "10")
set(PG_MAX_MAJOR_VERSION "14")

find_package(POSTGRESQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
if (NOT POSTGRES_VERSION VERSION_LESS PG_MAX_MAJOR_VERSION)
  message(FATAL_ERROR "Not supporting PostgreSQL ${POSTGRESQL_VERSION}")
endif()

add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER} -DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")

include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})
if(WIN32)
    include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32)
    if(MSVC)
        include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32_msvc/)
    endif()
endif()

if(WIN32)
    link_directories(${POSTGRESQL_LIBRARIES})
    link_libraries(postgres)
endif()
#--------------------------------


add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
if (CMAKE_COMPILER_IS_GNUCC)
	if (WITH_COVERAGE)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
	endif ()
endif ()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")


set(CONTROLIN "control.in")
set(CONTROLOUT "${MOBILITYDB_LOWERCASE_NAME}.control")
set(SQLPP "${MOBILITYDB_LOWERCASE_NAME}--${MOBILITYDB_VERSION}.sql.c")
set(MOBILITYDB_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}--${MOBILITYDB_VERSION}.sql")


add_custom_target(sqlscript ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE})
add_custom_target(control ALL DEPENDS ${CMAKE_BINARY_DIR}/${CONTROLOUT})


if (APPLE)
	SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader /usr/local/bin/postgres")
endif ()

# Currently does not work with other versions of PostGIS
find_package(POSTGIS 2.5 EXACT REQUIRED)
add_definitions(-DPOSTGIS_VERSION_STR="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})


#--------------------------------
# Belongs to MobilityDB
# Must go before target_link_libraries

include_directories("include")
add_subdirectory("src")
set(PROJECT_OBJECTS "$<TARGET_OBJECTS:general>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:point>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
add_library(${CMAKE_PROJECT_NAME} MODULE ${PROJECT_OBJECTS})


#--------------------------------

find_package(JSON-C REQUIRED)
include_directories(${JSON-C_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${JSON-C_LIBRARIES})

find_package(PROJ REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJ_LIBRARIES})
include_directories(${PROJ_INCLUDE_DIRS})

find_package(LWGEOM REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${LWGEOM_LIBRARIES})
include_directories(${LWGEOM_INCLUDE_DIRS})

find_package(GSL REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} ${GSL_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} ${GSL_CLBAS_LIBRARY})

check_symbol_exists(ffsl "string.h" HAS_FFSL)
if(NOT HAS_FFSL)
	add_definitions(-D NO_FFSL)
endif()

#--------------------------------
# Belongs to MobilityDB

include("point/point.cmake")
include("npoint/npoint.cmake")


add_subdirectory(test)


add_subdirectory(sql)

add_custom_command(
	OUTPUT ${CONTROLOUT}
	COMMAND cat ${CONTROLIN} | sed -e's/LIBNAME/${CMAKE_PROJECT_NAME}/' -e's/VERSION/${MOBILITYDB_VERSION}/'  > ${CMAKE_BINARY_DIR}/${CONTROLOUT}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${CONTROLIN})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")
install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
install(FILES "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}" DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
