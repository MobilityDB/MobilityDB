/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Spatial functions for temporal points.
 */

#include "point/tpoint_spatialfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_util.h"
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * PostGIS cache functions
 *****************************************************************************/

/**
 * Global variable to save the fcinfo when PostGIS functions need to access
 * the proj cache such as transform, geography_distance, or geography_azimuth
 */
FunctionCallInfo _FCINFO;

/**
 * Fetch from the cache the fcinfo of the external function
 */
FunctionCallInfo
fetch_fcinfo()
{
  assert(_FCINFO);
  return _FCINFO;
}

/**
 * Store in the cache the fcinfo of the external function
 */
void
store_fcinfo(FunctionCallInfo fcinfo)
{
  _FCINFO = fcinfo;
  return;
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * Generic function for the temporal ever/always comparison operators
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
tpoint_ev_al_comp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool result = func(temp, PointerGetDatum(gs));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Tpoint_ever_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfunc ever_eq()
 */
PGDLLEXPORT Datum
Tpoint_ever_eq(PG_FUNCTION_ARGS)
{
  return tpoint_ev_al_comp_ext(fcinfo, &tpoint_ever_eq);
}

PG_FUNCTION_INFO_V1(Tpoint_always_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfunc always_eq()
 */
PGDLLEXPORT Datum
Tpoint_always_eq(PG_FUNCTION_ARGS)
{
  return tpoint_ev_al_comp_ext(fcinfo, &tpoint_always_eq);
}

PG_FUNCTION_INFO_V1(Tpoint_ever_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfunc ever_ne()
 */
PGDLLEXPORT Datum
Tpoint_ever_ne(PG_FUNCTION_ARGS)
{
  return ! tpoint_ev_al_comp_ext(fcinfo, &tpoint_always_eq);
}

PG_FUNCTION_INFO_V1(Tpoint_always_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfunc always_ne()
 */
PGDLLEXPORT Datum
Tpoint_always_ne(PG_FUNCTION_ARGS)
{
  return ! tpoint_ev_al_comp_ext(fcinfo, &tpoint_ever_eq);
}

/*****************************************************************************
 * Trajectory function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_get_trajectory);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point
 * @sqlfunc trajectory()
 */
PGDLLEXPORT Datum
Tpoint_get_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tpoint_trajectory(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point
 * @sqlfunc SRID()
 */
PGDLLEXPORT Datum
Tpoint_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tpoint_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Tpoint_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the SRID of a temporal point
 * @sqlfunc setSRID()
 */
PGDLLEXPORT Datum
Tpoint_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tpoint_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * @brief Call the PostGIS transform function. We need to use the fcinfo cached
 * in the external functions tpoint_transform
 */
Datum
datum_transform(Datum value, Datum srid)
{
  return CallerFInfoFunctionCall2(transform, (fetch_fcinfo())->flinfo,
    InvalidOid, value, srid);
}

/**
 * @brief Transform a temporal point into another spatial reference system
 */
TInstant *
tpointinst_transform(const TInstant *inst, int srid)
{
  Datum geo = datum_transform(tinstant_value(inst), Int32GetDatum(srid));
  TInstant *result = tinstant_make(geo, inst->temptype, inst->t);
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * @brief Transform a temporal point into another spatial reference system
 */
TSequence *
tpointseq_transform(const TSequence *seq, int srid)
{
  interpType interp = MOBDB_FLAGS_GET_INTERP(seq->flags);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tpointinst_transform(tsequence_inst_n(seq, 0),
      Int32GetDatum(srid));
    TSequence *result = tinstant_to_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  /* General case */
  /* Call the discrete sequence function even for continuous sequences
   * to obtain a Multipoint that is sent to PostGIS for transformion */
  Datum multipoint = PointerGetDatum(tpointdiscseq_trajectory(seq));
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[i])));
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tinstant_make(point, inst->temptype, inst->t);
    pfree(DatumGetPointer(point));
  }
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(transf));
  pfree(DatumGetPointer(transf)); pfree(DatumGetPointer(multipoint));
  lwmpoint_free(lwmpoint);

  return tsequence_make_free(instants, seq->count, seq->count, true, true,
    interp, NORMALIZE_NO);
}

/**
 * @brief Transform a temporal point into another spatial reference system
 *
 * @note In order to do a SINGLE call to the PostGIS transform function we do
 * not iterate through the sequences and call the transform for the sequence
 */
TSequenceSet *
tpointseqset_transform(const TSequenceSet *ss, int srid)
{
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence *seq1 = tpointseq_transform(tsequenceset_seq_n(ss, 0),
      Int32GetDatum(srid));
    TSequenceSet *result = tsequence_to_tsequenceset(seq1);
    pfree(seq1);
    return result;
  }

  /* General case */
  int k = 0;
  const TSequence *seq;
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int maxcount = -1; /* number of instants of the longest sequence */
  for (int i = 0; i < ss->count; i++)
  {
    seq = tsequenceset_seq_n(ss, i);
    maxcount = Max(maxcount, seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      Datum value = tinstant_value(tsequence_inst_n(seq, j));
      GSERIALIZED *gsvalue = DatumGetGserializedP(value);
      points[k++] = lwgeom_from_gserialized(gsvalue);
    }
  }
  /* Last parameter set to STEPWISE to force the function to return multipoint */
  LWGEOM *lwgeom = lwpointarr_make_trajectory(points, ss->totalcount, STEPWISE);
  Datum multipoint = PointerGetDatum(geo_serialize(lwgeom));
  pfree(lwgeom);
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  TInstant **instants = palloc(sizeof(TInstant *) * maxcount);
  interpType interp = MOBDB_FLAGS_GET_INTERP(ss->flags);
  k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    seq = tsequenceset_seq_n(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[k++])));
      const TInstant *inst = tsequence_inst_n(seq, j);
      instants[j] = tinstant_make(point, inst->temptype, inst->t);
      pfree(DatumGetPointer(point));
    }
    sequences[i] = tsequence_make((const TInstant **) instants, seq->count,
      seq->count, seq->period.lower_inc, seq->period.upper_inc, interp,
      NORMALIZE_NO);
    for (int j = 0; j < seq->count; j++)
      pfree(instants[j]);
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ss->count,
    NORMALIZE_NO);
  for (int i = 0; i < ss->totalcount; i++)
    lwpoint_free((LWPOINT *) points[i]);
  pfree(points); pfree(instants);
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(transf));
  pfree(DatumGetPointer(transf)); pfree(DatumGetPointer(multipoint));
  lwmpoint_free(lwmpoint);
  return result;
}

