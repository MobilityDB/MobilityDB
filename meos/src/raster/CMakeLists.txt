add_library(raster OBJECT raster_quadbin.c raquet.c raquet_gdal.c raster_rtcore.c)
target_include_directories(raster PRIVATE
  "${CMAKE_SOURCE_DIR}/meos/include/raster"
  "${CMAKE_SOURCE_DIR}/postgis/rt_core"
  ${GDAL_INCLUDE_DIR})
