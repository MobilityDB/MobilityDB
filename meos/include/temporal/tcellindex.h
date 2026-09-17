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
 * @brief Shared scaffolding for the temporal Discrete-Global-Grid-System
 * (DGGS) cell-index family: th3index, tquadbin, and any future ts2cell.
 *
 * All three are the same shape: a uint64 cell id sampled over time with STEP
 * (never-interpolated) semantics — a `tbigint`-shaped temporal type. The
 * temporal machinery (I/O, constructors, accessors, comparison, lifting) is
 * therefore IDENTICAL across them; only the static-cell kernel differs.
 *
 * This file factors that shared machinery once. Each DGGS supplies a single
 * `DggsCellOps` descriptor — a table of Datum-convention static-cell function
 * pointers plus its catalog identity — and the generic temporal entry points
 * below lift those pointers with `tfunc_temporal`. The descriptor hides HOW a
 * DGGS sources its kernel:
 *
 *   - th3index : linked C library (Uber libh3)
 *   - tquadbin : first-party from the CARTO bit-spec (libm only, no vendoring)
 *   - ts2cell  : first-party S2CellId subset (libm only) — see note below
 *
 * ---------------------------------------------------------------------------
 * ADDING A NEW DGGS (worked example: Google S2)
 * ---------------------------------------------------------------------------
 *   1. Catalog: add T_S2CELL / T_S2CELLSET / T_TS2CELL to MeosType, and the
 *      T_TS2CELL case to `tcellindex_type()`.
 *   2. Static kernel: implement the S2CellId algebra (uint64 cell math). S2 has
 *      no maintained C API and its full C++ library is a heavy Abseil-dependent
 *      vendored blob; the recommended path mirrors quadbin — first-party the
 *      cell-id subset (cube-face projection + Hilbert curve), libm only.
 *   3. Descriptor: define a `DggsCellOps s2_cellops` with `datum_s2_*` wrappers,
 *      its point_temptype (T_TGEOGPOINT for S2's geodetic cells) and SRID.
 *   4. Register: add the `#if S2CELL case T_TS2CELL: return &s2_cellops;` line
 *      to `dggs_cellops()`, and the matching `option(S2CELL ...)` beside
 *      `option(H3 ...)`. The family option carries the STATIC TYPE's name, as
 *      H3 carries h3index and QUADBIN carries quadbin: the grid's bare name
 *      cannot serve here, because `S2` is a stab-line endpoint in the vendored
 *      PostGIS geodetic code (liblwgeom/lwgeodetic.c, lwgeodetic_tree.c) that a
 *      build-wide `-DS2=0|1` rewrites into a numeric constant.
 * No new temporal scaffolding, SQL boilerplate, or binding code is required:
 * the generic entry points below already cover the new type.
 */

#ifndef __TCELLINDEX_H__
#define __TCELLINDEX_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
/* The public generic accessors (tcellindex_get_resolution, …) are declared in
 * the umbrella header so bindings pick them up from it. */
#include <meos_cellindex.h>
#include "temporal/meos_catalog.h"

/*****************************************************************************
 * Per-DGGS descriptor — the ONLY thing a DGGS family must provide to plug
 * into the shared temporal cell-index machinery.
 *****************************************************************************/

/**
 * @brief Table of static-cell operations and catalog identity for one DGGS.
 *
 * The function pointers use the Datum calling convention so the generic
 * temporal entry points can pass them straight to `LiftedFunctionInfo.func`.
 * A NULL pointer means the DGGS does not offer that operation; the generic
 * wrapper then raises a not-implemented error rather than dereferencing it.
 */
typedef struct DggsCellOps
{
  /* Catalog identity */
  MeosType celltype;        /**< static cell type, e.g. T_QUADBIN */
  MeosType settype;         /**< cell set type, e.g. T_QUADBINSET */
  MeosType temptype;        /**< temporal type, e.g. T_TQUADBIN */
  int32    min_resolution;  /**< coarsest valid resolution */
  int32    max_resolution;  /**< finest valid resolution */

  /* Geometry result identity (H3 cells are geodetic -> tgeogpoint/4326;
   * quadbin cells are Web-Mercator -> tgeompoint/3857). */
  MeosType point_temptype;  /**< T_TGEOGPOINT or T_TGEOMPOINT */
  int32    point_srid;      /**< 4326 (H3/S2) or 3857 (quadbin) */

  /* Static-cell kernel, Datum convention. Bodies live in the DGGS family. */
  Datum (*get_resolution)(Datum cell);            /**< -> Int32  */
  Datum (*is_valid_cell)(Datum cell);             /**< -> Bool   */
  Datum (*cell_to_parent)(Datum cell, Datum res); /**< -> cell   */
  Datum (*cell_to_point)(Datum cell);             /**< -> GSERIALIZED point */
  Datum (*cell_to_boundary)(Datum cell);          /**< -> GSERIALIZED polygon */
  Datum (*cell_area)(Datum cell);                 /**< -> Float8 (m^2) */
} DggsCellOps;

/*****************************************************************************
 * Catalog predicate + descriptor registry
 *****************************************************************************/

/**
 * @brief Return true if @p type is a temporal DGGS cell-index type
 * (T_TH3INDEX, T_TQUADBIN, or a future T_TS2CELL).
 */
extern bool tcellindex_type(MeosType type);

/**
 * @brief Ensure that @p type is a temporal DGGS cell-index type, or raise an
 * error.
 */
extern bool ensure_tcellindex_type(MeosType type);

/**
 * @brief Return the operations descriptor for a temporal cell-index type, or
 * raise an error if @p temptype is not a (compiled-in) DGGS type.
 */
extern const DggsCellOps *dggs_cellops(MeosType temptype);

/**
 * @brief Ensure that a resolution lies in the range of the grid of a temporal
 * cell-index type, or raise an error.
 */
extern bool ensure_valid_cell_resolution(MeosType temptype, int32 resolution);

/**
 * @brief Ensure that a 64-bit integer encodes a cell of the grid of a temporal
 * cell-index type, or raise an error.
 */
extern bool ensure_valid_cell(Datum value, MeosType temptype);

/**
 * @brief Ensure that every value of a temporal 64-bit integer encodes a cell
 * of the grid of a temporal cell-index type, or raise an error.
 */
extern bool ensure_valid_tcell(const Temporal *temp, MeosType temptype);

/*****************************************************************************
 * Generic temporal entry points — shared by every DGGS.
 *
 * Each dispatches on `temp->temptype` via `dggs_cellops()` and lifts the
 * descriptor's static kernel over the temporal value. SQL/bindings expose
 * these once and overload on the concrete temporal type.
 *****************************************************************************/

/* Declared in the umbrella header <meos_cellindex.h> (included above). */

/*****************************************************************************
 * Geodetic bounding box of a cell boundary — shared by every DGGS whose
 * cells are defined on the sphere.
 *****************************************************************************/

extern void dggs_lonlat_boundary_set_box(const double *lons,
  const double *lats, int count, bool north_pole, bool south_pole,
  double *xmin, double *ymin, double *xmax, double *ymax);

/*****************************************************************************
 * Geodetic path of a segment — shared by every DGGS whose cells are defined
 * on the sphere.
 *****************************************************************************/

/**
 * @brief Great-circle path of a geodetic segment, the one a temporal geodetic
 * point moves along between two instants as `pointsegm_interpolate` places it
 */
typedef struct
{
  double lon;             /**< Longitude of the first endpoint, in radians */
  double lat;             /**< Latitude of the first endpoint, in radians */
  double dist;            /**< Angle of the path, in radians */
  double azimuth;         /**< Azimuth of the path at its first endpoint */
  double a[3];            /**< First endpoint, a unit vector */
  double normal[3];       /**< Unit normal of the circle of the path */
} DggsArc;

/**
 * @brief Straight path in longitude and latitude of a planar segment, the one
 * a temporal planar point moves along between two instants
 */
typedef struct
{
  double lon;             /**< Longitude of the first endpoint, in radians */
  double lat;             /**< Latitude of the first endpoint, in radians */
  double dlon;            /**< Step in longitude to the second endpoint */
  double dlat;            /**< Step in latitude to the second endpoint */
  double length;          /**< Length of the path in the plane, in radians */
  double curvature;       /**< Bound on the second derivative of the path */
} DggsLine;

/*****************************************************************************
 * Membership of a temporal cell index in a cell set — shared by every DGGS.
 *
 * The walk reads the values of the temporal value and the set alone, so one
 * statement of it answers for every grid; each family's `ever_eq` entry point
 * validates its own two types and calls it.
 *****************************************************************************/

extern bool tcellindex_ever_in_set(const Temporal *temp, const Set *cells);

/*****************************************************************************
 * Compaction of a set of cells of a quadtree grid — shared by every DGGS whose
 * cell is exactly the union of its four children.
 *****************************************************************************/

extern Set *dggs_quadtree_compact_cells(const Set *cells, MeosType temptype);
extern Set *dggs_quadtree_uncompact_cells(const Set *cells, int32 resolution,
  MeosType temptype, uint64 *(*children)(uint64, uint32_t, int *));

extern bool dggs_arc_init(double lon1, double lat1, double lon2, double lat2,
  DggsArc *arc);
extern bool dggs_arc_point(const DggsArc *arc, double t, double *lon,
  double *lat);
extern double dggs_arc_exit_param(const DggsArc *arc, const double *lons,
  const double *lats, int count, double tmin, bool convex);
extern bool dggs_line_init(double lon1, double lat1, double lon2,
  double lat2, DggsLine *line);
extern void dggs_line_point(const DggsLine *line, double t, double *lon,
  double *lat);
extern double dggs_line_exit_param(const DggsLine *line, const double *lons,
  const double *lats, int count, double tmin, bool convex);

#endif /* __TCELLINDEX_H__ */
