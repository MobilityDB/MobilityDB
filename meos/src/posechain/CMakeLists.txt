set(POSECHAIN_SOURCES
  posechain.c
  tposechain.c
  tposechain_boxops.c
  tposechain_compops.c
)

if(MEOS)
  list(APPEND POSECHAIN_SOURCES
  posechainset_meos.c
)
endif()

add_library(posechain OBJECT ${POSECHAIN_SOURCES})
