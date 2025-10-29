set(MB
  conv.c
  mbutils.c
  stringinfo_mb.c
  wstrcmp.c
  wstrncmp.c
)

add_library(mb OBJECT ${MB})

add_subdirectory("conversion_procs")
