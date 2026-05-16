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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

#include <postgres.h>

#include <meos.h>
#include <h3api.h>
#include "h3/h3_generated.h"
#include "h3/th3index_internal.h"
#include "temporal/postgres_types.h"

/*****************************************************************************
 * Parsing
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_inout
 * @brief Parse a string into an H3Index. See header for the accepted
 * input shapes.
 */
H3Index
h3index_parse(const char *str)
{
  if (str == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3index input must not be null");
    return (H3Index) 0;
  }

  /* Strip leading whitespace. */
  while (*str && isspace((unsigned char) *str))
    str++;

  /* Skip an optional "0x" / "0X" hex prefix. */
  bool force_hex = false;
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
  {
    force_hex = true;
    str += 2;
  }

  /* Detect representation:
   *   - if any character outside [0-9] appears (e.g. a-f), treat as hex.
   *   - otherwise treat as decimal.
   * H3 cells in hex are 15 chars; in decimal up to 20 chars (uint64 max).
   */
  bool is_hex = force_hex;
  if (! is_hex)
  {
    for (const char *p = str; *p && ! isspace((unsigned char) *p); p++)
    {
      if (! isdigit((unsigned char) *p))
      {
        is_hex = true;
        break;
      }
    }
  }

  H3Index cell = (H3Index) 0;
  char *end = NULL;
  errno = 0;
  if (is_hex)
    cell = (H3Index) strtoull(str, &end, 16);
  else
    cell = (H3Index) strtoull(str, &end, 10);

  if (errno != 0 || end == str || (end && *end != '\0' &&
      ! isspace((unsigned char) *end)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "invalid h3index input \"%s\"", str);
    return (H3Index) 0;
  }

  /* Reject the conventional "invalid" sentinel and anything libh3
   * does not consider a valid cell. */
  if (cell == (H3Index) 0 || ! h3_is_valid_cell_meos(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "h3index value %s does not encode a valid H3 cell", str);
    return (H3Index) 0;
  }

  return cell;
}

/*****************************************************************************
 * Output
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_inout
 * @brief Format an H3Index as its canonical hex string. Matches
 * h3-pg's `h3index_out` output: lowercase, no "0x" prefix, no
 * leading zeros.
 */
char *
h3index_to_string(H3Index cell)
{
  /* Maximum 16 hex chars for 64 bits + null terminator. */
  char *buf = palloc(17);
  snprintf(buf, 17, "%" PRIx64, (uint64_t) cell);
  return buf;
}

/*****************************************************************************
 * Comparison / ordering
 *****************************************************************************/

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if two h3index values are equal
 */
