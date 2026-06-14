set(QUADBIN_SRCS
  # First-party static quadbin cell kernel — the MobilityDB-owned analogue of
  # libh3 (no vendored code, libm only): tile/cell conversion, inspection,
  # hierarchy, traversal, Web-Mercator lat/lng, metrics, serialization.
  quadbin.c

  # Static `quadbin` SQL type — validating parser plus comparison / hash
  # helpers consumed by the central temporal type I/O dispatch.
  quadbin_meos.c

  # Set-returning static helpers (square subset; return `quadbinset`).
  quadbinset_meos.c

  # Datum wrappers + the quadbin_cellops descriptor that plugs the kernel into
  # the shared temporal cell-index machinery (meos/src/temporal/tcellindex.c).
  tquadbin_ops.c

  # Temporal `tquadbin` type-inheritance boilerplate: input, constructors,
  # accessors, validators, and the tbigint conversions.
  tquadbin.c

  # Planar bounding box functions for the temporal `tquadbin` subtypes,
  # dispatched from meos/src/geo/tgeo_boxops.c.
  tquadbin_boxops.c
)

add_library(quadbin OBJECT ${QUADBIN_SRCS})
