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
  # 274_th3index_accessors.in.sql routes directly to the generic
  # Temporal_start_value / Temporal_value_n / … symbols, so no
  # standalone C wrapper file is needed for the accessors.
  )
