cmake_minimum_required(VERSION 3.1)

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
include(CTest)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#-------------------------------------
# Verify PostgreSQL Version
#-------------------------------------
set(PG_MIN_MAJOR_VERSION "10")
set(PG_MAX_MAJOR_VERSION "14")

find_package(PostgreSQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
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


include_directories("include")
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

add_library(${CMAKE_PROJECT_NAME} MODULE ${SRCS})

if (APPLE)
	SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader /usr/local/bin/postgres")
endif ()

find_package(JSON-C REQUIRED)
message(STATUS "JSONC_INCLUDE_DIRs=${JSONC_INCLUDE_DIRS}")
include_directories(${JSONC_INCLUDE_DIRS})

find_library(HAS_LWGEOM lwgeom)
if (HAS_LWGEOM)
  target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LWGEOM})

	MESSAGE(STATUS "Found LWGEOM at ${HAS_LWGEOM} - compiling with PostGIS support")
  if(NOT EXISTS "${POSTGRESQL_DYNLIB_DIR}/postgis-2.5.so" AND NOT EXISTS "${POSTGRESQL_DYNLIB_DIR}/postgis-2.5.dll")
		message(FATAL_ERROR "Could not find postgis-2.5; only PostGIS 2.5 is currently supported")
	endif()

  message(STATUS "LWGEOM_ROOT=${LWGEOM_ROOT}")
  message(STATUS "PROJ_INCLUDE_DIR=${PROJ_INCLUDE_DIR}")
  #[[
	if (LWGEOM_ROOT)
		include_directories(${LWGEOM_ROOT}/include)
		MESSAGE(STATUS "Using custom liblwgeom root ${LWGEOM_ROOT}")
	endif()
  ]]
	if (PROJ_INCLUDE_DIR)
		include_directories(${PROJ_INCLUDE_DIR})
	endif()
else()
	message(FATAL_ERROR "Could not find lwgeom;")
endif()

add_subdirectory(test)
include("point/point.cmake")
include("npoint/npoint.cmake")

find_library(HAS_GSL gsl)
find_library(HAS_GSLCBLAS gslcblas)
if (HAS_GSL AND HAS_GSLCBLAS)
	MESSAGE(STATUS "Found GSL at ${HAS_GSL}, Found GSLCBLAS at ${HAS_GSLCBLAS}")
	target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_GSL}) # GSL)
	target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_GSLCBLAS}) # GSLCBLAS)
	include_directories(${GSL_INCLUDE_DIR})
	include_directories(${GSLCBLAS_INCLUDE_DIR})
else()
	message(FATAL_ERROR "Could not find gsl or gslcblas; ")
endif ()

#[[
find_library(HAS_LIBPROTOBUF protobuf-c)
if (HAS_LIBPROTOBUF)
	MESSAGE(STATUS "Found LIBPROTOBUF at ${HAS_LIBPROTOBUF}")
	target_link_libraries(${CMAKE_PROJECT_NAME} ${HAS_LIBPROTOBUF}) # LIBPROTOBUF)
	include_directories(${LIBPROTOBUF_INCLUDE_DIR})
else()
	message(FATAL_ERROR "Could not find protobuf-c; ")
endif ()
]]

check_symbol_exists(ffsl "string.h" HAS_FFSL)
if(NOT HAS_FFSL)
	add_definitions(-D NO_FFSL)
endif()

add_subdirectory(sql)
add_custom_command(
	OUTPUT ${CONTROLOUT}
	COMMAND cat ${CONTROLIN} | sed -e's/LIBNAME/${CMAKE_PROJECT_NAME}/' -e's/VERSION/${MOBILITYDB_VERSION}/'  > ${CMAKE_BINARY_DIR}/${CONTROLOUT}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${CONTROLIN})

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")
install(FILES "${CMAKE_BINARY_DIR}/${CONTROLOUT}" DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
install(FILES "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}" DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
