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
 * @brief PG wrappers for the TPCBox bounding-box type.
 *
 * Fixed-size struct (no varlena), so @c recv / @c send simply shuttle the
 * bytes through. @c in / @c out delegate to MEOS-layer @c tpcbox_in /
 * @c tpcbox_out, which accept the hex byte-image form.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <utils/timestamp.h>
/* pgpointcloud */
#include "pc_api.h"
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/span.h"  /* PG_GETARG_SPAN_P */
#include "pointcloud/tpcbox.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

/*****************************************************************************
 * Input / output
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_in);
/**
 * @ingroup mobilitydb_pointcloud_box_inout
 * @brief Return a TPCBox from its hex-encoded text representation
 * @sqlfn tpcbox_in()
 */
Datum
Tpcbox_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_TPCBOX_P(tpcbox_in(str));
}

PGDLLEXPORT Datum Tpcbox_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_out);
/**
 * @ingroup mobilitydb_pointcloud_box_inout
 * @brief Return the hex-encoded text representation of a TPCBox
 * @sqlfn tpcbox_out()
 */
Datum
Tpcbox_out(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  PG_RETURN_CSTRING(tpcbox_out(box, 15));
}

PGDLLEXPORT Datum Tpcbox_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_recv);
/**
 * @ingroup mobilitydb_pointcloud_box_inout
 * @brief Binary recv: read the raw TPCBox struct image
 * @sqlfn tpcbox_recv()
 */
Datum
Tpcbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TPCBox *result = palloc(sizeof(TPCBox));
  pq_copymsgbytes(buf, (char *) result, sizeof(TPCBox));
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Tpcbox_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_send);
/**
 * @ingroup mobilitydb_pointcloud_box_inout
 * @brief Binary send: emit the raw TPCBox struct image
 * @sqlfn tpcbox_send()
 */
Datum
Tpcbox_send(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (const char *) box, sizeof(TPCBox));
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_constructor_2d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_2d);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief 2D constructor — tpcbox(xmin, ymin, xmax, ymax, pcid, srid)
 * @sqlfn tpcbox()
 */
Datum
Tpcbox_constructor_2d(PG_FUNCTION_ARGS)
{
  double xmin = PG_GETARG_FLOAT8(0);
  double ymin = PG_GETARG_FLOAT8(1);
  double xmax = PG_GETARG_FLOAT8(2);
  double ymax = PG_GETARG_FLOAT8(3);
  int32 pcid = PG_GETARG_INT32(4);
  int32 srid = PG_GETARG_INT32(5);
  PG_RETURN_TPCBOX_P(tpcbox_make(true, false, false, false,
    srid, (uint32_t) pcid, xmin, xmax, ymin, ymax, 0.0, 0.0, NULL));
}

PGDLLEXPORT Datum Tpcbox_constructor_3d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_3d);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief 3D constructor — tpcbox_z(xmin, ymin, zmin, xmax, ymax, zmax, pcid, srid)
 * @sqlfn tpcbox_z()
 */
Datum
Tpcbox_constructor_3d(PG_FUNCTION_ARGS)
{
  double xmin = PG_GETARG_FLOAT8(0);
  double ymin = PG_GETARG_FLOAT8(1);
  double zmin = PG_GETARG_FLOAT8(2);
  double xmax = PG_GETARG_FLOAT8(3);
  double ymax = PG_GETARG_FLOAT8(4);
  double zmax = PG_GETARG_FLOAT8(5);
  int32 pcid = PG_GETARG_INT32(6);
  int32 srid = PG_GETARG_INT32(7);
  PG_RETURN_TPCBOX_P(tpcbox_make(true, true, false, false,
    srid, (uint32_t) pcid, xmin, xmax, ymin, ymax, zmin, zmax, NULL));
}

PGDLLEXPORT Datum Tpcbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_t);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief Time-only constructor — tpcbox_t(period, pcid)
 * @sqlfn tpcbox_t()
 */
Datum
Tpcbox_constructor_t(PG_FUNCTION_ARGS)
{
  Span *period = PG_GETARG_SPAN_P(0);
  int32 pcid = PG_GETARG_INT32(1);
  PG_RETURN_TPCBOX_P(tpcbox_make(false, false, true, false,
    0, (uint32_t) pcid, 0, 0, 0, 0, 0, 0, period));
}

PGDLLEXPORT Datum Tpcbox_constructor_xt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_xt);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief XY+T constructor — tpcbox_xt(xmin, ymin, xmax, ymax, period, pcid, srid)
 * @sqlfn tpcbox_xt()
 */
