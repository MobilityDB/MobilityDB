/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Public MEOS API for the temporal CARTO QUADBIN index type
 * (tquadbin) and its static `quadbin` cell type.
 *
 * Unlike the temporal H3 index (which links the vendored libh3 / h3-pg), the
 * quadbin cell algebra is implemented FIRST-PARTY in meos/src/quadbin/ from the
 * public CARTO quadbin bit-specification — no vendored code, no external link
 * dependency, libm only. The static-cell kernel below is the analogue of what
 * libh3 provides to the H3 family.
 *
 * Implementations live in meos/src/quadbin/. The PG V1 wrappers in
 * mobilitydb/src/quadbin/ call these symbols.
 */

#ifndef __MEOS_QUADBIN_H__
#define __MEOS_QUADBIN_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/**
 * @brief Ensure that the temporal value is a temporal quadbin cell.
 * Matches the pattern of `VALIDATE_TH3INDEX` / `VALIDATE_TBOOL`.
 */
#if MEOS
  #define VALIDATE_TQUADBIN(temp, ret) \
    do { \
      if (! ensure_not_null((void *) (temp)) || \
          ! ensure_temporal_isof_type((Temporal *) (temp), T_TQUADBIN) ) \
        return (ret); \
    } while (0)
#else
  #define VALIDATE_TQUADBIN(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TQUADBIN); \
      (void) (temp); \
    } while (0)
#endif /* MEOS */

/**
 * @brief A CARTO quadbin cell index, packed into a 64-bit integer.
 *
 * Quadbin is a square quadtree Discrete Global Grid System: a Web-Mercator
 * (EPSG:3857) slippy-tile `(z, x, y)` encoded as a single uint64. Resolution
 * (zoom) ranges 0..26. This is the square/aperture-4 counterpart of the
 * hexagonal/aperture-7 H3Index.
 */
typedef uint64 Quadbin;

/*****************************************************************************
 * Static `quadbin` cell kernel (first-party; analogue of libh3 for H3).
 * These are the pure-uint64 / libm primitives the temporal layer lifts.
 *****************************************************************************/

/* Validity */
extern bool quadbin_is_valid_index(Quadbin index);
extern bool quadbin_is_valid_cell(Quadbin cell);

/* Tile (z/x/y quadkey) conversion — NO H3 analogue (quadbin-specific) */
extern Quadbin quadbin_tile_to_cell(uint32_t x, uint32_t y, uint32_t z);
extern void quadbin_cell_to_tile(Quadbin cell, uint32_t *x, uint32_t *y,
  uint32_t *z);

/* Inspection */
extern uint32_t quadbin_get_resolution(Quadbin cell);

/* Hierarchy */
extern Quadbin quadbin_cell_to_parent(Quadbin cell, uint32_t parent_resolution);
extern Quadbin *quadbin_cell_to_children(Quadbin cell,
  uint32_t children_resolution, int *count);
extern Quadbin quadbin_cell_sibling(Quadbin cell, const char *direction);

/* Grid traversal */
extern Quadbin *quadbin_k_ring(Quadbin cell, int k, int *count);

/* Lat/Lng (Web-Mercator) */
extern Quadbin quadbin_point_to_cell(double longitude, double latitude,
  uint32_t resolution);
extern void quadbin_cell_to_point(Quadbin cell, double *longitude,
  double *latitude);
extern void quadbin_cell_to_bounding_box(Quadbin cell, double *xmin,
  double *ymin, double *xmax, double *ymax);

/* Geometry (lon/lat, SRID 4326) */
extern Quadbin geo_to_quadbin_cell(const GSERIALIZED *point,
  int32 resolution);
extern GSERIALIZED *quadbin_cell_to_geompoint(Quadbin cell);
extern GSERIALIZED *quadbin_cell_to_geom(Quadbin cell);

/* Bounding box */
extern STBox *quadbin_to_stbox(Quadbin cell);
extern STBox *quadbin_timestamptz_to_stbox(Quadbin cell, TimestampTz t);
extern STBox *quadbin_tstzspan_to_stbox(Quadbin cell, const Span *s);

/* Metrics */
extern double quadbin_cell_area(Quadbin cell);

/* Serialization (lowercase hex of the 64-bit index) */
extern char *quadbin_index_to_string(Quadbin index);
extern Quadbin quadbin_string_to_index(const char *str);

/* Quadkey (base-4 slippy-tile string) — NO H3 analogue (quadbin-specific) */
extern char *quadbin_cell_to_quadkey(Quadbin cell);

