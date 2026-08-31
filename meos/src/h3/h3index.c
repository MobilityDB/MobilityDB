/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Static h3index SQL type — parser, output, comparison /
 * hash helpers, plus every `datum_h3_*` Datum-calling-convention
 * wrapper consumed by the th3index lifting layer.
 *
 * Organisation follows cbuffer's convention: the base-type file
 * owns both the scalar MEOS API AND the Datum wrappers that lift
 * each scalar into the temporal-function machinery.
 *
 * Every helper here treats h3index as a uint64-backed value with
 * h3-pg-style hex output. Comparison and hashing reduce to plain
 * int64 bit operations — they carry no geographic meaning but are
 * required for btree indexing, ORDER BY, GROUP BY, DISTINCT, etc.
 *
 * Datum packing for H3Index values uses `DatumGetH3Index` /
 * `H3IndexGetDatum` from `h3index.h`. The wrappers that consume or
 * produce genuinely-int64 payloads (child positions, grid hop
 * counts) use `DatumGetInt64` / `Int64GetDatum` directly so the
 * distinction between a cell id and a plain integer is visible.
 */

#include "h3/h3index.h"

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
/* PostgreSQL */
#include <postgres.h>
/* H3 */
#include <h3api.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include <pg_int.h>
#include "temporal/temporal.h"
#include "temporal/type_inout.h"
#include "h3/h3_generated.h"
#include "h3/th3index_internal.h"

/*****************************************************************************
 * Validity predicates
 *
 * An h3index is a mode-tagged 64-bit identifier: the mode bits say whether
 * the value denotes a cell, a directed edge, or a vertex, and the three
 * predicates below are the only way to tell the modes apart. They are total
 * functions on the uint64 domain — they answer false rather than raising,
 * so they accept the invalid sentinel 0 and any other bit pattern, and are
 * therefore the primitives the input function and the per-mode validators
 * are written in terms of.
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_accessor
 * @brief Return true if an h3index is a valid H3 cell
 * @param[in] cell H3 index
 * @csqlfn #H3index_is_valid_cell()
 */
#if MEOS
bool
h3_is_valid_cell(H3Index cell)
{
  return h3_is_valid_cell_meos(cell);
}
#endif

/**
 * @ingroup meos_h3_base_accessor
 * @brief Return true if an h3index is a valid H3 directed edge
 * @param[in] edge H3 index
 * @csqlfn #H3index_is_valid_directed_edge()
 */
#if MEOS
bool
h3_is_valid_directed_edge(H3Index edge)
{
  return h3_is_valid_directed_edge_meos(edge);
}
#endif

/**
 * @ingroup meos_h3_base_accessor
 * @brief Return true if an h3index is a valid H3 vertex
 * @param[in] vertex H3 index
 * @csqlfn #H3index_is_valid_vertex()
 */
#if MEOS
bool
h3_is_valid_vertex(H3Index vertex)
{
  return h3_is_valid_vertex_meos(vertex);
}
#endif

/*****************************************************************************
 * Validators
 *
 * The per-mode checks behind the `VALIDATE_H3INDEX_*` macros. The three
 * modes share the same 64-bit representation, so the message names the mode
 * the operation required: without it a mode mismatch is indistinguishable
 * from a plain typo.
 *****************************************************************************/

/**
 * @brief Ensure that an h3index is a valid H3 cell
 */
bool
ensure_h3index_cell(H3Index cell)
{
  if (h3_is_valid_cell_meos(cell))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "h3index %" PRIx64 " is not a valid H3 cell", (uint64_t) cell);
  return false;
}

/**
 * @brief Ensure that an h3index is a valid H3 directed edge
 */
bool
ensure_h3index_directed_edge(H3Index edge)
{
  if (h3_is_valid_directed_edge_meos(edge))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "h3index %" PRIx64 " is not a valid H3 directed edge", (uint64_t) edge);
  return false;
}

/**
 * @brief Ensure that an h3index is a valid H3 vertex
 */
