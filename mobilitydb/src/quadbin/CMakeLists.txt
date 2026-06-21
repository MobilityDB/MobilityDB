add_library(pg_quadbin OBJECT
  quadbin.c
  quadbin_ops.c
  tquadbin.c
  tquadbin_ops.c
  tquadbin_compops.c
  # The square subset shared with every DGGS family plus the
  # quadbin-unique tile / quadkey conversions; the hexagon-only H3
  # families (directed edges, vertices, pentagon / base-cell /
  # class-III inspection, local-IJ traversal, great-circle metrics)
  # have no quadbin analogue and are not wrapped.
  #
  # 296_quadbinset.in.sql routes to the generic Set_* symbols and
  # 298_tquadbin.in.sql routes the temporal I/O / comparison to the
  # generic Temporal_* symbols, so no standalone C wrapper file is
  # needed for the quadbinset machinery nor for the tquadbin I/O.
  )
