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

# rt_core compiles clean under the standard -Wall -Wextra on GCC/clang, so it is
# linted there like MEOS; MSVC keeps its warnings suppressed pending a second pass.
if(CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
  target_compile_options(librtcore PRIVATE "/W0")  # MEOS
endif()
