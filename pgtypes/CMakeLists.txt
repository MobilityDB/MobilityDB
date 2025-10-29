
if(MEOS)
  # Enable code specific to MEOS, typically user-oriented wrapper functions
  # hiding internal implementation details from the API. Such functions are
  # not used in the MobilityDB PostgreSQL extension.
  add_definitions(-DMEOS=1)
endif()

# PostgreSQL Types
configure_file(pg_config.h.in pg_config.h)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/pgtypes")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/pgtypes")

# Set the location of the directory of the time zone database
add_definitions(-DSYSTEMTZDIR="/usr/share/zoneinfo")
message(STATUS "Directory of the time zone database: /usr/share/zoneinfo")

add_subdirectory(access)
add_subdirectory(catalog)
add_subdirectory(common)
add_subdirectory(parser)
add_subdirectory(port)
# add_subdirectory(storage)
add_subdirectory(timezone)
add_subdirectory(utils)

set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:access>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:catalog>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:common>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:conversion_procs>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:mb>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:parser>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:port>")
# set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:storage>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:timezone>")
set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:utils>")

# Define a static library for pgtypes
add_library(pgtypes STATIC ${PGTYPES_OBJECTS})

target_include_directories(pgtypes PUBLIC "${CMAKE_SOURCE_DIR}/pgtypes")
target_include_directories(pgtypes PUBLIC "${CMAKE_BINARY_DIR}/pgtypes")

