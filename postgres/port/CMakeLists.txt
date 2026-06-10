set(PORT_SOURCES
  pgstrcasecmp.c
  qsort.c
  qsort_arg.c
  snprintf.c
  )

# Native Windows MSVC has no <dirent.h>; MEOS code in postgres/common/pgfnames.c
# and postgres/timezone/pgtz.c includes it.  Add the vendored shim
# (BSD-licensed from PostgreSQL upstream).  MinGW already provides its own
# <dirent.h> implementation, so the .c body is itself guarded against MinGW.
if(WIN32)
  list(APPEND PORT_SOURCES dirent.c)
endif()

add_library(port OBJECT ${PORT_SOURCES})

set_property(TARGET port PROPERTY C_VISIBILITY_PRESET hidden)
set_property(TARGET port PROPERTY POSITION_INDEPENDENT_CODE ON)
