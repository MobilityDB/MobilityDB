#-------------------------------------
# MobilityDB CMake file
#-------------------------------------

message(STATUS "-------------------")
message(STATUS "Building MobilityDB")
message(STATUS "-------------------")

# Set the version and name of the MobilityDB library
set(MOBILITYDB_LIB_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(MOBILITYDB_LIB_NAME "${CMAKE_PROJECT_NAME}-${MOBILITYDB_LIB_VERSION}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set the name of the PostgreSQL extension
string(TOLOWER ${CMAKE_PROJECT_NAME} MOBILITYDB_LOWERCASE_NAME)
set(MOBILITYDB_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}--${PROJECT_VERSION}.sql")
set(MOBILITYDB_DATAGEN_EXTENSION_FILE "${MOBILITYDB_LOWERCASE_NAME}_datagen--${PROJECT_VERSION}.sql")

# Comment out code used for debugging purposes so it is not concerned by the coverage
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
  add_definitions(-DDEBUG_BUILD=1)
else()
  add_definitions(-DDEBUG_BUILD=0)
endif()

# Option to show debug messages for analyzing the selectivity estimation
option(DEBUG_SELECTIVITY
  "Set DEBUG_SELECTIVITY (default=OFF) to show debug messages for analyzing the
  selectivity estimation
  "
  OFF
)

if(DEBUG_SELECTIVITY)
  add_definitions(-DDEBUG_SELECTIVITY=1)
  message(STATUS "Showing debug messages for selectivity estimation")
endif()

#-------------------------------------
# Get PostgreSQL Version
#-------------------------------------

# Find PostgreSQL
set(PG_MIN_MAJOR_VERSION "14")
set(PG_MAX_MAJOR_VERSION "18")
find_package(POSTGRESQL ${PG_MIN_MAJOR_VERSION} REQUIRED)
if(NOT POSTGRES_VERSION VERSION_LESS PG_MAX_MAJOR_VERSION)
  message(FATAL_ERROR "Not supporting PostgreSQL ${POSTGRESQL_VERSION}")
endif()

include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR})
if(WIN32)
  include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32)
  if(MSVC)
    include_directories(SYSTEM ${POSTGRESQL_INCLUDE_DIR}/port/win32_msvc/)
  endif()
  link_directories(${POSTGRESQL_LIBRARIES})
  link_libraries(postgres)
endif()

# MobilityDB definitions for PostgreSQL version-dependent code
add_definitions(-DPOSTGRESQL_VERSION_STRING="${POSTGRESQL_VERSION_STRING}")
add_definitions(-DPOSTGRESQL_VERSION_NUMBER=${POSTGRESQL_VERSION_NUMBER})

#-------------------------------------
# Get PostGIS Version
#-------------------------------------

find_package(POSTGIS REQUIRED)

# POSTGIS_PGSQL_VERSION is used in PostGIS for PostgreSQL version-dependent code
# Note that we cannot add ${POSTGRESQL_VERSION_MINOR} since, e.g., for version
# 13.13 it will yield 143 which is not intended
math(EXPR POSTGIS_PGSQL_VERSION "${POSTGRESQL_VERSION_MAJOR} * 10")
message(STATUS "POSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION}")

add_definitions(-DPOSTGIS_VERSION_STRING="${POSTGIS_VERSION_STR}")
add_definitions(-DPOSTGIS_VERSION_NUMBER=${POSTGIS_VERSION_NUMBER})
add_definitions(-DPOSTGIS_PGSQL_VERSION=${POSTGIS_PGSQL_VERSION})

#--------------------------------
# MobilityDB directories
#--------------------------------

# PostgreSQL
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/pgtypes")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/pgtypes")

set(SRID_MAX "999999")
set(SRID_USR_MAX "998999")

# Embed PostGIS
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis/libpgcommon")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/postgis")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis/liblwgeom")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/postgis")
# add_subdirectory("${CMAKE_SOURCE_DIR}/postgis" "postgis")
add_subdirectory("${CMAKE_SOURCE_DIR}/postgis/libpgcommon" "postgis")

# MEOS
include_directories("${CMAKE_SOURCE_DIR}/meos/include")

# MobilityDB
include_directories("pg_include")
add_subdirectory("src")

