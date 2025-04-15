set(POSE_SRCS
  pose.c
  tpose.c
  tpose_compops.c
  tpose_boxops.c
  tpose_distance.c
  tpose_spatialfuncs.c
)

if(MEOS)
  list(APPEND POSE_SRCS
  poseset_meos.c
)
endif()

add_library(pose OBJECT ${POSE_SRCS})