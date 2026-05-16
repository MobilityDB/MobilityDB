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
 * @brief Internal declarations shared across the `h3index.c` and the
 * `th3index_*.c` topic files.
 *
 * Contains three groups:
 *
 *   1. Datum-calling-convention wrappers (`datum_h3_*`) plugged into
 *      `LiftedFunctionInfo.func` by the lifted temporal functions;
 *      bodies live in `meos/src/h3/h3index.c` (cbuffer convention —
 *      the base-type file owns all Datum wrappers).
 *   2. Hand-written static h3 adapters (GSERIALIZED conversions,
 *      next-resolution conveniences, unit-string dispatch) that
 *      live alongside their primary consumer in the matching
 *      `th3index_<topic>.c` file.
 *   3. The `H3Unit` enum shared between `th3index_metrics.c` (where
 *      it is produced from the user-supplied string) and
 *      `h3index.c` (where the `datum_h3_*` metric wrappers consume
 *      it).
 *
 * None of these symbols are part of the public MEOS API.
 */

#ifndef __TH3INDEX_INTERNAL_H__
#define __TH3INDEX_INTERNAL_H__

#include <postgres.h>
#ifndef MEOS
#include <fmgr.h>
#endif

#include <h3api.h>
#include <liblwgeom.h>
#include <meos.h>

/*****************************************************************************
 * Shared metrics enum
 *****************************************************************************/

typedef enum
{
  H3_UNIT_KM,
  H3_UNIT_M,
  H3_UNIT_RADS,
  H3_UNIT_KM2,
  H3_UNIT_M2,
  H3_UNIT_RADS2
} H3Unit;

/*****************************************************************************
 * Hand-written static h3 adapters
 * (those the extractor can't auto-generate — GSERIALIZED conversions,
 *  next-resolution conveniences, and the unit-string dispatcher).
 *****************************************************************************/

/* Point / polygon conversions — bodies in th3index_latlng.c. */
extern H3Index h3_gs_point_to_cell(const GSERIALIZED *point,
  int32 resolution);
extern GSERIALIZED *h3_cell_to_gs_point(H3Index cell);
extern GSERIALIZED *h3_cell_to_gs_boundary(H3Index cell);

/* Shared helper: build a geodetic SRID 4326 LWPOLY from a libh3
 * CellBoundary and serialise. Primary definition in
 * th3index_latlng.c; also called from th3index_edges.c. */
extern GSERIALIZED *cell_boundary_to_gs(const CellBoundary *bnd);

/* Hierarchy — next-resolution conveniences. Bodies in
 * th3index_hierarchy.c. */
extern H3Index h3_cell_to_parent_next_meos(H3Index cell);
extern H3Index h3_cell_to_center_child_next_meos(H3Index cell);

/* Directed edge / vertex geometry — bodies in th3index_edges.c /
 * th3index_vertices.c. */
extern GSERIALIZED *h3_directed_edge_to_gs_boundary(H3Index edge);
extern GSERIALIZED *h3_vertex_to_gs_point(H3Index vertex);

/* Grid traversal local-IJ coordinates — bodies in
 * th3index_traversal.c. */
extern GSERIALIZED *h3_cell_to_local_ij_meos(H3Index origin, H3Index cell);
extern H3Index h3_local_ij_to_cell_meos(H3Index origin,
  const GSERIALIZED *coord);

/* Metrics — bodies in th3index_metrics.c. */
extern H3Unit h3_unit_from_cstring(const char *unit);
extern double h3_cell_area_meos(H3Index cell, H3Unit unit);
extern double h3_edge_length_meos(H3Index edge, H3Unit unit);
extern double h3_gs_great_circle_distance_meos(const GSERIALIZED *a,
  const GSERIALIZED *b, H3Unit unit);

/*****************************************************************************
 * Datum wrappers (bodies in h3index.c)
 *****************************************************************************/

/* inspection */
extern Datum datum_h3_get_resolution(Datum d);
extern Datum datum_h3_get_base_cell_number(Datum d);
extern Datum datum_h3_is_valid_cell(Datum d);
extern Datum datum_h3_is_res_class_iii(Datum d);
extern Datum datum_h3_is_pentagon(Datum d);

/* hierarchy */
extern Datum datum_h3_cell_to_parent(Datum cell_d, Datum res_d);
extern Datum datum_h3_cell_to_parent_next(Datum cell_d);
extern Datum datum_h3_cell_to_center_child(Datum cell_d, Datum res_d);
extern Datum datum_h3_cell_to_center_child_next(Datum cell_d);
extern Datum datum_h3_cell_to_child_pos(Datum cell_d, Datum parent_res_d);
extern Datum datum_h3_child_pos_to_cell(Datum pos_d, Datum parent_d,
  Datum child_res_d);

/* directed edges */
extern Datum datum_h3_are_neighbor_cells(Datum origin_d, Datum dest_d);
extern Datum datum_h3_cells_to_directed_edge(Datum origin_d, Datum dest_d);
extern Datum datum_h3_is_valid_directed_edge(Datum d);
extern Datum datum_h3_get_directed_edge_origin(Datum d);
extern Datum datum_h3_get_directed_edge_destination(Datum d);
extern Datum datum_h3_directed_edge_to_boundary(Datum d);

/* vertices */
extern Datum datum_h3_cell_to_vertex(Datum cell_d, Datum vnum_d);
extern Datum datum_h3_vertex_to_latlng(Datum d);
extern Datum datum_h3_is_valid_vertex(Datum d);

/* grid traversal */
extern Datum datum_h3_grid_distance(Datum origin_d, Datum dest_d);
extern Datum datum_h3_cell_to_local_ij(Datum origin_d, Datum cell_d);
extern Datum datum_h3_local_ij_to_cell(Datum origin_d, Datum coord_d);

/* lat/lng conversions */
extern Datum datum_h3_latlng_to_cell(Datum point_d, Datum res_d);
extern Datum datum_h3_cell_to_latlng(Datum d);
extern Datum datum_h3_cell_to_boundary(Datum d);

/* metrics */
extern Datum datum_h3_cell_area(Datum cell_d, Datum unit_d);
extern Datum datum_h3_edge_length(Datum edge_d, Datum unit_d);
extern Datum datum_h3_great_circle_distance(Datum a_d, Datum b_d,
  Datum unit_d);

#endif /* __TH3INDEX_INTERNAL_H__ */
