set(H3_SRCS
  # Internal — auto-generated static h3 wrappers.
  h3_generated.c

  # Static h3index SQL type — parser, output, comparison/hash
  # helpers, and every `datum_h3_*` Datum-calling-convention wrapper
  # used by the lifting layer (cbuffer convention: all Datum
  # wrappers live alongside the base type they belong to).
  h3index.c
  # Static set-returning h3-pg ports.
  h3index_sets.c

  # Public inheritance boilerplate: inout, constructors,
  # conversions, accessors, validators.
  th3index.c

  # Bounding box functions: geodetic STBox from H3 cell boundary.
  th3index_boxops.c

  # Per-section lifted public API, listed in the same order as
  # the @defgroup sequence in meos/include/h3/doxygen_meos_h3.h
  # and the chapter sections in doc/temporal_h3_index.xml. Each of
  # these also carries the hand-written static h3 adapters that
  # could not be auto-generated (GSERIALIZED conversions, unit-
  # string dispatch, next-resolution conveniences).
  th3index_inspection.c
  th3index_hierarchy.c
  th3index_latlng.c
  th3index_edges.c
  th3index_vertices.c
  th3index_traversal.c
  th3index_metrics.c

  # Comparison operators — last group in the doxygen hierarchy,
  # so last in the source list too.
  th3index_compops.c
)

add_library(h3 OBJECT ${H3_SRCS})
