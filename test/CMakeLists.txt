

add_subdirectory(scripts)
add_subdirectory(general)

if (HAS_LWGEOM)
add_subdirectory(point)
add_subdirectory(npoint)
endif()

