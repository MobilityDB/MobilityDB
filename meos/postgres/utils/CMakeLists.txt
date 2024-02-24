add_library(utils OBJECT
  date.c
  datetime.c
  float.c
  formatting.c
  numutils.c
  timestamp.c
  )

set_property(TARGET liblwgeom PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET liblwgeom PROPERTY POSITION_INDEPENDENT_CODE ON)
