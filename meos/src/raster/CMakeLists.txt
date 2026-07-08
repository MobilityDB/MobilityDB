add_library(raster OBJECT raster_quadbin.c raquet.c raquet_gdal.c)
target_include_directories(raster PRIVATE
  "${CMAKE_SOURCE_DIR}/meos/include/raster"
  ${GDAL_INCLUDE_DIR})
