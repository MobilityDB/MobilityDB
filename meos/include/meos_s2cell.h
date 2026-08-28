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
 * @brief Public MEOS API for the temporal Google S2 index type (ts2cell) and
 * its static `s2cell` cell type.
 *
 * S2 has no maintained C API and its C++ library is a heavy Abseil-dependent
 * blob, so the cell algebra is implemented FIRST-PARTY in meos/src/s2cell/ from
 * the public S2 cell-id specification — cube-face projection plus a Hilbert
 * space-filling curve — exactly as the quadbin family is first-partied from
 * the CARTO bit-spec. It links only libm.
 *
 * Implementations live in meos/src/s2cell/. The PG V1 wrappers in
 * mobilitydb/src/s2cell/ call these symbols.
 */

#ifndef __MEOS_S2CELL_H__
#define __MEOS_S2CELL_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/*****************************************************************************
 * Definitions
 *****************************************************************************/

/**
 * @brief A Google S2 cell index, packed into a 64-bit integer.
 * @details S2 is a quadtree Discrete Global Grid System on the sphere: the
 * six faces of a circumscribed cube are projected onto the sphere and each
 * face is recursively subdivided into four children. A cell identifier packs
 * the face in its three leading bits, the position along the Hilbert curve in
 * the following `2 * level` bits, and a single trailing sentinel bit that
 * records the level. Levels range 0..30, and cells are geodetic (SRID 4326).
 */
typedef uint64 S2CellId;

/** @brief Coarsest S2 level, one of the six cube faces */
#define S2_MIN_LEVEL     0
/** @brief Finest S2 level, a leaf cell of about one square centimetre */
#define S2_MAX_LEVEL     30
/** @brief Number of cube faces the sphere is decomposed into */
#define S2_NUM_FACES     6
/** @brief Maximum length of the canonical S2 token, without the terminator */
#define S2_TOKEN_MAXLEN  16

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Ensure that the temporal value is a temporal S2 cell.
 * Matches the pattern of `VALIDATE_TQUADBIN` / `VALIDATE_TH3INDEX`.
 */
#if MEOS
  #define VALIDATE_TS2CELL(temp, ret) \
    do { \
      if (! ensure_not_null((void *) (temp)) || \
          ! ensure_temporal_isof_type((Temporal *) (temp), T_TS2CELL) ) \
        return (ret); \
    } while (0)
#else
  #define VALIDATE_TS2CELL(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TS2CELL); \
      (void) (temp); \
    } while (0)
#endif /* MEOS */

/*===========================================================================*
 * Functions for S2 cells
 *===========================================================================*/

/* Input and output functions */

extern S2CellId s2cell_in(const char *str);
extern char *s2cell_out(S2CellId cell);

/* Comparison functions */

extern bool s2cell_eq(S2CellId a, S2CellId b);
extern bool s2cell_ne(S2CellId a, S2CellId b);
extern bool s2cell_lt(S2CellId a, S2CellId b);
extern bool s2cell_le(S2CellId a, S2CellId b);
extern bool s2cell_gt(S2CellId a, S2CellId b);
extern bool s2cell_ge(S2CellId a, S2CellId b);
extern int s2cell_cmp(S2CellId a, S2CellId b);

/* Hash functions */

extern uint32 s2cell_hash(S2CellId cell);
extern uint64 s2cell_hash_extended(S2CellId cell, uint64 seed);

/*****************************************************************************
 * Static `s2cell` kernel (first-party; the S2 analogue of libh3 for H3).
 * These are the pure-uint64 / libm primitives the temporal layer lifts.
 *****************************************************************************/

/* Validity */

extern bool s2cell_is_valid_cell(S2CellId cell);

/* Token conversion — NO H3/quadbin analogue (S2-specific) */

extern char *s2cell_cell_to_token(S2CellId cell);
extern S2CellId s2cell_token_to_cell(const char *token);

/* Inspection */

extern uint32_t s2cell_get_resolution(S2CellId cell);
extern uint32_t s2cell_get_face(S2CellId cell);