# Must be after add_library
if(CMAKE_GENERATOR STREQUAL "Unix Makefiles" OR CMAKE_GENERATOR STREQUAL "Ninja")
  # Disable all warnings on embeded code from PostgreSQL and PostGIS
  # target_compile_options(liblwgeom PRIVATE "-w")
  target_compile_options(libpgcommon PRIVATE "-w")
  # target_compile_options(ryu PRIVATE "-w")
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  # Disable all warnings on embeded code from PostgreSQL and PostGIS
  # target_compile_options(liblwgeom PRIVATE "/W0")
  # target_compile_options(libpgcommon PRIVATE "/W0")
  # target_compile_options(ryu PRIVATE "/W0")
  # Enable all warnings on MEOS and MobilityDB
  # target_compile_options(temporal PRIVATE "/W4")
  # target_compile_options(geo PRIVATE "/W4")
  target_compile_options(pg_temporal PRIVATE "/W4")
  target_compile_options(pg_geo PRIVATE "/W4")
  if(CBUFFER)
    # target_compile_options(cbuffer PRIVATE "/W4")
    target_compile_options(pg_cbuffer PRIVATE "/W4")
  endif()
  if(NPOINT)
    # target_compile_options(npoint PRIVATE "/W4")
    target_compile_options(pg_npoint PRIVATE "/W4")
  endif()
  if(POSE)
    # target_compile_options(pose PRIVATE "/W4")
    target_compile_options(pg_pose PRIVATE "/W4")
  endif()
  if(RGEO)
    # target_compile_options(rgeo PRIVATE "/W4")
    target_compile_options(pg_rgeo PRIVATE "/W4")
  endif()
endif()

#--------------------------------
# Build MobilityDB library
#--------------------------------

# MEOS
# set(PROJECT_OBJECTS "$<TARGET_OBJECTS:temporal>")
# set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:geo>")
# if(CBUFFER)
  # message(STATUS "Including circular buffers")
  # set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:cbuffer>")
# endif()
# if(NPOINT)
  # message(STATUS "Including network points")
  # set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:npoint>")
# endif()
# if(POSE)
  # message(STATUS "Including geoposes")
  # set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pose>")
# endif()
# if(RGEO)
  # message(STATUS "Including rigid geometries")
  # set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:rgeo>")
# endif()

# MobilityDB
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_temporal>")
set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_geo>")
if(CBUFFER)
  message(STATUS "Including circular buffers")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_cbuffer>")
endif()
if(NPOINT)
  message(STATUS "Including network points")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_npoint>")
endif()
if(POSE)
  message(STATUS "Including geoposes")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_pose>")
endif()
if(RGEO)
  message(STATUS "Including rigid geometries")
  set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:pg_rgeo>")
endif()

set(PROJECT_OBJECTS ${PROJECT_OBJECTS} "$<TARGET_OBJECTS:libpgcommon>")

# Build the library
add_library(${MOBILITYDB_LIB_NAME} SHARED ${PROJECT_OBJECTS})
if(APPLE)
  set_target_properties(${MOBILITYDB_LIB_NAME} PROPERTIES
    SUFFIX ".dylib"
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup -bundle_loader ${POSTGRESQL_BIN_DIR}/postgres"
  )
endif()

#--------------------------------
# Specify libraries to link
#--------------------------------

# Link to MEOS static library (built with MEOS=0)
# This was built by the meos subdirectory before mobilitydb subdirectory
target_link_libraries(${MOBILITYDB_LIB_NAME} meos)

target_link_libraries(${MOBILITYDB_LIB_NAME} ${JSON-C_LIBRARIES})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GEOS_LIBRARY})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${PROJ_LIBRARIES})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_LIBRARY})
target_link_libraries(${MOBILITYDB_LIB_NAME} ${GSL_CBLAS_LIBRARY})
if(WIN32)
  message(STATUS "Adding PostGIS as a target link library")
  target_link_libraries(${MOBILITYDB_LIB_NAME} ${POSTGIS_LIBRARY})
endif()

# Link the application to pgtypes and postgis
target_link_libraries(${MOBILITYDB_LIB_NAME} pgtypes)
target_link_libraries(${MOBILITYDB_LIB_NAME} postgis)

#--------------------------------
# Belongs to MobilityDB
#--------------------------------

message(STATUS "Including SQL definitions and tests")
add_subdirectory(sql)
add_subdirectory(datagen)
add_subdirectory(test)

add_custom_target(mobilitydb_sql ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE})
add_custom_target(mobilitydb_datagen_sql ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE})
add_custom_target(mobilitydb_control ALL DEPENDS ${CMAKE_BINARY_DIR}/mobilitydb.control)

# add_custom_target(datagen_sql ALL DEPENDS ${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE})
# add_custom_target(datagen_control ALL DEPENDS ${CMAKE_BINARY_DIR}/datagen/mobilitydb_datagen.control)

install(
  FILES "${CMAKE_BINARY_DIR}/mobilitydb.control" "${CMAKE_BINARY_DIR}/${MOBILITYDB_EXTENSION_FILE}"
  DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")
install(TARGETS ${MOBILITYDB_LIB_NAME} DESTINATION "${POSTGRESQL_DYNLIB_DIR}")
install(
  FILES "${CMAKE_BINARY_DIR}/mobilitydb_datagen.control" "${CMAKE_BINARY_DIR}/${MOBILITYDB_DATAGEN_EXTENSION_FILE}"
  DESTINATION "${POSTGRESQL_SHARE_DIR}/extension")

#-----------------------------------------------------------------------------
# The End
#-----------------------------------------------------------------------------
