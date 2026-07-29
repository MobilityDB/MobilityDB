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

# Vendored upstream PostGIS sources that MobilityDB does not maintain or lint;
# suppress their compiler warnings on this target only, as done for the other
# vendored libraries (liblwgeom, ryu, pointcloud_libpc).
target_compile_options(librtcore PRIVATE "-w")
