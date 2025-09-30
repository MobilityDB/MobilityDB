add_library(timezone OBJECT
  findtimezone.c
  localtime.c
  pgtz.c
  )

# set_property(TARGET timezone PROPERTY C_VISIBILITY_PRESET hidden)
# set_property(TARGET timezone PROPERTY POSITION_INDEPENDENT_CODE ON)
