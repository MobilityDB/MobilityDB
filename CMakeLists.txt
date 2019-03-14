cmake_minimum_required(VERSION 3.1) 
project(MobilityDB VERSION 0.0.1)

set(SRCS
src/AggregateFuncs.c
src/ArithmeticOps.c
src/BooleanOps.c
src/BoundBoxOps.c
src/ComparisonOps.c
src/DoubleN.c
src/GenericOps.c
src/indexesstat.c
src/IndexGistTemporal.c
src/IndexGistTime.c
src/IndexGistTnumber.c
src/IndexSpgistTemporal.c
src/IndexSpgistTime.c
src/IndexSpgistTnumber.c
src/OidCache.c
src/Parser.c
src/Period.c
src/PeriodSelfuncs.c
src/PeriodSet.c
src/Range.c
src/RelativePosOps.c
src/TemporalAnalyze.c
src/Temporal.c
src/TemporalI.c
src/TemporalInst.c
src/TemporalS.c
src/TemporalSelfuncs.c
src/TemporalSeq.c
src/TemporalTypes.c
src/TempSelFuncs.c
src/TimeOps.c
src/TimeSelfuncs.c
src/TimestampSet.c
src/TimeTypanalyze.c
src/WAggregateFuncs.c
)

set(SQL
src/sql/00_Catalog.in.sql
src/sql/01_Period.in.sql
src/sql/01_PeriodSet.in.sql
src/sql/01_Range.in.sql
src/sql/01_TimestampSet.in.sql
src/sql/01_TimeTypesOps.in.sql
src/sql/02_IndexGistTime.in.sql
src/sql/02_IndexSPGistTime.in.sql
src/sql/02_Temporal.in.sql
src/sql/06_DoubleN.in.sql
src/sql/06_TempSelFuncs.in.sql
src/sql/07_ArithmeticOps.in.sql
src/sql/08_BooleanOps.in.sql
src/sql/09_ComparisonOps.in.sql
src/sql/10_AggregateFuncs.in.sql
src/sql/11_RelativePosOps.in.sql
src/sql/11_WAggregateFuncs.in.sql
src/sql/12_BoundBoxOps.in.sql
src/sql/13_IndexGistTemporal.in.sql
src/sql/13_IndexSPGistTemporal.in.sql
src/sql/99_OpCache.in.sql
)

include(CTest)
enable_testing()

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

add_definitions(-Wall -Wextra -std=gnu1x -Wno-unused-parameter)
if(CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-Wno-misleading-indentation)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

string(TOLOWER ${CMAKE_PROJECT_NAME} LCNAME)

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


