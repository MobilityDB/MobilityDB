add_library(raster OBJECT raster_quadbin.c)
target_include_directories(raster PRIVATE
  "${CMAKE_SOURCE_DIR}/meos/include/raster")
