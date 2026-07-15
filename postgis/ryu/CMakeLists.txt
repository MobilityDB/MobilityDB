add_library(ryu OBJECT
  d2s.c
  )

# Hide the vendored ryu float-formatting symbols from the shared library's
# dynamic table (see postgis/liblwgeom/CMakeLists.txt). Established by #447.
set_property(TARGET ryu PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET ryu PROPERTY POSITION_INDEPENDENT_CODE ON)
