/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * `th3index_*.c` topic files
 * @details Contains three groups:
 *
 * 1. Datum-calling-convention wrappers (`datum_h3_*`) plugged into
 * `LiftedFunctionInfo.func` by the lifted temporal functions;
 * bodies live in `meos/src/h3/h3index.c` (cbuffer convention —
 * the base-type file owns all Datum wrappers).
 * 2. Hand-written static h3 adapters (GSERIALIZED conversions,
 * next-resolution conveniences, unit-string dispatch) that
 * live alongside their primary consumer in the matching
 * `th3index_<topic>.c` file.
 * 3. The `H3Unit` enum shared between `th3index_metrics.c` (where
 * it is produced from the user-supplied string) and
 * `h3index.c` (where the `datum_h3_*` metric wrappers consume
 * it).
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
#include <meos_h3.h>  /* public h3 geometry-conversion declarations */
#include "temporal/tcellindex.h"  /* DggsOutline, DggsEdgeCross */

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

/* Shared helper: build an SRID 4326 polygon, geodetic or planar, from a
 * libh3 CellBoundary and serialise. Primary definition in
 * th3index_latlng.c; also called from th3index_edges.c and h3index.c. */
extern GSERIALIZED *cell_boundary_to_gs(const CellBoundary *bnd,
  bool geodetic);

/* Geodetic centroid and boundary of a cell, for the shared cell operations
 * whose temporal results are geodetic. Bodies in th3index_latlng.c. */
extern GSERIALIZED *h3index_cell_to_geogpoint(H3Index cell);
extern GSERIALIZED *h3index_cell_to_geog(H3Index cell);

/* Cell traversal helpers shared between the static-geo cover
 * (h3_geo.c::geo_to_h3index_set) and the temporal densifier
 * (th3index_latlng.c::tpointseq_densify_to_th3index); cell lookup is a thin
 * latLngToCell wrapper that returns 0 on libh3 error. */
extern double h3_segment_length_deg(double lon1, double lat1, double lon2,
  double lat2, bool geodetic);
/* The crossings of the walk fill a DggsEdgeCross for a planar segment. */
extern int h3_segment_cells(double lon1, double lat1, double lon2,
  double lat2, bool geodetic, int32 resolution, H3Index *cells, double *enter,
  DggsEdgeCross *cross,
  int maxout);
extern H3Index h3_latlng_deg_to_cell(double lat_deg, double lng_deg,
  int32 resolution);

/* The canonical outline of a cell: the vertices of the vertex index in
 * degrees, with the extra vertex of an edge crossing a face of the
 * icosahedron read from the directed edge leaving the lower of its two
 * cells, so that two neighbours state every vertex they share with the same
 * doubles. Body in h3_geo.c. */
extern bool h3_cell_outline(H3Index cell, DggsOutline *out);
/* The owner of a boundary point: whether a cell holds an edge or a vertex of
 * its outline, by the exact signs of the rule of the cell walks (it holds its
 * north and west sides). Bodies in h3_geo.c. */
extern bool h3_outline_edge_held(const DggsOutline *o, int k);
extern bool h3_outline_edge_held_geodetic(const DggsOutline *o, int k);
extern bool h3_outline_vertex_held_geodetic(const DggsOutline *o, int k);
extern bool h3_outline_vertex_held(const DggsOutline *o, int k);
/* The microsecond at which a segment crosses the line of an edge of an
 * outline, truncated, exactly. Body in h3_geo.c. */
extern int64 h3_geodetic_edge_offset(double x0, double y0, double x1,
  double y1, int64 D, double ax, double ay, double bx, double by, double f,
  bool *whole);
/* Whether a cell holds a position of the plane of longitude and latitude, by
 * the exact signs of its outline and the owner rule, and the planar cell of
 * a position. Bodies in h3_geo.c. */
extern bool h3_outline_holds_point(const DggsOutline *o, double x, double y);
/* Whether a geodetic cell shares a position with a geodetic box, the cell
 * read with the boundary points it owns. Body in h3_geo.c. */
extern bool h3_cell_meets_box(H3Index cell, const STBox *box,
  bool border_inc);
extern H3Index h3_point_cell(double lon, double lat, bool geodetic,
  int32 resolution);
/* The cells around the poles at a resolution. Body in th3index_boxops.c. */
extern void h3index_pole_cells(int32_t res, H3Index *north, H3Index *south);

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

/*****************************************************************************
 * Split of a temporal point by the cells it crosses (body in
 * th3index_latlng.c, typed by tgeompoint_h3index_split and
 * tgeogpoint_h3index_split in the same file)
 *****************************************************************************/

extern Temporal **tpoint_h3index_split(const Temporal *temp, int32 resolution,
  bool border_inc, Datum **cells, int *count);

#endif /* __TH3INDEX_INTERNAL_H__ */