/* Hierarchy */

extern S2CellId s2cell_cell_to_parent(S2CellId cell, uint32_t level);
extern S2CellId s2cell_cell_to_child(S2CellId cell, uint32_t level,
  uint32_t position);
extern S2CellId *s2cell_cell_to_children(S2CellId cell, uint32_t level,
  int *count);
extern bool s2cell_cell_contains(S2CellId cell, S2CellId other);
extern int s2cell_common_ancestor_level(S2CellId a, S2CellId b);

/* Hilbert range — the contiguous descendant interval of a cell */

extern S2CellId s2cell_range_min(S2CellId cell);
extern S2CellId s2cell_range_max(S2CellId cell);

/* Traversal */

extern S2CellId *s2cell_edge_neighbors(S2CellId cell, int *count);

/* Geometry conversion */

extern S2CellId s2cell_point_to_cell(double longitude, double latitude,
  uint32_t level);

/* Metrics */

extern double s2cell_cell_area(S2CellId cell);
extern double s2cell_edge_length(S2CellId cell, uint32_t edge);

/* Geometry (lon/lat, SRID 4326) */

extern S2CellId geo_to_s2cell_cell(const GSERIALIZED *point, int32 level);
extern GSERIALIZED *s2cell_cell_to_geogpoint(S2CellId cell);
extern GSERIALIZED *s2cell_cell_to_geog(S2CellId cell);

/* Bounding box */

extern STBox *s2cell_to_stbox(S2CellId cell);
extern STBox *s2cell_timestamptz_to_stbox(S2CellId cell, TimestampTz t);
extern STBox *s2cell_tstzspan_to_stbox(S2CellId cell, const Span *s);

/* Set-returning static helpers (return `s2cellset`) */

extern Set *s2cell_edge_neighbors_set(S2CellId cell);
extern Set *s2cell_cell_to_children_set(S2CellId cell, int children_level);

/*****************************************************************************
 * Temporal `ts2cell` inheritance
 *****************************************************************************/

/* Input and output */

extern Temporal *ts2cell_in(const char *str);
extern TInstant *ts2cellinst_in(const char *str);
extern TSequence *ts2cellseq_in(const char *str, interpType interp);
extern TSequenceSet *ts2cellseqset_in(const char *str);

/* Constructors */

extern Temporal *ts2cell_make(S2CellId value, TimestampTz t);
extern TInstant *ts2cellinst_make(S2CellId value, TimestampTz t);
extern TSequence *ts2cellseq_make(const S2CellId *values,
  const TimestampTz *times, int count, bool lower_inc, bool upper_inc);
extern TSequenceSet *ts2cellseqset_make(const TSequence **sequences,
  int count);

/* Accessors */

extern S2CellId ts2cell_start_value(const Temporal *temp);
extern S2CellId ts2cell_end_value(const Temporal *temp);
extern bool ts2cell_value_n(const Temporal *temp, int n, S2CellId *result);
extern S2CellId *ts2cell_values(const Temporal *temp, int *count);
extern bool ts2cell_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, S2CellId *result);

/* Conversions */

extern Temporal *tbigint_to_ts2cell(const Temporal *temp);
extern Temporal *ts2cell_to_tbigint(const Temporal *temp);

/* Ever, always and temporal comparisons */

extern int ever_eq_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern int ever_eq_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern int ever_ne_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern int ever_ne_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern int always_eq_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern int always_eq_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern int always_ne_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern int always_ne_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern int ever_eq_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *teq_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern Temporal *teq_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern Temporal *teq_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *tne_s2cell_ts2cell(S2CellId cell, const Temporal *temp);
extern Temporal *tne_ts2cell_s2cell(const Temporal *temp, S2CellId cell);
extern Temporal *tne_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);

/* Token conversion, which no other DGGS carries */

extern Temporal *ts2cell_cell_to_token(const Temporal *temp);

/*****************************************************************************/

#endif /* __MEOS_S2CELL_H__ */
