set(S2CELL_SOURCES
  s2cell.c
  s2cell_geo.c
  s2cellset.c
  ts2cell.c
  ts2cell_boxops.c
  ts2cell_compops.c
  ts2cell_ops.c
)

add_library(s2cell OBJECT ${S2CELL_SOURCES})