Datum
Tpcbox_constructor_xt(PG_FUNCTION_ARGS)
{
  double xmin = PG_GETARG_FLOAT8(0);
  double ymin = PG_GETARG_FLOAT8(1);
  double xmax = PG_GETARG_FLOAT8(2);
  double ymax = PG_GETARG_FLOAT8(3);
  Span *period = PG_GETARG_SPAN_P(4);
  int32 pcid = PG_GETARG_INT32(5);
  int32 srid = PG_GETARG_INT32(6);
  PG_RETURN_TPCBOX_P(tpcbox_make(true, false, true, false,
    srid, (uint32_t) pcid, xmin, xmax, ymin, ymax, 0.0, 0.0, period));
}

PGDLLEXPORT Datum Tpcbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_zt);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief XYZ+T constructor — tpcbox_zt(xmin, ymin, zmin, xmax, ymax, zmax, period, pcid, srid)
 * @sqlfn tpcbox_zt()
 */
Datum
Tpcbox_constructor_zt(PG_FUNCTION_ARGS)
{
  double xmin = PG_GETARG_FLOAT8(0);
  double ymin = PG_GETARG_FLOAT8(1);
  double zmin = PG_GETARG_FLOAT8(2);
  double xmax = PG_GETARG_FLOAT8(3);
  double ymax = PG_GETARG_FLOAT8(4);
  double zmax = PG_GETARG_FLOAT8(5);
  Span *period = PG_GETARG_SPAN_P(6);
  int32 pcid = PG_GETARG_INT32(7);
  int32 srid = PG_GETARG_INT32(8);
  PG_RETURN_TPCBOX_P(tpcbox_make(true, true, true, false,
    srid, (uint32_t) pcid, xmin, xmax, ymin, ymax, zmin, zmax, period));
}

/*****************************************************************************
 * Conversion — pcpatch → tpcbox
 *****************************************************************************/

PGDLLEXPORT Datum Pcpatch_to_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_to_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief Return a TPCBox built from a pcpatch — auto-fills SRID from the
 *   pgpointcloud schema via the per-backend schema cache
 * @sqlfn tpcbox()
 */
Datum
Pcpatch_to_tpcbox(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  PCSCHEMA *schema = mobilitydb_pc_schema(pa->pcid);
  TPCBox *result = pcpatch_to_tpcbox(pa, (int32_t) schema->srid);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Pcpatch_to_tpcbox_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpatch_to_tpcbox_srid);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief Return a TPCBox built from a pcpatch with an explicit SRID override
 * @details Skips the schema lookup. Useful when pointcloud_formats.srid = 0
 *   and the caller knows the real SRID from a PostGIS context.
 * @sqlfn tpcbox()
 */
Datum
Pcpatch_to_tpcbox_srid(PG_FUNCTION_ARGS)
{
  Pcpatch *pa = PG_GETARG_PCPATCH_P(0);
  int32 srid = PG_GETARG_INT32(1);
  TPCBox *result = pcpatch_to_tpcbox(pa, srid);
  PG_FREE_IF_COPY(pa, 0);
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Pcpoint_to_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pcpoint_to_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_constructor
 * @brief Return a degenerate (single-point) TPCBox built from a pcpoint
 * @details Spatial bounds equal the point's X/Y/[Z]. SRID comes from the
 *   schema; pcid from the pcpoint itself. The per-backend schema cache
 *   makes this possible without dragging pgpointcloud's PG-layer API
 *   into MEOS.
 * @sqlfn tpcbox()
 */
Datum
Pcpoint_to_tpcbox(PG_FUNCTION_ARGS)
{
  Pcpoint *pt = PG_GETARG_PCPOINT_P(0);
  TPCBox *result = pcpoint_to_tpcbox(pt, meos_pc_schema(pt->pcid));
  PG_FREE_IF_COPY(pt, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

/* Helper: each Tpcbox_<dim>min/max wrapper just unpacks the TPCBox arg,
 * calls the corresponding MEOS getter (out-param style, returns false
 * when the dimension is absent), and translates false → NULL. */
#define TPCBOX_ACCESSOR_DOUBLE_BODY(meos_fn) \
  TPCBox *box = PG_GETARG_TPCBOX_P(0); \
  double v; \
  if (! meos_fn(box, &v)) PG_RETURN_NULL(); \
  PG_RETURN_FLOAT8(v);

PGDLLEXPORT Datum Tpcbox_xmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_xmin);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the minimum X of a TPCBox; NULL if no XY dimensions
 * @sqlfn xmin()
 */
Datum
Tpcbox_xmin(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_xmin) }

PGDLLEXPORT Datum Tpcbox_xmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_xmax);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the maximum X of a TPCBox; NULL if no XY dimensions
 * @sqlfn xmax()
 */
