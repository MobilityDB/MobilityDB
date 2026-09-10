add_library(librtcore OBJECT
  rt_band.c
  rt_context.c
  rt_gdal.c
  rt_geometry.c
  rt_mapalgebra.c
  rt_pixel.c
  rt_raster.c
  rt_serialize.c
  rt_spatial_relationship.c
  rt_statistics.c
  rt_util.c
  rt_warp.c
  rt_wkb.c
  )

target_include_directories(librtcore PRIVATE ${GDAL_INCLUDE_DIR})

# Hide the vendored raster core symbols from the shared library's dynamic table,
# as postgis/liblwgeom/CMakeLists.txt does for liblwgeom. librttopo.so, which
# PostgreSQL co-loads through postgis_raster's GDAL, also exports rtalloc,
# rtrealloc and rterror, with a context argument first. Without hiding, ELF
# global symbol interposition binds our internal rt_core calls to librttopo's
# copy whenever postgis_raster is loaded before the extension, and the first
# raster the extension deserializes crashes the backend.
set_property(TARGET librtcore PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET librtcore PROPERTY POSITION_INDEPENDENT_CODE ON)

# rt_core compiles clean under the standard -Wall -Wextra on GCC/clang, so it is
# linted there like MEOS; MSVC keeps its warnings suppressed pending a second pass.
if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  target_compile_options(librtcore PRIVATE "/W0")  # MEOS
endif()
