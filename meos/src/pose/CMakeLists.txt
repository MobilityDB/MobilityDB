set(POSE_SOURCES
  pose.c
  pose_geopose.c
  posechain.c
  tpose.c
  tpose_boxops.c
  tpose_compops.c
  tpose_distance.c
  tpose_spatialfuncs.c
  tposechain.c
  tposechain_boxops.c
  tposechain_compops.c
)

if(MEOS)
  list(APPEND POSE_SOURCES
  posechainset_meos.c
  poseset_meos.c
)
endif()

add_library(pose OBJECT ${POSE_SOURCES})