Datum
Tpcbox_xmax(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_xmax) }

PGDLLEXPORT Datum Tpcbox_ymin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_ymin);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the minimum Y of a TPCBox; NULL if no XY dimensions
 * @sqlfn ymin()
 */
Datum
Tpcbox_ymin(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_ymin) }

PGDLLEXPORT Datum Tpcbox_ymax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_ymax);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the maximum Y of a TPCBox; NULL if no XY dimensions
 * @sqlfn ymax()
 */
Datum
Tpcbox_ymax(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_ymax) }

PGDLLEXPORT Datum Tpcbox_zmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_zmin);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the minimum Z of a TPCBox; NULL if no Z dimension
 * @sqlfn zmin()
 */
Datum
Tpcbox_zmin(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_zmin) }

PGDLLEXPORT Datum Tpcbox_zmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_zmax);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the maximum Z of a TPCBox; NULL if no Z dimension
 * @sqlfn zmax()
 */
Datum
Tpcbox_zmax(PG_FUNCTION_ARGS)
{ TPCBOX_ACCESSOR_DOUBLE_BODY(tpcbox_zmax) }

PGDLLEXPORT Datum Tpcbox_tmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_tmin);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the minimum timestamp of a TPCBox; NULL if no T dimension
 * @sqlfn tmin()
 */
Datum
Tpcbox_tmin(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  TimestampTz v;
  if (! tpcbox_tmin(box, &v)) PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(v);
}

PGDLLEXPORT Datum Tpcbox_tmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_tmax);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the maximum timestamp of a TPCBox; NULL if no T dimension
 * @sqlfn tmax()
 */
Datum
Tpcbox_tmax(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  TimestampTz v;
  if (! tpcbox_tmax(box, &v)) PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(v);
}

PGDLLEXPORT Datum Tpcbox_hasx(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_hasx);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return true if a TPCBox has the XY dimensions set
 * @sqlfn hasX()
 */
Datum Tpcbox_hasx(PG_FUNCTION_ARGS)
{ PG_RETURN_BOOL(tpcbox_hasx(PG_GETARG_TPCBOX_P(0))); }

PGDLLEXPORT Datum Tpcbox_hasz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_hasz);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return true if a TPCBox has the Z dimension set
 * @sqlfn hasZ()
 */
Datum Tpcbox_hasz(PG_FUNCTION_ARGS)
{ PG_RETURN_BOOL(tpcbox_hasz(PG_GETARG_TPCBOX_P(0))); }

PGDLLEXPORT Datum Tpcbox_hast(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_hast);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return true if a TPCBox has the T (time) dimension set
 * @sqlfn hasT()
 */
Datum Tpcbox_hast(PG_FUNCTION_ARGS)
{ PG_RETURN_BOOL(tpcbox_hast(PG_GETARG_TPCBOX_P(0))); }

PGDLLEXPORT Datum Tpcbox_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_srid);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the SRID of a TPCBox
 * @sqlfn SRID()
 */
Datum Tpcbox_srid(PG_FUNCTION_ARGS)
{ PG_RETURN_INT32(tpcbox_srid(PG_GETARG_TPCBOX_P(0))); }

PGDLLEXPORT Datum Tpcbox_pcid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_pcid);
/**
 * @ingroup mobilitydb_pointcloud_box_accessor
 * @brief Return the pgPointCloud schema id of a TPCBox
 * @sqlfn pcid()
 */
Datum Tpcbox_pcid(PG_FUNCTION_ARGS)
{ PG_RETURN_INT32((int32) tpcbox_pcid(PG_GETARG_TPCBOX_P(0))); }

PGDLLEXPORT Datum Tpcbox_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_to_stbox);
/**
 * @ingroup mobilitydb_pointcloud_box_conversion
 * @brief Project a TPCBox to a STBox by dropping the pcid
 * @sqlfn stbox()
 */
Datum Tpcbox_to_stbox(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  STBox *result = tpcbox_to_stbox(box);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformations
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_round);
/**
 * @ingroup mobilitydb_pointcloud_box
 * @brief Return a TPCBox with coordinates rounded to a given number
 *   of decimal digits.
 * @sqlfn round()
 */
Datum
Tpcbox_round(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  int maxdd = PG_GETARG_INT32(1);
  TPCBox *result = tpcbox_round(box, maxdd);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Tpcbox_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_set_srid);
