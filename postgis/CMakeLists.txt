include_directories("liblwgeom")
include_directories("libpgcommon")
include_directories("ryu")

add_subdirectory("liblwgeom")
# if(NOT MEOS)
  add_subdirectory("libpgcommon")
# endif()
add_subdirectory("ryu")
