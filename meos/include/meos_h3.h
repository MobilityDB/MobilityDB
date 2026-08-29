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
 * @brief Public MEOS API for the temporal H3 index type (th3index).
 *
 * Implementations live in meos/src/h3/. The PG V1 wrappers in
 * mobilitydb/src/h3/ call these symbols.
 */

#ifndef __MEOS_H3_H__
#define __MEOS_H3_H__

#include <stdbool.h>
#include <stdint.h>
#include <h3api.h>
#include <meos.h>
#include <meos_geo.h>

/**
 * @brief Ensure that the temporal value is a temporal H3 cell.
 * Matches the pattern of `VALIDATE_TCBUFFER` / `VALIDATE_TBOOL`.
 */
#if MEOS
  #define VALIDATE_TH3INDEX(temp, ret) \
    do { \
      if (! ensure_not_null((void *) (temp)) || \
          ! ensure_temporal_isof_type((Temporal *) (temp), T_TH3INDEX) ) \
        return (ret); \
    } while (0)
#else
  #define VALIDATE_TH3INDEX(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TH3INDEX); \
      (void) (temp); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Ensure that an h3index is a valid H3 cell.
 * @details An h3index is a mode-tagged identifier, so the operation decides
 * which of the three predicates applies: a cell operation requires a cell, a
 * directed-edge operation a directed edge, a vertex operation a vertex. The
 * validity predicates themselves, and the mode-agnostic surfaces (input and
 * output, comparison, ordering, hashing) take any bit pattern by design and
 * must not use these macros.
 * @note Unlike the type-validation macros above, these validate in both
 * builds rather than asserting in the extension build. A temporal value's
 * type is settled by the SQL type system before a function sees it, so there
 * the assertion states a proven invariant, whereas the validity of an
 * h3index is settled by nothing: the type is provided by the h3 extension,
 * whose input function and bigint cast admit any 64-bit pattern. The value
 * is therefore user input at every call, and asserting on user input aborts
 * the backend instead of reporting the error.
 */
#define VALIDATE_H3INDEX_CELL(cell, ret) \
  do { \
    if (! ensure_h3index_cell(cell) ) \
      return (ret); \
  } while (0)

/**
 * @brief Ensure that an h3index is a valid H3 directed edge.
 * Matches the pattern of `VALIDATE_H3INDEX_CELL` above.
 */
#define VALIDATE_H3INDEX_DIRECTED_EDGE(edge, ret) \
  do { \
    if (! ensure_h3index_directed_edge(edge) ) \
      return (ret); \
  } while (0)

/**
 * @brief Ensure that an h3index is a valid H3 vertex.
 * Matches the pattern of `VALIDATE_H3INDEX_CELL` above.
 */
#define VALIDATE_H3INDEX_VERTEX(vertex, ret) \
  do { \
    if (! ensure_h3index_vertex(vertex) ) \
      return (ret); \
  } while (0)

/*****************************************************************************
 * Static h3index SQL type — analogue of meos_cbuffer.h's
 * static-cbuffer section.
 *****************************************************************************/

/* Input and output */

/* The bare-named I/O, comparison, and hashing functions collide with the
 * C symbols of the h3-pg PostgreSQL extension, so they exist only in the
 * MEOS library build; the extension build uses the meos_-prefixed twins
 * internally (see h3/h3index.h). The installed public header receives the
 * unconditional declarations spliced from h3_ext_defs.in.h so that every
 * binding generated from the MEOS catalog keeps the full surface. The WKB
 * functions stay inline because h3-pg defines no such symbols. */
#if MEOS
#include "h3_ext_defs.in.h"
#endif /* MEOS */

extern H3Index h3index_from_wkb(const uint8_t *wkb, size_t size);
extern H3Index h3index_from_hexwkb(const char *hexwkb);
extern uint8_t *h3index_as_wkb(H3Index cell, uint8_t variant, size_t *size_out);
extern char *h3index_as_hexwkb(H3Index cell, uint8_t variant, size_t *size_out);

/* The cell operations (each returning a heap-allocated `Set *` owned by
 * the caller; NULL on libh3 failure after raising a `meos_error`) are
 * declared in h3_ext_defs.in.h spliced above: their bare names collide
 * with the C symbols of the h3-pg PostgreSQL extension. */

/* Inspection */

extern uint32_t h3index_get_resolution(H3Index cell);

/* Hierarchy */

extern H3Index h3index_cell_to_parent(H3Index cell, uint32_t parent_resolution);

/* Geometry (lon/lat, SRID 4326) */

extern GSERIALIZED *h3index_cell_to_point(H3Index cell);
extern GSERIALIZED *h3index_cell_to_boundary(H3Index cell);

/* Metrics */

extern double h3index_cell_area(H3Index cell);

/*****************************************************************************
 * Type inheritance (analogue of meos_cbuffer.h's tcbuffer section)
 *****************************************************************************/

/* Input */
extern Temporal *th3index_in(const char *str);
extern TInstant *th3indexinst_in(const char *str);
extern TSequence *th3indexseq_in(const char *str, interpType interp);
extern TSequenceSet *th3indexseqset_in(const char *str);

