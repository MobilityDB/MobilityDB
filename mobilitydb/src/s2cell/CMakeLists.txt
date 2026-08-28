add_library(pg_s2cell OBJECT
  s2cell.c
  ts2cell.c
  # 601_s2cellset.in.sql routes to the generic Set_* symbols, so the
  # set machinery needs no standalone C wrapper file.
  )
