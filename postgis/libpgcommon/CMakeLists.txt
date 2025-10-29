add_library(libpgcommon OBJECT
  gserialized_gist.c
  lwgeom_cache.c
  lwgeom_pg.c
  lwgeom_transform.c
  shared_gserialized.c
  )

# set_property(TARGET libpgcommon PROPERTY C_VISIBILITY_PRESET hidden)
# set_property(TARGET libpgcommon PROPERTY POSITION_INDEPENDENT_CODE ON)