bool
ensure_h3index_vertex(H3Index vertex)
{
  if (h3_is_valid_vertex_meos(vertex))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "h3index %" PRIx64 " is not a valid H3 vertex", (uint64_t) vertex);
  return false;
}

/*****************************************************************************
 * Scalar MEOS API for the shared DggsCellOps faces
 *
 * The faces every grid publishes, spelled as `quadbin_*` and `s2cell_*` spell
 * them, over the h3-pg imports of `h3_generated.h`. The unit is the one the
 * `DggsCellOps` descriptor fixes: an area is square metres, so the unit
 * argument `h3_cell_area_meos()` carries does not reach this surface.
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_inspection
 * @brief Return the resolution of an H3 cell
 * @param[in] cell H3 cell
 * @csqlfn #H3index_get_resolution()
 */
uint32_t
h3index_get_resolution(H3Index cell)
{
  /* Ensure the validity of the arguments */
  if (! ensure_h3index_cell(cell))
    return 0;
  return (uint32_t) h3_get_resolution_meos(cell);
}

/**
 * @ingroup meos_h3_base_hierarchy
 * @brief Return the parent of an H3 cell at a coarser resolution
 * @param[in] cell H3 cell
 * @param[in] parent_resolution Target resolution (<= resolution of @p cell)
 * @return The parent cell, or 0 on error
 * @csqlfn #H3index_cell_to_parent()
 */
