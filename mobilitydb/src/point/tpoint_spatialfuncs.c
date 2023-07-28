/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Spatial functions for temporal points.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/tsequence.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_restrfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
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

PGDLLEXPORT Datum Tpoint_ever_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_ever_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is ever equal to a point
 * @sqlfunc ever_eq()
 */
Datum
Tpoint_ever_eq(PG_FUNCTION_ARGS)
{
  return tpoint_ev_al_comp_ext(fcinfo, &tpoint_ever_eq);
}

PGDLLEXPORT Datum Tpoint_always_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_always_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is always equal to a point
 * @sqlfunc always_eq()
 */
Datum
Tpoint_always_eq(PG_FUNCTION_ARGS)
{
  return tpoint_ev_al_comp_ext(fcinfo, &tpoint_always_eq);
}

PGDLLEXPORT Datum Tpoint_ever_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_ever_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is ever different from a point
 * @sqlfunc ever_ne()
 */
Datum
Tpoint_ever_ne(PG_FUNCTION_ARGS)
{
  return ! tpoint_ev_al_comp_ext(fcinfo, &tpoint_always_eq);
}

PGDLLEXPORT Datum Tpoint_always_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_always_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal point is always different from a point
 * @sqlfunc always_ne()
 */
Datum
Tpoint_always_ne(PG_FUNCTION_ARGS)
{
  return ! tpoint_ev_al_comp_ext(fcinfo, &tpoint_ever_eq);
}

/*****************************************************************************
 * Trajectory function
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_trajectory(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_trajectory);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point
 * @sqlfunc trajectory()
 */
Datum
Tpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tpoint_trajectory(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal point
 * @sqlfunc SRID()
 */
Datum
Tpoint_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tpoint_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tpoint_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the SRID of a temporal point
 * @sqlfunc setSRID()
 */
Datum
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
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TInstant *inst = tpointinst_transform(TSEQUENCE_INST_N(seq, 0),
      Int32GetDatum(srid));
    TSequence *result = tinstant_to_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  /* General case */
  /* Call the discrete sequence function even for continuous sequences
   * to obtain a Multipoint that is sent to PostGIS for transformion */
  Datum multipoint = PointerGetDatum(tpointseq_disc_trajectory(seq));
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    Datum point = PointerGetDatum(geo_serialize((LWGEOM *) (lwmpoint->geoms[i])));
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    instants[i] = tinstant_make(point, inst->temptype, inst->t);
    pfree(DatumGetPointer(point));
  }
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(transf));
  pfree(DatumGetPointer(transf)); pfree(DatumGetPointer(multipoint));
  lwmpoint_free(lwmpoint);

  return tsequence_make_free(instants, seq->count, true, true, interp,
    NORMALIZE_NO);
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
    TSequence *seq1 = tpointseq_transform(TSEQUENCESET_SEQ_N(ss, 0),
      Int32GetDatum(srid));
    TSequenceSet *result = tsequence_to_tsequenceset(seq1);
    pfree(seq1);
    return result;
  }

  /* General case */
  int npoints = 0;
  const TSequence *seq;
  LWGEOM **points = palloc(sizeof(LWGEOM *) * ss->totalcount);
  int maxcount = -1; /* number of instants of the longest sequence */
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    maxcount = Max(maxcount, seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      Datum value = tinstant_value(TSEQUENCE_INST_N(seq, j));
      GSERIALIZED *gsvalue = DatumGetGserializedP(value);
      points[npoints++] = lwgeom_from_gserialized(gsvalue);
    }
  }
  /* Last parameter set to STEP to force the function to return multipoint */
  LWGEOM *lwgeom = lwpointarr_make_trajectory(points, ss->totalcount, STEP);
  Datum multipoint = PointerGetDatum(geo_serialize(lwgeom));
  lwgeom_free(lwgeom);
  Datum transf = datum_transform(multipoint, srid);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(transf);
  LWMPOINT *lwmpoint = lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  TInstant **instants = palloc(sizeof(TInstant *) * maxcount);
  interpType interp = MEOS_FLAGS_GET_INTERP(ss->flags);
  npoints = 0;
  for (int i = 0; i < ss->count; i++)
  {
    seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      GSERIALIZED *point = geo_serialize((LWGEOM *)
        (lwmpoint->geoms[npoints++]));
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      instants[j] = tinstant_make(PointerGetDatum(point), inst->temptype,
        inst->t);
      pfree(point);
    }
    sequences[i] = tsequence_make((const TInstant **) instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE_NO);
    for (int j = 0; j < seq->count; j++)
      pfree(instants[j]);
  }
  TSequenceSet *result = tsequenceset_make_free(sequences, ss->count,
    NORMALIZE_NO);
  pfree(instants);
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tpointinst_transform((TInstant *) temp, srid);
  else if (temp->subtype == TSEQUENCE)
    result =  (Temporal *) tpointseq_transform((TSequence *) temp, srid);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tpointseqset_transform((TSequenceSet *) temp, srid);
  return result;
}

