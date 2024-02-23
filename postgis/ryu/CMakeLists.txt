add_library(ryu OBJECT
  d2s.c
  )

set_property(TARGET liblwgeom PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET liblwgeom PROPERTY POSITION_INDEPENDENT_CODE ON)