H3Index
h3index_cell_to_parent(H3Index cell, uint32_t parent_resolution)
{
  /* Ensure the validity of the arguments */
  if (! ensure_h3index_cell(cell))
    return 0;
  if (parent_resolution > (uint32_t) h3_get_resolution_meos(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The parent resolution must not be finer than the resolution of the cell");
    return 0;
  }
  return h3_cell_to_parent_meos(cell, (int32) parent_resolution);
}

/**
 * @ingroup meos_h3_base_metrics
 * @brief Return the area in square metres of an H3 cell
 * @param[in] cell H3 cell
 * @csqlfn #H3index_cell_area()
 */
double
h3index_cell_area(H3Index cell)
{
  /* Ensure the validity of the arguments */
  if (! ensure_h3index_cell(cell))
    return 0.0;
  return h3_cell_area_meos(cell, H3_UNIT_M2);
}

/*****************************************************************************
 * Datum wrappers for inspection
 *****************************************************************************/

/**
 * @brief Return the resolution of an H3 cell
 */
Datum
datum_h3_get_resolution(Datum d)
{
  return Int32GetDatum(h3_get_resolution_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return the base cell number of an H3 cell
 */
Datum
datum_h3_get_base_cell_number(Datum d)
{
  return Int32GetDatum(h3_get_base_cell_number_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return true if a value is a valid H3 cell
 */
Datum
datum_h3_is_valid_cell(Datum d)
{
  return BoolGetDatum(h3_is_valid_cell_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return true if an H3 cell has a Class III resolution
 */
Datum
datum_h3_is_res_class_iii(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(d), (Datum) 0);
  return BoolGetDatum(h3_is_res_class_iii_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return true if an H3 cell is a pentagon
 */
Datum
datum_h3_is_pentagon(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(d), (Datum) 0);
  return BoolGetDatum(h3_is_pentagon_meos(DatumGetH3Index(d)));
}

/*****************************************************************************
 * Datum wrappers for hierarchy
 *
 * The (h3, resolution) forms are 2-arg Datum functions so they can be
 * plugged into `tfunc_temporal` with `numparam = 1`.
 *****************************************************************************/

/**
 * @brief Return the parent of an H3 cell at a given resolution
 */
Datum
datum_h3_cell_to_parent(Datum cell_d, Datum res_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return H3IndexGetDatum(h3_cell_to_parent_meos(DatumGetH3Index(cell_d),
    DatumGetInt32(res_d)));
}

/**
 * @brief Return the parent of an H3 cell at the next coarser resolution
 */
Datum
datum_h3_cell_to_parent_next(Datum cell_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return H3IndexGetDatum(h3_cell_to_parent_next_meos(DatumGetH3Index(cell_d)));
}

/**
 * @brief Return the center child of an H3 cell at a given resolution
 */
Datum
datum_h3_cell_to_center_child(Datum cell_d, Datum res_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return H3IndexGetDatum(h3_cell_to_center_child_meos(DatumGetH3Index(cell_d),
    DatumGetInt32(res_d)));
}

/**
 * @brief Return the center child of an H3 cell at the next finer resolution
 */
Datum
datum_h3_cell_to_center_child_next(Datum cell_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return H3IndexGetDatum(h3_cell_to_center_child_next_meos(
    DatumGetH3Index(cell_d)));
}

/**
 * @brief Return the position of an H3 cell among the children of its ancestor at
 * a given resolution
 */
Datum
datum_h3_cell_to_child_pos(Datum cell_d, Datum parent_res_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  /* Return is a position index (int64), not a cell — plain
   * Int64GetDatum is correct here. */
  return Int64GetDatum(h3_cell_to_child_pos_meos(DatumGetH3Index(cell_d),
    DatumGetInt32(parent_res_d)));
}

/**
 * @brief Return the child at a given position and resolution of an H3 cell
 */
Datum
datum_h3_child_pos_to_cell(Datum pos_d, Datum parent_d, Datum child_res_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(parent_d), (Datum) 0);
  /* pos_d carries a plain int64 child position. */
  return H3IndexGetDatum(h3_child_pos_to_cell_meos(DatumGetInt64(pos_d),
    DatumGetH3Index(parent_d), DatumGetInt32(child_res_d)));
}

/*****************************************************************************
 * Datum wrappers for directed edges
 *****************************************************************************/

/**
 * @brief Return true if two H3 cells are neighbors
 */
Datum
datum_h3_are_neighbor_cells(Datum origin_d, Datum dest_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(origin_d), (Datum) 0);
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(dest_d), (Datum) 0);
  return BoolGetDatum(h3_are_neighbor_cells_meos(DatumGetH3Index(origin_d),
    DatumGetH3Index(dest_d)));
}

/**
 * @brief Return the directed edge from an origin H3 cell to a destination one
 */
Datum
datum_h3_cells_to_directed_edge(Datum origin_d, Datum dest_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(origin_d), (Datum) 0);
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(dest_d), (Datum) 0);
  return H3IndexGetDatum(h3_cells_to_directed_edge_meos(
    DatumGetH3Index(origin_d), DatumGetH3Index(dest_d)));
}

/**
 * @brief Return true if a value is a valid H3 directed edge
 */
Datum
datum_h3_is_valid_directed_edge(Datum d)
{
  return BoolGetDatum(h3_is_valid_directed_edge_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return the origin cell of an H3 directed edge
 */
Datum
datum_h3_get_directed_edge_origin(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_DIRECTED_EDGE(DatumGetH3Index(d), (Datum) 0);
  return H3IndexGetDatum(h3_get_directed_edge_origin_meos(DatumGetH3Index(d)));
}

/**
 * @brief Return the destination cell of an H3 directed edge
 */
Datum
datum_h3_get_directed_edge_destination(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_DIRECTED_EDGE(DatumGetH3Index(d), (Datum) 0);
  return H3IndexGetDatum(h3_get_directed_edge_destination_meos(
    DatumGetH3Index(d)));
}

/**
 * @brief Return the boundary of an H3 directed edge as a geometry
 */
Datum
datum_h3_directed_edge_to_boundary(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_DIRECTED_EDGE(DatumGetH3Index(d), (Datum) 0);
  GSERIALIZED *gs = h3_directed_edge_to_gs_boundary(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/*****************************************************************************
 * Datum wrappers for vertices
 *****************************************************************************/

/**
 * @brief Return a vertex of an H3 cell given its vertex number
 */
Datum
datum_h3_cell_to_vertex(Datum cell_d, Datum vnum_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return H3IndexGetDatum(h3_cell_to_vertex_meos(DatumGetH3Index(cell_d),
    DatumGetInt32(vnum_d)));
}

/**
 * @brief Return the point of an H3 vertex
 */
Datum
datum_h3_vertex_to_latlng(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_VERTEX(DatumGetH3Index(d), (Datum) 0);
  GSERIALIZED *gs = h3_vertex_to_gs_point(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/**
 * @brief Return true if a value is a valid H3 vertex
 */
Datum
datum_h3_is_valid_vertex(Datum d)
{
  return BoolGetDatum(h3_is_valid_vertex_meos(DatumGetH3Index(d)));
}

/*****************************************************************************
 * Datum wrappers for grid traversal
 *****************************************************************************/

/**
 * @brief Return the grid distance in hops between two H3 cells
 */
Datum
datum_h3_grid_distance(Datum origin_d, Datum dest_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(origin_d), (Datum) 0);
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(dest_d), (Datum) 0);
  /* Return is a hop count (int64), not a cell. */
  return Int64GetDatum(h3_grid_distance_meos(DatumGetH3Index(origin_d),
    DatumGetH3Index(dest_d)));
}

/**
 * @brief Return the local IJ coordinates of an H3 cell relative to an origin cell
 */
Datum
datum_h3_cell_to_local_ij(Datum origin_d, Datum cell_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(origin_d), (Datum) 0);
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  GSERIALIZED *gs = h3_cell_to_local_ij_meos(DatumGetH3Index(origin_d),
    DatumGetH3Index(cell_d));
  return PointerGetDatum(gs);
}

/**
 * @brief Return the H3 cell at local IJ coordinates relative to an origin cell
 */
Datum
datum_h3_local_ij_to_cell(Datum origin_d, Datum coord_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(origin_d), (Datum) 0);
  const GSERIALIZED *coord = (GSERIALIZED *) DatumGetPointer(coord_d);
  return H3IndexGetDatum(h3_local_ij_to_cell_meos(
    DatumGetH3Index(origin_d), coord));
}

/*****************************************************************************
 * Datum wrappers for lat/lng conversions
 *****************************************************************************/

/**
 * @brief Return the H3 cell of a given resolution containing a point
 */
Datum
datum_h3_latlng_to_cell(Datum point_d, Datum res_d)
{
  const GSERIALIZED *point = (GSERIALIZED *) DatumGetPointer(point_d);
  return H3IndexGetDatum(geo_to_h3index_cell(point,
    DatumGetInt32(res_d)));
}

/**
 * @brief Return the center point of an H3 cell
 */
Datum
datum_h3_cell_to_latlng(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(d), (Datum) 0);
  GSERIALIZED *gs = h3index_cell_to_point(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/**
 * @brief Return the boundary of an H3 cell as a geometry
 */
Datum
datum_h3_cell_to_boundary(Datum d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(d), (Datum) 0);
  GSERIALIZED *gs = h3index_cell_to_boundary(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/*****************************************************************************
 * Datum wrappers for metrics
 *
 * These take the unit as an auxiliary parameter (H3Unit enum stored
 * in an Int32 datum — no temporal unit support).
 *****************************************************************************/

/**
 * @brief Return the area of an H3 cell in a given unit
 */
Datum
datum_h3_cell_area(Datum cell_d, Datum unit_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_CELL(DatumGetH3Index(cell_d), (Datum) 0);
  return Float8GetDatum(h3_cell_area_meos(DatumGetH3Index(cell_d),
    (H3Unit) DatumGetInt32(unit_d)));
}

/**
 * @brief Return the length of an H3 directed edge in a given unit
 */
Datum
datum_h3_edge_length(Datum edge_d, Datum unit_d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_H3INDEX_DIRECTED_EDGE(DatumGetH3Index(edge_d), (Datum) 0);
  return Float8GetDatum(h3_edge_length_meos(DatumGetH3Index(edge_d),
    (H3Unit) DatumGetInt32(unit_d)));
}

/**
 * @brief Return the great circle distance between two points in a given unit
 */
Datum
datum_h3_great_circle_distance(Datum a_d, Datum b_d, Datum unit_d)
{
  const GSERIALIZED *a = (GSERIALIZED *) DatumGetPointer(a_d);
  const GSERIALIZED *b = (GSERIALIZED *) DatumGetPointer(b_d);
  return Float8GetDatum(h3_gs_great_circle_distance_meos(a, b,
    (H3Unit) DatumGetInt32(unit_d)));
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_inout
 * @brief Parse a string into an H3Index. See header for the accepted
 * input shapes.
 * @sqlfn h3index_in()
 */
#if MEOS
H3Index
h3index_in(const char *str)
{
  return meos_h3index_in(str);
}
#endif
H3Index
meos_h3index_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, (H3Index) 0);

  /* The accepted input is an optional "0x" or "0X" prefix followed by at
   * least one and at most 16 significant hexadecimal digits, leading zeros
   * being insignificant, and nothing else. There is no cell-validity check.
   * The domain is bounded here rather than inherited from libh3, which
   * saturates a value wider than 64 bits and stops at the first character it
   * cannot read: both map strings that differ onto the value of an unrelated
   * cell, so distinct inputs would compare equal and corrupt joins, grouping
   * and deduplication. MobilityDB must yield identical results in every
   * binding, whichever version of libh3 the binding links, hence the check
   * precedes the delegation. The parsing itself is left to libh3 */
  const char *p = str;
  if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    p += 2;
  const char *start = p;
  while (*p == '0')
    p++;
  const char *significant = p;
  while (isxdigit((unsigned char) *p))
    p++;
  if (*p != '\0' || p == start)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "invalid h3index input \"%s\": expected hexadecimal digits with an "
      "optional \"0x\" prefix", str);
    return (H3Index) 0;
  }
  if (p - significant > 16)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "invalid h3index input \"%s\": at most 16 hexadecimal digits are "
      "allowed", str);
    return (H3Index) 0;
  }

  H3Index cell;
  H3Error err;
  if ((err = stringToH3(str, &cell)) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "invalid h3index input \"%s\": h3 error code %u", str, (unsigned) err);
    return (H3Index) 0;
  }

  /* The value must denote something in the H3 indexing system. A well-formed
   * hexadecimal literal is not by itself an h3index: the mode bits and the
   * digit sequence encode a cell, a directed edge or a vertex, and a literal
   * such as "abc" or "12345" is none of the three. Accepting it would let a
   * value with no location enter a spatiotemporal type, where it compares,
   * groups and indexes as if it were a place. The zero sentinel is admitted
   * because h3 reserves it as the conventional "no index" marker, which the
   * validity predicates report as false and which callers test for */
  if (cell != (H3Index) 0 && ! h3_is_valid_cell_meos(cell) &&
      ! h3_is_valid_directed_edge_meos(cell) &&
      ! h3_is_valid_vertex_meos(cell))
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "invalid h3index input \"%s\": not a valid H3 cell, directed edge or "
      "vertex", str);
    return (H3Index) 0;
  }
  return cell;
}