/* Static `quadbin` SQL-type I/O (validating parser + ordering / hashing) */
extern Quadbin quadbin_parse(const char *str);
extern bool quadbin_eq(Quadbin a, Quadbin b);
extern bool quadbin_ne(Quadbin a, Quadbin b);
extern bool quadbin_lt(Quadbin a, Quadbin b);
extern bool quadbin_le(Quadbin a, Quadbin b);
extern bool quadbin_gt(Quadbin a, Quadbin b);
extern bool quadbin_ge(Quadbin a, Quadbin b);
extern int quadbin_cmp(Quadbin a, Quadbin b);
extern uint32_t quadbin_hash(Quadbin cell);

/* Set-returning static helpers (square subset; return `quadbinset`) */
extern Set *quadbin_grid_disk(Quadbin origin, int k);
extern Set *quadbin_cell_to_children_set(Quadbin origin,
  int children_resolution);

/*****************************************************************************
 * Temporal `tquadbin` inheritance — PENDING IMPLEMENTATION.
 *
 * Cloned mechanically from the th3index surface (see meos_h3.h), dropping the
 * hexagon-only families (directed edges, vertices, pentagon / base-cell /
 * class-III inspection) and adding the tile/quadkey conversions above. The
 * `// extern` commented form marks a forward declaration parked awaiting its
 * kernel implementation (project convention: still public/binding surface,
 * held to the same naming convention).
 *****************************************************************************/

/* Input */
extern Temporal *tquadbin_in(const char *str);
extern TInstant *tquadbininst_in(const char *str);
extern TSequence *tquadbinseq_in(const char *str, interpType interp);
extern TSequenceSet *tquadbinseqset_in(const char *str);

/* Constructors */
extern Temporal *tquadbin_make(Quadbin value, TimestampTz t);
extern TInstant *tquadbininst_make(Quadbin value, TimestampTz t);
extern TSequence *tquadbinseq_make(const Quadbin *values,
  const TimestampTz *times, int count, bool lower_inc, bool upper_inc);
extern TSequenceSet *tquadbinseqset_make(const TSequence **sequences, int count);

/* Accessors */
extern Quadbin tquadbin_start_value(const Temporal *temp);
extern Quadbin tquadbin_end_value(const Temporal *temp);
extern bool tquadbin_value_n(const Temporal *temp, int n, Quadbin *result);
extern Quadbin *tquadbin_values(const Temporal *temp, int *count);
extern bool tquadbin_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, Quadbin *result);

/* MEOS-level conversions to/from tbigint */
extern Temporal *tbigint_to_tquadbin(const Temporal *temp);
extern Temporal *tquadbin_to_tbigint(const Temporal *temp);

/* Ever/always + temporal comparison operators (eq/ne) — as th3index */
extern int ever_eq_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern int ever_eq_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern int ever_ne_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern int ever_ne_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern int always_eq_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern int always_eq_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern int always_ne_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern int always_ne_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern int ever_eq_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *teq_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern Temporal *teq_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern Temporal *teq_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *tne_quadbin_tquadbin(Quadbin cell, const Temporal *temp);
extern Temporal *tne_tquadbin_quadbin(const Temporal *temp, Quadbin cell);
extern Temporal *tne_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);

/* Inspection (square subset: resolution + validity only) */
// extern Temporal *tquadbin_get_resolution(const Temporal *temp);
// extern Temporal *tquadbin_is_valid_cell(const Temporal *temp);

/* Hierarchy */
// extern Temporal *tquadbin_cell_to_parent(const Temporal *temp, int32 resolution);
// extern Temporal *tquadbin_cell_to_parent_next(const Temporal *temp);
// extern Temporal *tquadbin_cell_to_children(const Temporal *temp, int32 resolution);

/* Lat/Lng conversion (Web-Mercator centroid / boundary) */
// extern Temporal *tgeompoint_to_tquadbin(const Temporal *temp, int32 resolution);
// extern Temporal *tquadbin_to_tgeompoint(const Temporal *temp);
// extern Temporal *tquadbin_cell_to_boundary(const Temporal *temp);

/* Quadkey conversion (no H3 analogue): temporal base-4 slippy-tile string */
extern Temporal *tquadbin_cell_to_quadkey(const Temporal *temp);

/* Grid traversal + metrics */
// extern Temporal *tquadbin_grid_distance(const Temporal *origin,
//   const Temporal *dest);
// extern Temporal *tquadbin_cell_area(const Temporal *temp);

/* Static geometry → quadbin cell set + ever-intersects predicate */
// extern Set *geo_to_quadbin_set(const GSERIALIZED *gs, int32 resolution);
// extern int ever_eq_quadbinset_tquadbin(const Set *cells, const Temporal *tqb);

#endif /* __MEOS_QUADBIN_H__ */
