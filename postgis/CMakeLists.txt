# Vendored PostGIS source needs to see meos_tls.h so its WKT
# lexer/parser globals can be tagged MEOS_TLS for thread-safety.
include_directories("${CMAKE_SOURCE_DIR}/meos/include")

add_subdirectory("liblwgeom")
if(NOT MEOS)
  add_subdirectory("libpgcommon")
endif()
add_subdirectory("ryu")