/*****************************************************************************/

/**
 * @ingroup meos_h3_base_inout
 * @brief Format an H3Index as its canonical hex string. Matches
 * h3-pg's `h3index_out` output: lowercase, no "0x" prefix, no
 * leading zeros.
 * @sqlfn h3index_out()
 */
#if MEOS
char *
h3index_out(H3Index cell)
{
  return meos_h3index_out(cell);
}
#endif
char *
meos_h3index_out(H3Index cell)
{
  /* Maximum 16 hex chars for 64 bits + null terminator. Delegate to
   * libh3, exactly as the h3-pg extension's output function does */
  char *buf = palloc(17);
  H3Error err;
  if ((err = h3ToString(cell, buf, 17)) != E_SUCCESS)
  {
    pfree(buf);
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "h3 library error %u while formatting an h3index", (unsigned) err);
    return NULL;
  }
  return buf;
}

/*****************************************************************************
 * WKB and HexWKB input/output functions
 *
 * An h3index is a geographic cell with the constant default SRID WGS84
 * (EPSG:4326) — like a PostGIS geography. These functions route the cell
 * through the generic type WKB machinery, which (as for every spatial base
 * type) emits the SRID only for the extended (EWKB) variant; the plain-WKB
 * form omits it and a reader assumes 4326. This is the same base-value I/O
 * surface npoint and cbuffer expose, so a downstream tool with no native
 * spatial extension can round-trip a cell value through MEOS alone.
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_inout
 * @brief Return an h3index from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #H3index_from_wkb()
 */