bool
h3index_eq(H3Index a, H3Index b)
{
  return a == b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if two h3index values are not equal
 */
bool
h3index_ne(H3Index a, H3Index b)
{
  return a != b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is less than the second
 */
bool
h3index_lt(H3Index a, H3Index b)
{
  return a < b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is less than or equal to
 * the second
 */
bool
h3index_le(H3Index a, H3Index b)
{
  return a <= b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is greater than the second
 */
bool
h3index_gt(H3Index a, H3Index b)
{
  return a > b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return true if the first h3index is greater than or equal
 * to the second
 */
bool
h3index_ge(H3Index a, H3Index b)
{
  return a >= b;
}

/**
 * @ingroup meos_h3_base_comp
 * @brief Return -1 / 0 / 1 depending on whether the first h3index is
 * less than, equal to, or greater than the second
 */
int
h3index_cmp(H3Index a, H3Index b)
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
 */
uint32
h3index_hash(H3Index cell)
{
  return pg_hashint8((int64) cell);
}

/*****************************************************************************
 * Datum wrappers for inspection
 *****************************************************************************/

Datum
datum_h3_get_resolution(Datum d)
{
  return Int32GetDatum(h3_get_resolution_meos(DatumGetH3Index(d)));
}

Datum
datum_h3_get_base_cell_number(Datum d)
{
  return Int32GetDatum(h3_get_base_cell_number_meos(DatumGetH3Index(d)));
}

Datum
datum_h3_is_valid_cell(Datum d)
{
  return BoolGetDatum(h3_is_valid_cell_meos(DatumGetH3Index(d)));
}

Datum
datum_h3_is_res_class_iii(Datum d)
{
  return BoolGetDatum(h3_is_res_class_iii_meos(DatumGetH3Index(d)));
}

Datum
datum_h3_is_pentagon(Datum d)
{
  return BoolGetDatum(h3_is_pentagon_meos(DatumGetH3Index(d)));
}

/*****************************************************************************
 * Datum wrappers for hierarchy
 *
 * The (h3, resolution) forms are 2-arg Datum functions so they can be
 * plugged into `tfunc_temporal` with `numparam = 1`.
 *****************************************************************************/

Datum
datum_h3_cell_to_parent(Datum cell_d, Datum res_d)
{
  return H3IndexGetDatum(h3_cell_to_parent_meos(
    DatumGetH3Index(cell_d), DatumGetInt32(res_d)));
}

Datum
datum_h3_cell_to_parent_next(Datum cell_d)
{
  return H3IndexGetDatum(h3_cell_to_parent_next_meos(
    DatumGetH3Index(cell_d)));
}

Datum
datum_h3_cell_to_center_child(Datum cell_d, Datum res_d)
{
  return H3IndexGetDatum(h3_cell_to_center_child_meos(
    DatumGetH3Index(cell_d), DatumGetInt32(res_d)));
}

Datum
datum_h3_cell_to_center_child_next(Datum cell_d)
{
  return H3IndexGetDatum(h3_cell_to_center_child_next_meos(
    DatumGetH3Index(cell_d)));
}

Datum
datum_h3_cell_to_child_pos(Datum cell_d, Datum parent_res_d)
{
  /* Return is a position index (int64), not a cell — plain
   * Int64GetDatum is correct here. */
  return Int64GetDatum(h3_cell_to_child_pos_meos(
    DatumGetH3Index(cell_d), DatumGetInt32(parent_res_d)));
}

Datum
datum_h3_child_pos_to_cell(Datum pos_d, Datum parent_d, Datum child_res_d)
{
  /* pos_d carries a plain int64 child position. */
  return H3IndexGetDatum(h3_child_pos_to_cell_meos(
    DatumGetInt64(pos_d),
    DatumGetH3Index(parent_d),
    DatumGetInt32(child_res_d)));
}

/*****************************************************************************
 * Datum wrappers for directed edges
 *****************************************************************************/

Datum
datum_h3_are_neighbor_cells(Datum origin_d, Datum dest_d)
{
  return BoolGetDatum(h3_are_neighbor_cells_meos(
    DatumGetH3Index(origin_d),
    DatumGetH3Index(dest_d)));
}

Datum
datum_h3_cells_to_directed_edge(Datum origin_d, Datum dest_d)
{
  return H3IndexGetDatum(h3_cells_to_directed_edge_meos(
    DatumGetH3Index(origin_d),
    DatumGetH3Index(dest_d)));
}

Datum
datum_h3_is_valid_directed_edge(Datum d)
{
  return BoolGetDatum(h3_is_valid_directed_edge_meos(
    DatumGetH3Index(d)));
}

Datum
datum_h3_get_directed_edge_origin(Datum d)
{
  return H3IndexGetDatum(h3_get_directed_edge_origin_meos(
    DatumGetH3Index(d)));
}

Datum
datum_h3_get_directed_edge_destination(Datum d)
{
  return H3IndexGetDatum(h3_get_directed_edge_destination_meos(
    DatumGetH3Index(d)));
}

Datum
datum_h3_directed_edge_to_boundary(Datum d)
{
  GSERIALIZED *gs = h3_directed_edge_to_gs_boundary(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/*****************************************************************************
 * Datum wrappers for vertices
 *****************************************************************************/

Datum
datum_h3_cell_to_vertex(Datum cell_d, Datum vnum_d)
{
  return H3IndexGetDatum(h3_cell_to_vertex_meos(
    DatumGetH3Index(cell_d), DatumGetInt32(vnum_d)));
}

Datum
datum_h3_vertex_to_latlng(Datum d)
{
  GSERIALIZED *gs = h3_vertex_to_gs_point(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

Datum
datum_h3_is_valid_vertex(Datum d)
{
  return BoolGetDatum(h3_is_valid_vertex_meos(DatumGetH3Index(d)));
}

/*****************************************************************************
 * Datum wrappers for grid traversal
 *****************************************************************************/

Datum
datum_h3_grid_distance(Datum origin_d, Datum dest_d)
{
  /* Return is a hop count (int64), not a cell. */
  return Int64GetDatum(h3_grid_distance_meos(
    DatumGetH3Index(origin_d),
    DatumGetH3Index(dest_d)));
}

Datum
datum_h3_cell_to_local_ij(Datum origin_d, Datum cell_d)
{
  GSERIALIZED *gs = h3_cell_to_local_ij_meos(
    DatumGetH3Index(origin_d),
    DatumGetH3Index(cell_d));
  return PointerGetDatum(gs);
}

Datum
datum_h3_local_ij_to_cell(Datum origin_d, Datum coord_d)
{
  const GSERIALIZED *coord = (GSERIALIZED *) DatumGetPointer(coord_d);
  return H3IndexGetDatum(h3_local_ij_to_cell_meos(
    DatumGetH3Index(origin_d), coord));
}

/*****************************************************************************
 * Datum wrappers for lat/lng conversions
 *****************************************************************************/

Datum
datum_h3_latlng_to_cell(Datum point_d, Datum res_d)
{
  const GSERIALIZED *point = (GSERIALIZED *) DatumGetPointer(point_d);
  return H3IndexGetDatum(h3_gs_point_to_cell(point,
    DatumGetInt32(res_d)));
}

Datum
datum_h3_cell_to_latlng(Datum d)
{
  GSERIALIZED *gs = h3_cell_to_gs_point(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

Datum
datum_h3_cell_to_boundary(Datum d)
{
  GSERIALIZED *gs = h3_cell_to_gs_boundary(DatumGetH3Index(d));
  return PointerGetDatum(gs);
}

/*****************************************************************************
 * Datum wrappers for metrics
 *
 * These take the unit as an auxiliary parameter (H3Unit enum stored
 * in an Int32 datum — no temporal unit support).
 *****************************************************************************/

Datum
datum_h3_cell_area(Datum cell_d, Datum unit_d)
{
  return Float8GetDatum(h3_cell_area_meos(DatumGetH3Index(cell_d),
    (H3Unit) DatumGetInt32(unit_d)));
}

Datum
datum_h3_edge_length(Datum edge_d, Datum unit_d)
{
  return Float8GetDatum(h3_edge_length_meos(DatumGetH3Index(edge_d),
    (H3Unit) DatumGetInt32(unit_d)));
}

Datum
datum_h3_great_circle_distance(Datum a_d, Datum b_d, Datum unit_d)
{
  const GSERIALIZED *a = (GSERIALIZED *) DatumGetPointer(a_d);
  const GSERIALIZED *b = (GSERIALIZED *) DatumGetPointer(b_d);
  return Float8GetDatum(h3_gs_great_circle_distance_meos(a, b,
    (H3Unit) DatumGetInt32(unit_d)));
}

/*****************************************************************************/