/**
 * @brief Transform a temporal point into another spatial reference system
 */
Temporal *
tpoint_transform(const Temporal *temp, int srid)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_transform((TInstant *) temp, srid);
  else if (temp->subtype == TSEQUENCE)
    result =  (Temporal *) tpointseq_transform((TSequence *) temp, srid);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_transform((TSequenceSet *) temp, srid);
  return result;
}

PG_FUNCTION_INFO_V1(Tpoint_transform);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Transform a temporal point into another spatial reference system
 * @sqlfunc transform()
 */
PGDLLEXPORT Datum
Tpoint_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int srid = PG_GETARG_INT32(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_transform(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tgeompoint_to_tgeogpoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Convert a temporal geometry point to a temporal geography point
 * @sqlfunc tgeogpoint()
 */
PGDLLEXPORT Datum
Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOM_TO_GEOG);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tgeogpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Convert a temporal geography point to a temporal geometry point
 * @sqlfunc tgeompoint()
 */
PGDLLEXPORT Datum
Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOG_TO_GEOM);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set precision of the coordinates
 *****************************************************************************/

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_point(POINTARRAY *points, uint32_t i, int prec, bool hasz,
  bool hasm)
{
  /* N.B. lwpoint->point can be of 2, 3, or 4 dimensions depending on
   * the values of the arguments hasz and hasm !!! */
  POINT4D *pt = (POINT4D *) getPoint_internal(points, i);
  pt->x = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->x), prec));
  pt->y = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->y), prec));
  if (hasz && hasm)
  {
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), prec));
    pt->m = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->m), prec));
  }
  else if (hasz)
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), prec));
  else if (hasm)
    /* The m coordinate is located at the third double of the point */
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), prec));
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_point(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == POINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOINT *lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  round_point(lwpoint->point, 0, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoint);
  pfree(lwpoint);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_linestring(LWLINE *lwline, Datum prec, bool hasz, bool hasm)
{
  int npoints = lwline->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwline->points, i, prec, hasz, hasm);
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_linestring(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == LINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  round_linestring(lwline, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwline);
  lwfree(lwline);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_triangle(LWTRIANGLE *lwtriangle, Datum prec, bool hasz, bool hasm)
{
  int npoints = lwtriangle->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwtriangle->points, i, prec, hasz, hasm);
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_triangle(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == TRIANGLETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWTRIANGLE *lwtriangle = lwgeom_as_lwtriangle(lwgeom_from_gserialized(gs));
  round_triangle(lwtriangle, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwtriangle);
  lwfree(lwtriangle);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_circularstring(LWCIRCSTRING *lwcircstring, Datum prec, bool hasz,
  bool hasm)
{
  int npoints = lwcircstring->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwcircstring->points, i, prec, hasz, hasm);
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_circularstring(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == CIRCSTRINGTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCIRCSTRING *lwcircstring = lwgeom_as_lwcircstring(lwgeom_from_gserialized(gs));
  round_circularstring(lwcircstring, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwcircstring);
  lwfree(lwcircstring);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_polygon(LWPOLY *lwpoly, Datum prec, bool hasz, bool hasm)
{
  int nrings = lwpoly->nrings;
  for (int i = 0; i < nrings; i++)
  {
    POINTARRAY *points = lwpoly->rings[i];
    int npoints = points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point(points, j, prec, hasz, hasm);
  }
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_polygon(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == POLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOLY *lwpoly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs));
  round_polygon(lwpoly, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoly);
  lwfree(lwpoly);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_multipoint(LWMPOINT *lwmpoint, Datum prec, bool hasz, bool hasm)
{
  int ngeoms = lwmpoint->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOINT *lwpoint = lwmpoint->geoms[i];
    round_point(lwpoint->point, 0, prec, hasz, hasm);
  }
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multipoint(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == MULTIPOINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOINT *lwmpoint =  lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  round_multipoint(lwmpoint, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmpoint);
  lwfree(lwmpoint);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_multilinestring(LWMLINE *lwmline, Datum prec, bool hasz, bool hasm)
{
  int ngeoms = lwmline->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWLINE *lwline = lwmline->geoms[i];
    int npoints = lwline->points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point(lwline->points, j, prec, hasz, hasm);
  }
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multilinestring(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == MULTILINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMLINE *lwmline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  round_multilinestring(lwmline, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmline);
  lwfree(lwmline);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static void
round_multipolygon(LWMPOLY *lwmpoly, Datum prec, bool hasz, bool hasm)
{
  int ngeoms = lwmpoly->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOLY *lwpoly = lwmpoly->geoms[i];
    round_polygon(lwpoly, prec, hasz, hasm);
  }
  return;
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multipolygon(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == MULTIPOLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOLY *lwmpoly = lwgeom_as_lwmpoly(lwgeom_from_gserialized(gs));
  round_multipolygon(lwmpoly, prec, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmpoly);
  lwfree(lwmpoly);
  return PointerGetDatum(result);
}

/**
 * Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_geometrycollection(GSERIALIZED *gs, Datum prec)
{
  assert(gserialized_get_type(gs) == COLLECTIONTYPE);
  LWCOLLECTION *lwcol = lwgeom_as_lwcollection(lwgeom_from_gserialized(gs));
  int ngeoms = lwcol->ngeoms;
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  for (int i = 0; i < ngeoms; i++)
  {
    LWGEOM *lwgeom = lwcol->geoms[i];
    if (lwgeom->type == POINTTYPE)
      round_point((lwgeom_as_lwpoint(lwgeom))->point, 0, prec, hasz, hasm);
    else if (lwgeom->type == LINETYPE)
      round_linestring(lwgeom_as_lwline(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == TRIANGLETYPE)
      round_triangle(lwgeom_as_lwtriangle(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == CIRCSTRINGTYPE)
      round_circularstring(lwgeom_as_lwcircstring(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == POLYGONTYPE)
      round_polygon(lwgeom_as_lwpoly(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == MULTIPOINTTYPE)
      round_multipoint(lwgeom_as_lwmpoint(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == MULTILINETYPE)
      round_multilinestring(lwgeom_as_lwmline(lwgeom), prec, hasz, hasm);
    else if (lwgeom->type == MULTIPOLYGONTYPE)
      round_multipolygon(lwgeom_as_lwmpoly(lwgeom), prec, hasz, hasm);
    else
      elog(ERROR, "Unsupported geometry type");
  }
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwcol);
  lwfree(lwcol);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places.
 *
 * @note Currently not all geometry types are allowed
 */
Datum
datum_round_geo(Datum value, Datum prec)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  if (gserialized_is_empty(gs))
    return PointerGetDatum(gserialized_copy(gs));

  uint32_t type = gserialized_get_type(gs);
  if (type == POINTTYPE)
    return datum_round_point(gs, prec);
  if (type == LINETYPE)
    return datum_round_linestring(gs, prec);
  if (type == TRIANGLETYPE)
    return datum_round_triangle(gs, prec);
  if (type == CIRCSTRINGTYPE)
    return datum_round_circularstring(gs, prec);
  if (type == POLYGONTYPE)
    return datum_round_polygon(gs, prec);
  if (type == MULTIPOINTTYPE)
    return datum_round_multipoint(gs, prec);
  if (type == MULTILINETYPE)
    return datum_round_multilinestring(gs, prec);
  if (type == MULTIPOLYGONTYPE)
    return datum_round_multipolygon(gs, prec);
  if (type == COLLECTIONTYPE)
    return datum_round_geometrycollection(gs, prec);
  elog(ERROR, "Unsupported geometry type");
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Set the precision of the coordinates of a temporal point to a
 * number of decimal places.
 * @sqlfunc round()
 */
Temporal *
tpoint_round(const Temporal *temp, int prec)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_geo;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(prec);
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(Geo_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Sets the precision of the coordinates of the geometry
 * @sqlfunc round()
 */
PGDLLEXPORT Datum
Geo_round(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int prec = PG_GETARG_INT32(1);
  PG_RETURN_POINTER(datum_round_geo(PointerGetDatum(gs), Int32GetDatum(prec)));
}

PG_FUNCTION_INFO_V1(Tpoint_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the precision of the coordinates of a temporal point to a number
 * of decimal places
 * @sqlfunc round()
 */
PGDLLEXPORT Datum
Tpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int prec = PG_GETARG_INT32(1);
  Temporal *result = tpoint_round(temp, prec);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_get_x);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the X coordinates of a temporal point
 * @sqlfunc getX()
 */
PGDLLEXPORT Datum
Tpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_get_y);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the Y coordinates of a temporal point
 * @sqlfunc getY()
 */
PGDLLEXPORT Datum
Tpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 1);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_get_z);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the Z coordinates of a temporal point
 * @sqlfunc getZ()
 */
PGDLLEXPORT Datum
Tpoint_get_z(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence (set) point
 * @sqlfunc length()
 */
PGDLLEXPORT Datum
Tpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tpoint_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tpoint_cumulative_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point
 * @sqlfunc cumulativeLength()
 */
PGDLLEXPORT Datum
Tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_speed);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfunc speed()
 */
PGDLLEXPORT Datum
Tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_twcentroid);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point
 * @sqlfunc twcentroid()
 */
PGDLLEXPORT Datum
Tpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = PointerGetDatum(tpoint_twcentroid(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_azimuth);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point
 * @sqlfunc azimuth()
 */
PGDLLEXPORT Datum
Tpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Bearing_point_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two geometry/geography points
 * @note The following function is meant to be included in PostGIS one day
 * @sqlfunc bearing()
 */
PGDLLEXPORT Datum
Bearing_point_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *geo2 = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result;
  bool found = bearing_point_point(geo1, geo2, &result);
  PG_FREE_IF_COPY(geo1, 0);
  PG_FREE_IF_COPY(geo2, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Bearing_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a geometry/geography point
 * and a temporal point
 * @sqlfunc bearing()
 */
PGDLLEXPORT Datum
Bearing_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_point(temp, gs, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Bearing_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a temporal point and a
 * geometry/geography point
 * @sqlfunc bearing()
 */
PGDLLEXPORT Datum
Bearing_tpoint_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_point(temp, gs, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Bearing_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two temporal points
 * @sqlfunc bearing()
 */
PGDLLEXPORT Datum
Bearing_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_is_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect.
 * @sqlfunc isSimple()
 */
PGDLLEXPORT Datum
Tpoint_is_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(tpoint_is_simple(temp));
}

PG_FUNCTION_INFO_V1(Tpoint_make_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Split a temporal point into an array of non self-intersecting pieces
 * @sqlfunc makeSimple()
 */
PGDLLEXPORT Datum
Tpoint_make_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Temporal **pieces = tpoint_make_simple(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) pieces, count);
  pfree_array((void **) pieces, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * Restrict a temporal point to (the complement of) a geometry
 *
 * Mixing 2D/3D is enabled to compute, for example, 2.5D operations
 */
static Datum
tpoint_restrict_geometry_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpoint_restrict_geometry(temp, geo, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_at_geometry);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to a geometry
 * @sqlfunc atGeometry()
 */
PGDLLEXPORT Datum
Tpoint_at_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tpoint_minus_geometry);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a geometry
 * @sqlfunc minusGeometry()
 */
PGDLLEXPORT Datum
Tpoint_minus_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_at_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to a spatiotemporal box
 * @sqlfunc atStbox()
 */
PGDLLEXPORT Datum
Tpoint_at_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_minus_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a spatiotemporal box
 * @sqlfunc minusStbox()
 */
PGDLLEXPORT Datum
Tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