H3Index
h3index_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, (H3Index) 0);
  return DatumGetH3Index(type_from_wkb(wkb, size, T_H3INDEX));
}

/**
 * @ingroup meos_h3_base_inout
 * @brief Return an h3index from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #H3index_from_hexwkb()
 */
H3Index
h3index_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, (H3Index) 0);
  size_t size = strlen(hexwkb);
  return DatumGetH3Index(type_from_hexwkb(hexwkb, size, T_H3INDEX));
}

/**
 * @ingroup meos_h3_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of an h3index
 * @param[in] cell H3 cell id
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #H3index_as_wkb()
 */
uint8_t *
h3index_as_wkb(H3Index cell, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(H3IndexGetDatum(cell), T_H3INDEX, variant, size_out);
}

/**
 * @ingroup meos_h3_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of an h3index
 * @param[in] cell H3 cell id
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #H3index_as_hexwkb()
 */
char *
h3index_as_hexwkb(H3Index cell, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(H3IndexGetDatum(cell), T_H3INDEX,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Comparison / ordering
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if two h3index values are equal
 * @sqlop @p =
 */
#if MEOS
bool
h3index_eq(H3Index a, H3Index b)
{
  return a == b;
}
#endif
bool
meos_h3index_eq(H3Index a, H3Index b)
{
  return a == b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if two h3index values are not equal
 * @sqlop @p <>
 */
#if MEOS
bool
h3index_ne(H3Index a, H3Index b)
{
  return a != b;
}
#endif
bool
meos_h3index_ne(H3Index a, H3Index b)
{
  return a != b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is less than the second
 * @sqlop @p <
 */
#if MEOS
bool
h3index_lt(H3Index a, H3Index b)
{
  return a < b;
}
#endif
bool
meos_h3index_lt(H3Index a, H3Index b)
{
  return a < b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is less than or equal to
 * the second
 * @sqlop @p <=
 */
#if MEOS
bool
h3index_le(H3Index a, H3Index b)
{
  return a <= b;
}
#endif
bool
meos_h3index_le(H3Index a, H3Index b)
{
  return a <= b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is greater than the second
 * @sqlop @p >
 */
#if MEOS
bool
h3index_gt(H3Index a, H3Index b)
{
  return a > b;
}
#endif
bool
meos_h3index_gt(H3Index a, H3Index b)
{
  return a > b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is greater than or equal
 * to the second
 * @sqlop @p >=
 */
#if MEOS
bool
h3index_ge(H3Index a, H3Index b)
{
  return a >= b;
}
#endif
bool
meos_h3index_ge(H3Index a, H3Index b)
{
  return a >= b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return -1 / 0 / 1 depending on whether the first h3index is
 * less than, equal to, or greater than the second
 * @sqlfn h3index_cmp()
 */
#if MEOS
int
h3index_cmp(H3Index a, H3Index b)
{
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}
#endif
int
meos_h3index_cmp(H3Index a, H3Index b)
{
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/*****************************************************************************
 * Hashing
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_accessor
 * @brief Return the 32-bit hash value of an h3index — matches the
 * result `hashint8` would produce on the same bit pattern, which is
 * what the SQL hash opclass was previously delegating to.
 * @sqlfn hash()
 */
#if MEOS
uint32
h3index_hash(H3Index cell)
{
  return int64_hash((int64) cell);
}
#endif
uint32
meos_h3index_hash(H3Index cell)
{
  return int64_hash((int64) cell);
}

/*****************************************************************************/