/**
 * @ingroup mobilitydb_pointcloud_box
 * @brief Return a TPCBox with the SRID overwritten (no coordinate
 *   reprojection).
 * @sqlfn setSRID()
 */
Datum
Tpcbox_set_srid(PG_FUNCTION_ARGS)
{
  TPCBox *box = PG_GETARG_TPCBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  TPCBox *result = tpcbox_set_srid(box, srid);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * Set operations
 *****************************************************************************/

PGDLLEXPORT Datum Union_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_setops
 * @brief Return the union of two TPCBox values (must share pcid)
 * @sqlfn tpcbox_union()
 * @sqlop @p +
 */
Datum
Union_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{
  TPCBox *a = PG_GETARG_TPCBOX_P(0);
  TPCBox *b = PG_GETARG_TPCBOX_P(1);
  TPCBox *result = union_tpcbox_tpcbox(a, b, /* strict */ false);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

PGDLLEXPORT Datum Intersection_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_setops
 * @brief Return the intersection of two TPCBox values; NULL if disjoint
 *   or pcid mismatch
 * @sqlfn tpcbox_intersection()
 * @sqlop @p *
 */
Datum
Intersection_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{
  TPCBox *a = PG_GETARG_TPCBOX_P(0);
  TPCBox *b = PG_GETARG_TPCBOX_P(1);
  TPCBox *result = intersection_tpcbox_tpcbox(a, b);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TPCBOX_P(result);
}

/*****************************************************************************
 * Topological predicates
 *****************************************************************************/

/* Each TPCBOX_PRED_2 wrapper unpacks two TPCBox args, calls the MEOS
 * predicate, returns the boolean.  Explicit per-function definitions
 * (rather than a single TPCBOX_PRED_2(name, meos_fn) macro invocation)
 * so each gets its own Doxygen @ingroup tag. */
#define TPCBOX_PRED_2_BODY(meos_fn) \
  TPCBox *a = PG_GETARG_TPCBOX_P(0); \
  TPCBox *b = PG_GETARG_TPCBOX_P(1); \
  PG_RETURN_BOOL(meos_fn(a, b));

#define TPCBOX_PRED_2_DEFN(fn_name, meos_fn, group, sql_name, sql_op) \
  PGDLLEXPORT Datum fn_name(PG_FUNCTION_ARGS); \
  PG_FUNCTION_INFO_V1(fn_name); \
  /** \
   * @ingroup group \
   * @brief PG wrapper for meos_fn \
   * @sqlfn sql_name \
   * @sqlop @p sql_op \
   */ \
  Datum fn_name(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(meos_fn) }

PGDLLEXPORT Datum Contains_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_topo
 * @brief PG wrapper: tpcbox @> tpcbox
 * @sqlfn tpcbox_contains()
 * @sqlop @p \@>
 */
Datum Contains_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(contains_tpcbox_tpcbox) }

PGDLLEXPORT Datum Contained_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_topo
 * @brief PG wrapper: tpcbox <@ tpcbox
 * @sqlfn tpcbox_contained()
 * @sqlop @p <@
 */
Datum Contained_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(contained_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overlaps_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_topo
 * @brief PG wrapper: tpcbox && tpcbox
 * @sqlfn tpcbox_overlaps()
 * @sqlop @p &&
 */
Datum Overlaps_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overlaps_tpcbox_tpcbox) }

PGDLLEXPORT Datum Same_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_topo
 * @brief PG wrapper: tpcbox ~= tpcbox
 * @sqlfn tpcbox_same()
 * @sqlop @p ~=
 */
Datum Same_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(same_tpcbox_tpcbox) }

PGDLLEXPORT Datum Adjacent_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_topo
 * @brief PG wrapper: tpcbox -|- tpcbox
 * @sqlfn tpcbox_adjacent()
 * @sqlop @p -|-
 */
Datum Adjacent_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(adjacent_tpcbox_tpcbox) }

/* Position predicates */

PGDLLEXPORT Datum Left_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox << tpcbox (strictly left, X-axis)
 * @sqlop @p <<
 */
Datum Left_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(left_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overleft_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox &< tpcbox (does not extend right, X-axis)
 * @sqlop @p &<
 */
Datum Overleft_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overleft_tpcbox_tpcbox) }

PGDLLEXPORT Datum Right_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox >> tpcbox (strictly right, X-axis)
 * @sqlop @p >>
 */
Datum Right_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(right_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overright_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox &> tpcbox (does not extend left, X-axis)
 * @sqlop @p &>
 */
Datum Overright_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overright_tpcbox_tpcbox) }

