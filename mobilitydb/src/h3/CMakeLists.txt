add_library(pg_h3 OBJECT
  h3index.c
  h3index_sets.c
  th3index.c
  th3index_inspection.c
  th3index_hierarchy.c
  th3index_latlng.c
  th3index_edges.c
  th3index_vertices.c
  th3index_traversal.c
  th3index_metrics.c
  th3index_compops.c
  # Static-geometry → H3 cell / cell set + the Set ever-intersects
  # predicate that consumes the resulting cell set against a th3index
  # trip path (cross-platform spatial prefilter).
  h3_geo.c
  # 287_th3index_casts.in.sql reuses functions defined in 282 — no
  # standalone C wrapper file needed for the cast SQL.
  # 274_th3index_accessors.in.sql routes directly to the generic
  # Temporal_start_value / Temporal_value_n / … symbols, so no
  # standalone C wrapper file is needed for the accessors either.
  )