PGDLLEXPORT Datum Tpoint_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_transform);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Transform a temporal point into another spatial reference system
 * @sqlfunc transform()
 */
Datum
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

PGDLLEXPORT Datum Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tgeogpoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Convert a temporal geometry point to a temporal geography point
 * @sqlfunc tgeogpoint()
 */
Datum
Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOM_TO_GEOG);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Convert a temporal geography point to a temporal geometry point
 * @sqlfunc tgeompoint()
 */
Datum
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
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_point(POINTARRAY *points, uint32_t i, Datum size, bool hasz, bool hasm)
{
  /* N.B. lwpoint->point can be of 2, 3, or 4 dimensions depending on
   * the values of the arguments hasz and hasm !!! */
  POINT4D *pt = (POINT4D *) getPoint_internal(points, i);
  pt->x = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->x), size));
  pt->y = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->y), size));
  if (hasz && hasm)
  {
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), size));
    pt->m = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->m), size));
  }
  else if (hasz)
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), size));
  else if (hasm)
    /* The m coordinate is located at the third double of the point */
    pt->z = DatumGetFloat8(datum_round_float(Float8GetDatum(pt->z), size));
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_point(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == POINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOINT *lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs));
  round_point(lwpoint->point, 0, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoint);
  pfree(lwpoint);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_linestring(LWLINE *lwline, Datum size, bool hasz, bool hasm)
{
  int npoints = lwline->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwline->points, i, size, hasz, hasm);
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_linestring(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == LINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWLINE *lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  round_linestring(lwline, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwline);
  lwfree(lwline);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_triangle(LWTRIANGLE *lwtriangle, Datum size, bool hasz, bool hasm)
{
  int npoints = lwtriangle->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwtriangle->points, i, size, hasz, hasm);
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_triangle(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == TRIANGLETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWTRIANGLE *lwtriangle = lwgeom_as_lwtriangle(lwgeom_from_gserialized(gs));
  round_triangle(lwtriangle, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwtriangle);
  lwfree(lwtriangle);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_circularstring(LWCIRCSTRING *lwcircstring, Datum size, bool hasz,
  bool hasm)
{
  int npoints = lwcircstring->points->npoints;
  for (int i = 0; i < npoints; i++)
    round_point(lwcircstring->points, i, size, hasz, hasm);
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_circularstring(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == CIRCSTRINGTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWCIRCSTRING *lwcircstring = lwgeom_as_lwcircstring(lwgeom_from_gserialized(gs));
  round_circularstring(lwcircstring, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwcircstring);
  lwfree(lwcircstring);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_polygon(LWPOLY *lwpoly, Datum size, bool hasz, bool hasm)
{
  int nrings = lwpoly->nrings;
  for (int i = 0; i < nrings; i++)
  {
    POINTARRAY *points = lwpoly->rings[i];
    int npoints = points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point(points, j, size, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_polygon(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == POLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWPOLY *lwpoly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs));
  round_polygon(lwpoly, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoly);
  lwfree(lwpoly);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_multipoint(LWMPOINT *lwmpoint, Datum size, bool hasz, bool hasm)
{
  int ngeoms = lwmpoint->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOINT *lwpoint = lwmpoint->geoms[i];
    round_point(lwpoint->point, 0, size, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multipoint(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == MULTIPOINTTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOINT *lwmpoint =  lwgeom_as_lwmpoint(lwgeom_from_gserialized(gs));
  round_multipoint(lwmpoint, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmpoint);
  lwfree(lwmpoint);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_multilinestring(LWMLINE *lwmline, Datum size, bool hasz, bool hasm)
{
  int ngeoms = lwmline->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWLINE *lwline = lwmline->geoms[i];
    int npoints = lwline->points->npoints;
    for (int j = 0; j < npoints; j++)
      round_point(lwline->points, j, size, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multilinestring(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == MULTILINETYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMLINE *lwmline = lwgeom_as_lwmline(lwgeom_from_gserialized(gs));
  round_multilinestring(lwmline, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmline);
  lwfree(lwmline);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static void
round_multipolygon(LWMPOLY *lwmpoly, Datum size, bool hasz, bool hasm)
{
  int ngeoms = lwmpoly->ngeoms;
  for (int i = 0; i < ngeoms; i++)
  {
    LWPOLY *lwpoly = lwmpoly->geoms[i];
    round_polygon(lwpoly, size, hasz, hasm);
  }
  return;
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_multipolygon(GSERIALIZED *gs, Datum size)
{
  assert(gserialized_get_type(gs) == MULTIPOLYGONTYPE);
  bool hasz = (bool) FLAGS_GET_Z(gs->gflags);
  bool hasm = (bool) FLAGS_GET_M(gs->gflags);
  LWMPOLY *lwmpoly = lwgeom_as_lwmpoly(lwgeom_from_gserialized(gs));
  round_multipolygon(lwmpoly, size, hasz, hasm);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwmpoly);
  lwfree(lwmpoly);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places
 */
static Datum
datum_round_geometrycollection(GSERIALIZED *gs, Datum size)
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
      round_point((lwgeom_as_lwpoint(lwgeom))->point, 0, size, hasz, hasm);
    else if (lwgeom->type == LINETYPE)
      round_linestring(lwgeom_as_lwline(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == TRIANGLETYPE)
      round_triangle(lwgeom_as_lwtriangle(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == CIRCSTRINGTYPE)
      round_circularstring(lwgeom_as_lwcircstring(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == POLYGONTYPE)
      round_polygon(lwgeom_as_lwpoly(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == MULTIPOINTTYPE)
      round_multipoint(lwgeom_as_lwmpoint(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == MULTILINETYPE)
      round_multilinestring(lwgeom_as_lwmline(lwgeom), size, hasz, hasm);
    else if (lwgeom->type == MULTIPOLYGONTYPE)
      round_multipolygon(lwgeom_as_lwmpoly(lwgeom), size, hasz, hasm);
    else
      elog(ERROR, "Unsupported geometry type");
  }
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwcol);
  lwfree(lwcol);
  return PointerGetDatum(result);
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places.
 * @note Currently not all geometry types are allowed
 */
Datum
datum_round_geo(Datum value, Datum size)
{
  GSERIALIZED *gs = DatumGetGserializedP(value);
  if (gserialized_is_empty(gs))
    return PointerGetDatum(gserialized_copy(gs));

  uint32_t type = gserialized_get_type(gs);
  if (type == POINTTYPE)
    return datum_round_point(gs, size);
  if (type == LINETYPE)
    return datum_round_linestring(gs, size);
  if (type == TRIANGLETYPE)
    return datum_round_triangle(gs, size);
  if (type == CIRCSTRINGTYPE)
    return datum_round_circularstring(gs, size);
  if (type == POLYGONTYPE)
    return datum_round_polygon(gs, size);
  if (type == MULTIPOINTTYPE)
    return datum_round_multipoint(gs, size);
  if (type == MULTILINETYPE)
    return datum_round_multilinestring(gs, size);
  if (type == MULTIPOLYGONTYPE)
    return datum_round_multipolygon(gs, size);
  if (type == COLLECTIONTYPE)
    return datum_round_geometrycollection(gs, size);
  elog(ERROR, "Unsupported geometry type");
  return Float8GetDatum(0); /* make compiler quiet */
}

/**
 * @brief Set the precision of the coordinates to the number of decimal places.
 */
Set *
geoset_round(const Set *s, Datum size)
{
  Datum *values = palloc(sizeof(Datum) * s->count);
  for (int i = 0; i < s->count; i++)
  {
    Datum value = SET_VAL_N(s, i);
    values[i] = datum_round_geo(value, size);
  }
  Set *result = set_make(values, s->count, s->basetype, ORDERED);
  pfree(values);
  return result;
}

/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the precision of the coordinates of a temporal point to a
 * number of decimal places.
 * @sqlfunc round()
 */
Temporal *
tpoint_round(const Temporal *temp, Datum size)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_geo;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

PGDLLEXPORT Datum Geo_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Sets the precision of the coordinates of the geometry
 * @sqlfunc round()
 */
Datum
Geo_round(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum result = datum_round_geo(PointerGetDatum(gs), size);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Geoset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Sets the precision of the coordinates of the geometry set
 * @sqlfunc round()
 */
Datum
Geoset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Set *result = geoset_round(s, size);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the precision of the coordinates of a temporal point to a number
 * of decimal places
 * @sqlfunc round()
 */
Datum
Tpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tpoint_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_get_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_x);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the X coordinates of a temporal point
 * @sqlfunc getX()
 */
Datum
Tpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_get_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_y);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the Y coordinates of a temporal point
 * @sqlfunc getY()
 */
Datum
Tpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 1);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_get_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_z);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Get the Z coordinates of a temporal point
 * @sqlfunc getZ()
 */
Datum
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

PGDLLEXPORT Datum Tpoint_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence (set) point
 * @sqlfunc length()
 */
Datum
Tpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tpoint_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tpoint_cumulative_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_cumulative_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal point
 * @sqlfunc cumulativeLength()
 */
Datum
Tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_convex_hull);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the convex hull of  a temporal point
 * @sqlfunc convexHull()
 */
Datum
Tpoint_convex_hull(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  GSERIALIZED *result = tpoint_convex_hull(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_speed);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfunc speed()
 */
Datum
Tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Direction function
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_direction(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_direction);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the direction of a temporal point, that is, the azimuth
 * between the first and the last points
 * @sqlfunc direction()
 */
Datum
Tpoint_direction(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result;
  bool found = tpoint_direction(temp, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_twcentroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_twcentroid);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point
 * @sqlfunc twcentroid()
 */
Datum
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

PGDLLEXPORT Datum Tpoint_azimuth(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_azimuth);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point
 * @sqlfunc azimuth()
 */
Datum
Tpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal angular difference
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_angular_difference(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_angular_difference);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal angular difference of a temporal geometry point
 * @sqlfunc angularDifference()
 */
Datum
Tpoint_angular_difference(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_angular_difference(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

PGDLLEXPORT Datum Bearing_point_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_point_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two geometry/geography points
 * @note The following function is meant to be included in PostGIS one day
 * @sqlfunc bearing()
 */
Datum
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

PGDLLEXPORT Datum Bearing_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a geometry/geography point
 * and a temporal point
 * @sqlfunc bearing()
 */
Datum
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

PGDLLEXPORT Datum Bearing_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a temporal point and a
 * geometry/geography point
 * @sqlfunc bearing()
 */
Datum
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

PGDLLEXPORT Datum Bearing_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two temporal points
 * @sqlfunc bearing()
 */
Datum
Bearing_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_is_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_is_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect.
 * @sqlfunc isSimple()
 */
Datum
Tpoint_is_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool result = tpoint_is_simple(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tpoint_make_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_make_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Split a temporal point into an array of non self-intersecting pieces
 * @sqlfunc makeSimple()
 */
Datum
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
 * @brief Restrict a temporal point to (the complement of) a geometry and
 * possibly a period.
 * @note Mixing 2D/3D is enabled to compute, for example, 2.5D operations.
 * However the geometry must be in 2D.
 */
static Datum
tpoint_restrict_geom_time_ext(FunctionCallInfo fcinfo, bool atfunc,
  bool resttime)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)|| (resttime && PG_ARGISNULL(3)))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Span *zspan = NULL;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    zspan = PG_GETARG_SPAN_P(2);
  Span *period = NULL;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
    period = PG_GETARG_SPAN_P(3);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_restrict_geom_time(temp, geo, zspan, period,
    atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to a geometry
 * @sqlfunc atGeometry()
 */
Datum
Tpoint_at_geom(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geom_time_ext(fcinfo, REST_AT, REST_TIME_NO);
}

PGDLLEXPORT Datum Tpoint_at_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to a geometry
 * @sqlfunc atGeometry()
 */
Datum
Tpoint_at_geom_time(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geom_time_ext(fcinfo, REST_AT, REST_TIME);
}

PGDLLEXPORT Datum Tpoint_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a geometry
 * @sqlfunc minusGeometry()
 */
Datum
Tpoint_minus_geom(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geom_time_ext(fcinfo, REST_MINUS, REST_TIME_NO);
}

PGDLLEXPORT Datum Tpoint_minus_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a geometry and a
 * period
 * @sqlfunc minusGeometryTime()
 */
Datum
Tpoint_minus_geom_time(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geom_time_ext(fcinfo, REST_MINUS, REST_TIME);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to a spatiotemporal box
 * @sqlfunc atStbox()
 */
Datum
Tpoint_at_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, BORDER_INC, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a spatiotemporal box
 * @sqlfunc minusStbox()
 */
Datum
Tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, BORDER_INC, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
