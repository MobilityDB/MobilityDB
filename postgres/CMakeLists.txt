include_directories("common")
include_directories("lib")
include_directories("port")
include_directories("utils")

# Vendored PostgreSQL files (timezone, port, ...) need to see the
# meos_tls.h header so that pg_tz globals can be marked thread-local
# in lockstep with their MEOS callers.
include_directories("${CMAKE_SOURCE_DIR}/meos/include")

add_subdirectory("common")
add_subdirectory("port")
add_subdirectory("timezone")
add_subdirectory("utils")