/* Constructors */
extern Temporal *th3index_make(H3Index value, TimestampTz t);
extern TInstant *th3indexinst_make(H3Index value, TimestampTz t);
extern TSequence *th3indexseq_make(const H3Index *values,
  const TimestampTz *times, int count, bool lower_inc, bool upper_inc);
extern TSequenceSet *th3indexseqset_make(const TSequence **sequences, int count);

/* Accessors */
extern H3Index th3index_start_value(const Temporal *temp);
extern H3Index th3index_end_value(const Temporal *temp);
extern bool th3index_value_n(const Temporal *temp, int n, H3Index *result);
extern H3Index *th3index_values(const Temporal *temp, int *count);
extern bool th3index_value_at_timestamptz(const Temporal *temp, TimestampTz t,
  bool strict, H3Index *result);

/* MEOS-level conversions to/from tbigint */
extern Temporal *tbigint_to_th3index(const Temporal *temp);
extern Temporal *th3index_to_tbigint(const Temporal *temp);

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

extern int ever_eq_h3index_th3index(H3Index cell, const Temporal *temp);
extern int ever_eq_th3index_h3index(const Temporal *temp, H3Index cell);
extern int ever_ne_h3index_th3index(H3Index cell, const Temporal *temp);
extern int ever_ne_th3index_h3index(const Temporal *temp, H3Index cell);
extern int always_eq_h3index_th3index(H3Index cell, const Temporal *temp);
extern int always_eq_th3index_h3index(const Temporal *temp, H3Index cell);
extern int always_ne_h3index_th3index(H3Index cell, const Temporal *temp);
extern int always_ne_th3index_h3index(const Temporal *temp, H3Index cell);
extern int ever_eq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Temporal comparison operators
 *****************************************************************************/

extern Temporal *teq_h3index_th3index(H3Index cell, const Temporal *temp);
extern Temporal *teq_th3index_h3index(const Temporal *temp, H3Index cell);
extern Temporal *teq_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);
extern Temporal *tne_h3index_th3index(H3Index cell, const Temporal *temp);
extern Temporal *tne_th3index_h3index(const Temporal *temp, H3Index cell);
extern Temporal *tne_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Inspection
 *****************************************************************************/

extern Temporal *th3index_get_base_cell_number(const Temporal *temp);
extern Temporal *th3index_is_res_class_iii(const Temporal *temp);
extern Temporal *th3index_is_pentagon(const Temporal *temp);

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

extern Temporal *th3index_cell_to_parent_next(const Temporal *temp);
extern Temporal *th3index_cell_to_center_child(const Temporal *temp, int32 resolution);
extern Temporal *th3index_cell_to_center_child_next(const Temporal *temp);
extern Temporal *th3index_cell_to_child_pos(const Temporal *temp, int32 parent_res);
extern Temporal *th3index_child_pos_to_cell(const Temporal *child_pos,
  const Temporal *parent, int32 child_res);

/*****************************************************************************
 * Lat/Lng conversion
 *****************************************************************************/

extern Temporal *tgeogpoint_to_th3index(const Temporal *temp, int32 resolution);
extern Temporal *tgeompoint_to_th3index(const Temporal *temp, int32 resolution);
extern Temporal *th3index_to_tgeogpoint(const Temporal *temp);
extern Temporal *th3index_to_tgeompoint(const Temporal *temp);

extern Set    *h3index_to_set(H3Index cell);

/* Static geometry → H3 cell / cell set.  See meos/src/h3/h3_geo.c. */
extern H3Index geo_to_h3index_cell(const GSERIALIZED *point, int32 resolution);
extern Set    *geo_to_h3index_set(const GSERIALIZED *gs,    int32 resolution);
extern STBox  *h3index_to_stbox(H3Index cell);
extern STBox  *h3index_timestamptz_to_stbox(H3Index cell, TimestampTz t);
extern STBox  *h3index_tstzspan_to_stbox(H3Index cell, const Span *s);
extern int     ever_eq_h3indexset_th3index(const Set *cells,
                                                  const Temporal *th3idx);

/*****************************************************************************
 * Directed edges
 *****************************************************************************/

extern Temporal *th3index_are_neighbor_cells(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_cells_to_directed_edge(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_is_valid_directed_edge(const Temporal *edge);
extern Temporal *th3index_get_directed_edge_origin(const Temporal *edge);
extern Temporal *th3index_get_directed_edge_destination(const Temporal *edge);
extern Temporal *th3index_directed_edge_to_boundary(const Temporal *edge);

/*****************************************************************************
 * Vertices
 *****************************************************************************/

extern Temporal *th3index_cell_to_vertex(const Temporal *temp, int32 vertex_num);
extern Temporal *th3index_vertex_to_latlng(const Temporal *temp);
extern Temporal *th3index_is_valid_vertex(const Temporal *temp);

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

extern Temporal *th3index_grid_distance(const Temporal *origin,
  const Temporal *dest);
extern Temporal *th3index_cell_to_local_ij(const Temporal *origin,
  const Temporal *cell);
extern Temporal *th3index_local_ij_to_cell(const Temporal *origin,
  const Temporal *coord);

/*****************************************************************************
 * Metrics
 *****************************************************************************/

extern Temporal *th3index_edge_length(const Temporal *temp);
extern Temporal *tgeogpoint_great_circle_distance(const Temporal *a,
  const Temporal *b);

#endif /* __MEOS_H3_H__ */
