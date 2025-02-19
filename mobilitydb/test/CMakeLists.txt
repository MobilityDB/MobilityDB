add_subdirectory(scripts)
add_subdirectory(general)
add_subdirectory(point)
if(CBUFFER)
  add_subdirectory(cbuffer)
endif()
if(GEO)
  add_subdirectory(geo)
endif()
if(NPOINT)
  add_subdirectory(npoint)
endif()


