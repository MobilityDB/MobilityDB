#ifndef H3_EXT_DEFS_H
#define H3_EXT_DEFS_H

/* The following functions have the same name as the C symbols of the h3-pg
 * PostgreSQL extension. The MEOS library defines them (they are the public
 * binding surface); the MobilityDB PostgreSQL extension does not, so that
 * both the h3 and mobilitydb extensions can be loaded in the same server.
 * Input parses and output prints with libh3's stringToH3 / h3ToString —
 * hexadecimal with an optional "0x" prefix, no cell-validity check —
 * exactly as h3-pg does. Comparison / ordering / hashing operate on the
 * uint64 cell identifier; they carry no geographic meaning but are
 * required for ordering, grouping, and hashing. */

extern H3Index h3index_in(const char *str);
extern char *h3index_out(H3Index cell);
extern bool h3index_eq(H3Index a, H3Index b);
extern bool h3index_ne(H3Index a, H3Index b);
extern bool h3index_lt(H3Index a, H3Index b);
extern bool h3index_le(H3Index a, H3Index b);
extern bool h3index_gt(H3Index a, H3Index b);
extern bool h3index_ge(H3Index a, H3Index b);
extern int h3index_cmp(H3Index a, H3Index b);
extern uint32 h3index_hash(H3Index cell);

/* Cell operations. Each returns a heap-allocated `Set *` owned by the
 * caller; NULL on libh3 failure after raising a `meos_error`. */

/** Return all cells within `k` grid steps of `origin` (including
 * `origin` itself at k=0). */
extern Set *h3_grid_disk(H3Index origin, int k);

/** Return all children of `origin` at resolution `childRes`. */
extern Set *h3_cell_to_children(H3Index origin, int childRes);

/** Return the compacted representation of `cells` (finer cells merged
 * up into parents where the full hexagonal set of siblings is present). */
extern Set *h3_compact_cells(const Set *cells);

/** Return the uncompacted representation of `cells` at resolution `res`
 * (fails if any input is finer than `res`). */
extern Set *h3_uncompact_cells(const Set *cells, int res);

/** Return all cells at exactly `k` grid steps from `origin`.
 * Fails near pentagons (libh3's unsafe ring). */
extern Set *h3_grid_ring(H3Index origin, int k);

/** Return the cells on the inclusive path from `start` to `end`.
 * Fails on non-comparable resolutions or paths crossing pentagons. */
extern Set *h3_grid_path_cells(H3Index start, H3Index end);

/** Return all outgoing directed edges of `origin` (up to 6;
 * pentagons have 5). */
extern Set *h3_origin_to_directed_edges(H3Index origin);

/** Return all vertexes of `cell` (up to 6; pentagons have 5). */
extern Set *h3_cell_to_vertexes(H3Index cell);

/** Return the icosahedron face indexes intersected by `cell` as an
 * intset. Each face index is in 0..19. */
extern Set *h3_get_icosahedron_faces(H3Index cell);

#endif /* H3_EXT_DEFS_H */
