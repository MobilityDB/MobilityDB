set(POSECHAIN_SOURCES
  posechain.c
)

if(MEOS)
  list(APPEND POSECHAIN_SOURCES
  posechainset_meos.c
)
endif()

add_library(posechain OBJECT ${POSECHAIN_SOURCES})
