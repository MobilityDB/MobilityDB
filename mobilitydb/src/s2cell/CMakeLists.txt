add_library(pg_s2cell OBJECT
  s2cell.c
  s2cell_ops.c
  ts2cell.c
  ts2cell_compops.c
  ts2cell_ops.c
  # The subset shared with every DGGS family plus the operations that
  # are S2's own: the cube face, the Hilbert range a cell spans, the
  # level of the deepest common ancestor, and the token. There is no
  # k-ring — the Hilbert curve gives four edge neighbours and no ring
  # of a given radius.
  #
  # 601_s2cellset.in.sql routes to the generic Set_* symbols, so the
  # set machinery needs no standalone C wrapper file.
  )
