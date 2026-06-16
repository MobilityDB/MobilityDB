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
 * @brief C implementation of raster_value() — PostGIS raster band sampling
 * along tgeompoint trajectories.
 *
 * The function iterates over the instants of the trajectory in C, filters
 * out-of-range positions with a bounding-box pre-check derived from the
 * raster's convex hull, calls PostGIS ST_Value() for each in-range instant,
 * and assembles the surviving (value, timestamp) pairs directly into a
 * heap-allocated TSequence (DISCRETE interpolation), bypassing the SQL
 * string_agg / to_char / text→tfloat pipeline.
 *
 * PostGIS raster internals (rt_api.h) are not exported from
 * postgis_raster-3.so.  The implementation therefore calls the published SQL
 * surface (ST_ConvexHull, ST_Value) via PostgreSQL's OidFunctionCall
 * mechanism, resolving OIDs once per session through regprocedurein.
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/*
 * utils/builtins.h pulls in fmgrprotos.h, which declares `json_object` as a
 * PostgreSQL callable function.  meos_internal.h then includes json-c/json.h
 * which tries to typedef the same name as a struct — a C-level conflict.
 * Forward-declare only the symbols we need to avoid the full builtins.h include.
 */
extern Datum regprocedurein(PG_FUNCTION_ARGS);
extern text *cstring_to_text(const char *s);
/* PostGIS liblwgeom (vendored) */
#include <liblwgeom.h>        /* GSERIALIZED, GBOX, gserialized_get_gbox_p */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>    /* temporal_insts_p, tsequence_make_free */
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_raster/temporal_raster.h"

/*****************************************************************************
 * OID cache helpers
 *****************************************************************************/

/**
 * @brief Return the OID of a function identified by its SQL signature string.
 *
 * Uses regprocedurein so that type names are resolved in the current search
 * path without hard-coding any numeric OIDs.
 */
static Oid
lookup_func_oid(const char *signature)
{
  return DatumGetObjectId(
    DirectFunctionCall1(regprocedurein,
      CStringGetDatum(signature)));
}

/** Cached OID for ST_ConvexHull(raster) → geometry */
static Oid st_convexhull_raster_oid = InvalidOid;

/** Cached OID for ST_Value(raster, integer, geometry, boolean) → float8 */
static Oid st_value_oid = InvalidOid;

static void
init_oids(void)
{
  if (st_convexhull_raster_oid == InvalidOid)
    st_convexhull_raster_oid =
      lookup_func_oid("st_convexhull(raster)");
  if (st_value_oid == InvalidOid)
    st_value_oid =
      lookup_func_oid("st_value(raster,integer,geometry,boolean,text)");
}

/*****************************************************************************
 * raster_value
 *****************************************************************************/

PGDLLEXPORT Datum Raster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] rast Raster
 * @param[in] traj Trajectory
 * @param[in] band Band number (1-based, default 1)
 * @csqlfn #Raster_value()
 */
Datum
Raster_value(PG_FUNCTION_ARGS)
{
  /* ── arguments ──────────────────────────────────────────────────────── */
  Datum     rast_datum = PG_GETARG_DATUM(0);
  Temporal *traj       = PG_GETARG_TEMPORAL_P(1);
  int32     band       = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);

  /* ── OID resolution (once per session) ──────────────────────────────── */
  init_oids();

  /* ── Raster convex hull — used as a bounding-box pre-filter ─────────── */
  Datum hull_datum =
    OidFunctionCall1(st_convexhull_raster_oid, rast_datum);
  GSERIALIZED *hull_gs = (GSERIALIZED *) DatumGetPointer(hull_datum);
  GBOX         hull_box;
  gserialized_get_gbox_p(hull_gs, &hull_box);
  pfree(hull_gs);

  /* ── Iterate over trajectory instants ───────────────────────────────── */
  int count;
  const TInstant **insts = temporal_insts_p(traj, &count);

  TInstant **result_insts = palloc(sizeof(TInstant *) * count);
  int ninsts = 0;

  /* Prepare a reusable FunctionCallInfo for ST_Value (handles NULL returns) */
  FmgrInfo flinfo;
  fmgr_info(st_value_oid, &flinfo);

  LOCAL_FCINFO(fcinfo_val, 5);
  InitFunctionCallInfoData(*fcinfo_val, &flinfo, 5,
                           DEFAULT_COLLATION_OID, NULL, NULL);
  /* Arg 0 (raster) and Arg 1 (band) are constant for every instant */
  fcinfo_val->args[0].value  = rast_datum;
  fcinfo_val->args[0].isnull = false;
  fcinfo_val->args[1].value  = Int32GetDatum(band);
  fcinfo_val->args[1].isnull = false;
  /* Arg 3 (exclude_nodata) = false — return the nodata sentinel as-is so we
   * can detect a NULL and skip the instant, matching the SQL STRICT semantics */
  fcinfo_val->args[3].value  = BoolGetDatum(false);
  fcinfo_val->args[3].isnull = false;
  /* Arg 4 (resample) = 'nearest' — PostGIS 3.6+ added this parameter */
  fcinfo_val->args[4].value  = PointerGetDatum(cstring_to_text("nearest"));
  fcinfo_val->args[4].isnull = false;

  for (int i = 0; i < count; i++)
  {
    Datum geom_datum = tinstant_value(insts[i]);

    /* ── Bounding-box pre-filter ─────────────────────────────────────── */
    GSERIALIZED *pt_gs = (GSERIALIZED *) DatumGetPointer(geom_datum);
    GBOX         pt_box;
    gserialized_get_gbox_p(pt_gs, &pt_box);
    /* For a POINT, pt_box.xmin == xmax and ymin == ymax */
    if (pt_box.xmin < hull_box.xmin || pt_box.xmin > hull_box.xmax ||
        pt_box.ymin < hull_box.ymin || pt_box.ymin > hull_box.ymax)
      continue;

    /* ── ST_Value call with NULL detection ───────────────────────────── */
    fcinfo_val->args[2].value  = geom_datum;
    fcinfo_val->args[2].isnull = false;
    fcinfo_val->isnull         = false;   /* reset before every call */

    Datum pixval = FunctionCallInvoke(fcinfo_val);
    if (fcinfo_val->isnull)
      continue;   /* nodata pixel or geometry outside the pixel grid */

    result_insts[ninsts++] =
      tinstant_make(pixval, T_TFLOAT, insts[i]->t);
  }

  pfree(insts);

  if (ninsts == 0)
  {
    pfree(result_insts);
    PG_FREE_IF_COPY(traj, 1);
    PG_RETURN_NULL();
  }

  TSequence *result =
    tsequence_make_free(result_insts, ninsts, true, true, DISCRETE, NORMALIZE);

  PG_FREE_IF_COPY(traj, 1);
  PG_RETURN_POINTER(result);
}