PGDLLEXPORT Datum Below_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox <<| tpcbox (strictly below, Y-axis)
 * @sqlop @p <<|
 */
Datum Below_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(below_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overbelow_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox &<| tpcbox (does not extend above, Y-axis)
 * @sqlop @p &<|
 */
Datum Overbelow_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overbelow_tpcbox_tpcbox) }

PGDLLEXPORT Datum Above_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox |>> tpcbox (strictly above, Y-axis)
 * @sqlop @p |>>
 */
Datum Above_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(above_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overabove_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox |&> tpcbox (does not extend below, Y-axis)
 * @sqlop @p |&>
 */
Datum Overabove_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overabove_tpcbox_tpcbox) }

PGDLLEXPORT Datum Front_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox <</ tpcbox (strictly in front, Z-axis)
 * @sqlop @p <</
 */
Datum Front_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(front_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overfront_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox &</ tpcbox (does not extend behind, Z-axis)
 * @sqlop @p &</
 */
Datum Overfront_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overfront_tpcbox_tpcbox) }

PGDLLEXPORT Datum Back_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox />> tpcbox (strictly behind, Z-axis)
 * @sqlop @p />>
 */
Datum Back_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(back_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overback_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox /&> tpcbox (does not extend in front, Z-axis)
 * @sqlop @p /&>
 */
Datum Overback_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overback_tpcbox_tpcbox) }

PGDLLEXPORT Datum Before_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox <<# tpcbox (strictly before, time)
 * @sqlop @p <<#
 */
Datum Before_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(before_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overbefore_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox &<# tpcbox (does not extend after, time)
 * @sqlop @p &<#
 */
Datum Overbefore_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overbefore_tpcbox_tpcbox) }

PGDLLEXPORT Datum After_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox #>> tpcbox (strictly after, time)
 * @sqlop @p #>>
 */
Datum After_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(after_tpcbox_tpcbox) }

PGDLLEXPORT Datum Overafter_tpcbox_tpcbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_tpcbox_tpcbox);
/**
 * @ingroup mobilitydb_pointcloud_box_pos
 * @brief PG wrapper: tpcbox #&> tpcbox (does not extend before, time)
 * @sqlop @p #&>
 */
Datum Overafter_tpcbox_tpcbox(PG_FUNCTION_ARGS)
{ TPCBOX_PRED_2_BODY(overafter_tpcbox_tpcbox) }

/*****************************************************************************
 * Comparison
 *****************************************************************************/

PGDLLEXPORT Datum Tpcbox_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_eq);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox = tpcbox
 * @sqlfn tpcbox_eq()
 * @sqlop @p =
 */
Datum Tpcbox_eq(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_eq) }

PGDLLEXPORT Datum Tpcbox_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_ne);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox <> tpcbox
 * @sqlfn tpcbox_ne()
 * @sqlop @p <>
 */
Datum Tpcbox_ne(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_ne) }

PGDLLEXPORT Datum Tpcbox_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_lt);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox < tpcbox
 * @sqlfn tpcbox_lt()
 * @sqlop @p <
 */
Datum Tpcbox_lt(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_lt) }

PGDLLEXPORT Datum Tpcbox_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_le);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox <= tpcbox
 * @sqlfn tpcbox_le()
 * @sqlop @p <=
 */
Datum Tpcbox_le(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_le) }

PGDLLEXPORT Datum Tpcbox_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_gt);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox > tpcbox
 * @sqlfn tpcbox_gt()
 * @sqlop @p >
 */
Datum Tpcbox_gt(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_gt) }

PGDLLEXPORT Datum Tpcbox_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_ge);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief PG wrapper: tpcbox >= tpcbox
 * @sqlfn tpcbox_ge()
 * @sqlop @p >=
 */
Datum Tpcbox_ge(PG_FUNCTION_ARGS) { TPCBOX_PRED_2_BODY(tpcbox_ge) }

PGDLLEXPORT Datum Tpcbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_cmp);
/**
 * @ingroup mobilitydb_pointcloud_box_comp
 * @brief B-tree comparator for tpcbox
 * @sqlfn tpcbox_cmp()
 */
Datum
Tpcbox_cmp(PG_FUNCTION_ARGS)
{
  TPCBox *a = PG_GETARG_TPCBOX_P(0);
  TPCBox *b = PG_GETARG_TPCBOX_P(1);
  PG_RETURN_INT32(tpcbox_cmp(a, b));
}

/*****************************************************************************/
