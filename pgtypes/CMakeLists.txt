# PostgreSQL Types
configure_file(pg_config.h.in pg_config.h)
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/pgtypes")
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/pgtypes")

# Set the location of the directory of the time zone database
add_definitions(-DSYSTEMTZDIR="/usr/share/zoneinfo")
message(STATUS "Directory of the time zone database: /usr/share/zoneinfo")

# Create an empty list that contains the source files of the pgtypes library.
# The subdirectories below will append its source files to the list.
set(PGTYPES_SOURCES "")

add_subdirectory(access)
add_subdirectory(catalog)
add_subdirectory(common)
add_subdirectory(parser)
add_subdirectory(port)
# add_subdirectory(storage)
add_subdirectory(timezone)
add_subdirectory(utils)

# Uncomment the following line to see all source files included
# message(STATUS "PGTYPES_SOURCES ${PGTYPES_SOURCES}")

# Compile the source files
add_library(pgtypes STATIC ${PGTYPES_SOURCES})

# Create the list of object files
# set(PGTYPES_OBJECTS "$<TARGET_OBJECTS:pgtypes>")

# Define a static library for pgtypes
# add_library(pgtypes STATIC ${PGTYPES_OBJECTS})

target_include_directories(pgtypes PUBLIC "${CMAKE_SOURCE_DIR}/pgtypes")
target_include_directories(pgtypes PUBLIC "${CMAKE_BINARY_DIR}/pgtypes")

