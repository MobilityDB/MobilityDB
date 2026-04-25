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

/**
 * @brief SQL: tpcbox(xmin, ymin, xmax, ymax, pcid, srid) — 2D spatial
 */
PGDLLEXPORT Datum Tpcbox_constructor_2d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_2d);
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

/**
 * @brief SQL: tpcbox_z(xmin, ymin, zmin, xmax, ymax, zmax, pcid, srid) — 3D
 */
PGDLLEXPORT Datum Tpcbox_constructor_3d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_3d);
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

/**
 * @brief SQL: tpcbox_t(period, pcid) — time-only
 */
PGDLLEXPORT Datum Tpcbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_t);
Datum
Tpcbox_constructor_t(PG_FUNCTION_ARGS)
{
  Span *period = PG_GETARG_SPAN_P(0);
  int32 pcid = PG_GETARG_INT32(1);
  PG_RETURN_TPCBOX_P(tpcbox_make(false, false, true, false,
    0, (uint32_t) pcid, 0, 0, 0, 0, 0, 0, period));
}

/**
 * @brief SQL: tpcbox_xt(xmin, ymin, xmax, ymax, period, pcid, srid)
 */
PGDLLEXPORT Datum Tpcbox_constructor_xt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_xt);
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

/**
 * @brief SQL: tpcbox_zt(xmin, ymin, zmin, xmax, ymax, zmax, period, pcid, srid)
 */
PGDLLEXPORT Datum Tpcbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_constructor_zt);
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
 * @brief SQL: tpcbox(pcpatch) — auto-fills SRID from the pgpointcloud
 *   schema via the per-backend schema cache.
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
 * @brief SQL: tpcbox(pcpatch, srid) — explicit SRID override, skips the
 *   schema lookup. Useful when pointcloud_formats.srid = 0 and the
 *   caller knows the real SRID from a PostGIS context.
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
 * @brief SQL: tpcbox(pcpoint) — degenerate (single-point) bbox with
 *   spatial bounds equal to the point's X/Y/[Z]. SRID comes from the
 *   schema; pcid from the pcpoint itself. The per-backend schema
 *   cache makes this possible without dragging pgpointcloud's
 *   PG-layer API into MEOS.
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

#define TPCBOX_PRED_2(fn_name, meos_fn) \
  PGDLLEXPORT Datum fn_name(PG_FUNCTION_ARGS); \
  PG_FUNCTION_INFO_V1(fn_name); \
  Datum fn_name(PG_FUNCTION_ARGS) \
  { \
    TPCBox *a = PG_GETARG_TPCBOX_P(0); \
    TPCBox *b = PG_GETARG_TPCBOX_P(1); \
    PG_RETURN_BOOL(meos_fn(a, b)); \
  }

TPCBOX_PRED_2(Contains_tpcbox_tpcbox,  contains_tpcbox_tpcbox)
TPCBOX_PRED_2(Contained_tpcbox_tpcbox, contained_tpcbox_tpcbox)
TPCBOX_PRED_2(Overlaps_tpcbox_tpcbox,  overlaps_tpcbox_tpcbox)
TPCBOX_PRED_2(Same_tpcbox_tpcbox,      same_tpcbox_tpcbox)
TPCBOX_PRED_2(Adjacent_tpcbox_tpcbox,  adjacent_tpcbox_tpcbox)

/* Position predicates */
TPCBOX_PRED_2(Left_tpcbox_tpcbox,       left_tpcbox_tpcbox)
TPCBOX_PRED_2(Overleft_tpcbox_tpcbox,   overleft_tpcbox_tpcbox)
TPCBOX_PRED_2(Right_tpcbox_tpcbox,      right_tpcbox_tpcbox)
TPCBOX_PRED_2(Overright_tpcbox_tpcbox,  overright_tpcbox_tpcbox)
TPCBOX_PRED_2(Below_tpcbox_tpcbox,      below_tpcbox_tpcbox)
TPCBOX_PRED_2(Overbelow_tpcbox_tpcbox,  overbelow_tpcbox_tpcbox)
TPCBOX_PRED_2(Above_tpcbox_tpcbox,      above_tpcbox_tpcbox)
TPCBOX_PRED_2(Overabove_tpcbox_tpcbox,  overabove_tpcbox_tpcbox)
TPCBOX_PRED_2(Front_tpcbox_tpcbox,      front_tpcbox_tpcbox)
TPCBOX_PRED_2(Overfront_tpcbox_tpcbox,  overfront_tpcbox_tpcbox)
TPCBOX_PRED_2(Back_tpcbox_tpcbox,       back_tpcbox_tpcbox)
TPCBOX_PRED_2(Overback_tpcbox_tpcbox,   overback_tpcbox_tpcbox)
TPCBOX_PRED_2(Before_tpcbox_tpcbox,     before_tpcbox_tpcbox)
TPCBOX_PRED_2(Overbefore_tpcbox_tpcbox, overbefore_tpcbox_tpcbox)
TPCBOX_PRED_2(After_tpcbox_tpcbox,      after_tpcbox_tpcbox)
TPCBOX_PRED_2(Overafter_tpcbox_tpcbox,  overafter_tpcbox_tpcbox)

/*****************************************************************************
 * Comparison
 *****************************************************************************/

TPCBOX_PRED_2(Tpcbox_eq, tpcbox_eq)
TPCBOX_PRED_2(Tpcbox_ne, tpcbox_ne)
TPCBOX_PRED_2(Tpcbox_lt, tpcbox_lt)
TPCBOX_PRED_2(Tpcbox_le, tpcbox_le)
TPCBOX_PRED_2(Tpcbox_gt, tpcbox_gt)
TPCBOX_PRED_2(Tpcbox_ge, tpcbox_ge)

PGDLLEXPORT Datum Tpcbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpcbox_cmp);
/**
 * @brief SQL: btree comparator for tpcbox
 */
Datum
Tpcbox_cmp(PG_FUNCTION_ARGS)
{
  TPCBox *a = PG_GETARG_TPCBOX_P(0);
  TPCBox *b = PG_GETARG_TPCBOX_P(1);
  PG_RETURN_INT32(tpcbox_cmp(a, b));
}

/*****************************************************************************/
