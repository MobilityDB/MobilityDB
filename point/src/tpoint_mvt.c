/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file tpoint_mvt.c
 * Mapbox Vector Tile functions for temporal points.
 */

#include <float.h>
#include <liblwgeom.h>

#include "tpoint_mvt.h"

#include "tempcache.h"
#include "temporaltypes.h"
#include "temporal_util.h"

#include "postgis.h"
#include "stbox.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_analytics.h"

/*****************************************************************************
 * MVT functions
 *****************************************************************************/

/*
 * Returns a temporal point with consecutive equal points removed.
 * Equality test on all dimensions of input.
 */
TInstantSet *
tpointinstset_remove_repeated_points(const TInstantSet *ti, double tolerance,
  int min_points)
{
  /* No-op on short inputs */
  if (ti->count <= min_points)
    return tinstantset_copy(ti);

  double tolsq = tolerance * tolerance;
  double dsq = FLT_MAX;

  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  instants[0] = tinstantset_inst_n(ti, 0);
  const POINT2D *last = datum_get_point2d_p(tinstant_value(instants[0]));
  int k = 1;
  for (int i = 1; i < ti->count; i++)
  {
    bool last_point = (i == ti->count - 1);
    const TInstant *inst = tinstantset_inst_n(ti, i);
    const POINT2D *pt = datum_get_point2d_p(tinstant_value(inst));

    /* Don't drop points if we are running short of points */
    if (ti->count - k > min_points + i)
    {
      if (tolerance > 0.0)
      {
        /* Only drop points that are within our tolerance */
        dsq = distance2d_sqr_pt_pt(last, pt);
        /* Allow any point but the last one to be dropped */
        if (!last_point && dsq <= tolsq)
          continue;
      }
      else
      {
        /* At tolerance zero, only skip exact dupes */
        if (FP_EQUALS(pt->x, last->x) && FP_EQUALS(pt->y, last->y))
          continue;
      }

      /* Got to last point, and it's not very different from
       * the point that preceded it. We want to keep the last
       * point, not the second-to-last one, so we pull our write
       * index back one value */
      if (last_point && k > 1 && tolerance > 0.0 && dsq <= tolsq)
      {
        k--;
      }
    }

    /* Save the point */
    instants[k++] = inst;
    last = pt;
  }
  /* Construct the result */
  TInstantSet *result = tinstantset_make(instants, ti->count, MERGE_NO);
  pfree(instants);
  return result;
}

/*
 * Returns a temporal point with consecutive equal points removed.
 * Equality test on x and y dimensions of input.
 */
TSequence *
tpointseq_remove_repeated_points(const TSequence *seq, double tolerance,
  int min_points)
{
  /* No-op on short inputs */
  if (seq->count <= min_points)
    return tsequence_copy(seq);

  double tolsq = tolerance * tolerance;
  double dsq = FLT_MAX;

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  instants[0] = tsequence_inst_n(seq, 0);
  const POINT2D *last = datum_get_point2d_p(tinstant_value(instants[0]));
  int k = 1;
  for (int i = 1; i < seq->count; i++)
  {
    bool last_point = (i == seq->count - 1);
    const TInstant *inst = tsequence_inst_n(seq, i);
    const POINT2D *pt = datum_get_point2d_p(tinstant_value(inst));

    /* Don't drop points if we are running short of points */
    if (seq->count - i > min_points - k)
    {
      if (tolerance > 0.0)
      {
        /* Only drop points that are within our tolerance */
        dsq = distance2d_sqr_pt_pt(last, pt);
        /* Allow any point but the last one to be dropped */
        if (!last_point && dsq <= tolsq)
          continue;
      }
      else
      {
        /* At tolerance zero, only skip exact dupes */
        if (FP_EQUALS(pt->x, last->x) && FP_EQUALS(pt->y, last->y))
          continue;
      }

      /* Got to last point, and it's not very different from
       * the point that preceded it. We want to keep the last
       * point, not the second-to-last one, so we pull our write
       * index back one value */
      if (last_point && k > 1 && tolerance > 0.0 && dsq <= tolsq)
      {
        k--;
      }
    }

    /* Save the point */
    instants[k++] = inst;
    last = pt;
  }
  /* Construct the result */
  TSequence *result = tsequence_make(instants, k, seq->period.lower_inc,
    seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  pfree(instants);
  return result;
}

/*
 * Returns a temporal point with consecutive equal points removed.
 * Equality test on x and y dimensions of input.
 */
TSequenceSet *
tpointseqset_remove_repeated_points(const TSequenceSet *ts, double tolerance,
  int min_points)
{
  const TSequence * seq;

  /* Singleton sequence set */
  if (ts->count == 1)
  {
    seq = tsequenceset_seq_n(ts, 0);
    TSequence *seq1 = tpointseq_remove_repeated_points(seq, tolerance,
      min_points);
    TSequenceSet *result = tsequence_to_tsequenceset(seq1);
    pfree(seq1);
    return result;
  }

  /* No-op on short inputs */
  if (ts->totalcount <= min_points)
    return tsequenceset_copy(ts);

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  int npoints = 0;
  for (int i = 0; i < ts->count; i++)
  {
    seq = tsequenceset_seq_n(ts, i);
    /* Don't drop sequences if we are running short of points */
    if (ts->totalcount - npoints > min_points)
    {
      /* Minimum number of points set to 2 */
      sequences[i] = tpointseq_remove_repeated_points(seq, tolerance, 2);
      npoints += sequences[i]->count;
    }
    else
    {
      /* Save the sequence */
      sequences[i] = tsequence_copy(seq);
    }
  }
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/*
 * Returns a temporal point with consecutive equal points removed.
 * Equality test on x and y dimensions of input.
 */
Temporal *
tpoint_remove_repeated_points(const Temporal *temp, double tolerance,
  int min_points)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tpointinstset_remove_repeated_points((TInstantSet *) temp,
      tolerance, min_points);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tpointseq_remove_repeated_points((TSequence *) temp,
      tolerance, min_points);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tpointseqset_remove_repeated_points((TSequenceSet *) temp,
      tolerance, min_points);
  return result;
}

/*****************************************************************************
 * Affine functions
 *****************************************************************************/

/**
 * Affine transform a temporal point.
 */
TInstant *
tpointinst_affine(TInstant *inst, const AFFINE *a)
{
  int srid = tpointinst_srid(inst);
  Datum value = tinstant_value(inst);
  LWPOINT *lwpoint;
  if (MOBDB_FLAGS_GET_Z(inst->flags))
  {
    POINT3DZ p3d = datum_get_point3dz(value);
    double x = p3d.x;
    double y = p3d.y;
    double z = p3d.z;
    p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
    p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
    p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
    lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
  }
  else
  {
    POINT2D p2d = datum_get_point2d(value);
    double x = p2d.x;
    double y = p2d.y;
    p2d.x = a->afac * x + a->bfac * y + a->xoff;
    p2d.y = a->dfac * x + a->efac * y + a->yoff;
    lwpoint = lwpoint_make2d(srid, p2d.x, p2d.y);
  }
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  TInstant *result = tinstant_make(PointerGetDatum(gs), inst->t,
    type_oid(T_GEOMETRY));
  lwpoint_free(lwpoint);
  pfree(gs);
  return result;
}

/**
 * Affine transform a temporal point.
 */
TInstantSet *
tpointinstset_affine(TInstantSet *ti, const AFFINE *a)
{
  int srid = tpointinstset_srid(ti);
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);
    LWPOINT *lwpoint;
    if (MOBDB_FLAGS_GET_Z(ti->flags))
    {
      POINT3DZ p3d = datum_get_point3dz(value);
      double x = p3d.x;
      double y = p3d.y;
      double z = p3d.z;
      p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
      p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
      p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
      lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
    }
    else
    {
      POINT2D p2d = datum_get_point2d(value);
      double x = p2d.x;
      double y = p2d.y;
      p2d.x = a->afac * x + a->bfac * y + a->xoff;
      p2d.y = a->dfac * x + a->efac * y + a->yoff;
      lwpoint = lwpoint_make2d(srid, p2d.x, p2d.y);
    }
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    instants[i] = tinstant_make(PointerGetDatum(gs), inst->t,
      type_oid(T_GEOMETRY));
    lwpoint_free(lwpoint);
    pfree(gs);
  }
  TInstantSet *result = tinstantset_make_free(instants, ti->count, MERGE_NO);
  return result;
}

/**
 * Affine transform a temporal point.
 */
TSequence *
tpointseq_affine(const TSequence *seq, const AFFINE *a)
{
  int srid = tpointseq_srid(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    Datum value = tinstant_value(inst);
    LWPOINT *lwpoint;
    if (MOBDB_FLAGS_GET_Z(seq->flags))
    {
      POINT3DZ p3d = datum_get_point3dz(value);
      double x = p3d.x;
      double y = p3d.y;
      double z = p3d.z;
      p3d.x = a->afac * x + a->bfac * y + a->cfac * z + a->xoff;
      p3d.y = a->dfac * x + a->efac * y + a->ffac * z + a->yoff;
      p3d.z = a->gfac * x + a->hfac * y + a->ifac * z + a->zoff;
      lwpoint = lwpoint_make3dz(srid, p3d.x, p3d.y, p3d.z);
    }
    else
    {
      POINT2D p2d = datum_get_point2d(value);
      double x = p2d.x;
      double y = p2d.y;
      p2d.x = a->afac * x + a->bfac * y + a->xoff;
      p2d.y = a->dfac * x + a->efac * y + a->yoff;
      lwpoint = lwpoint_make2d(srid, p2d.x, p2d.y);
    }
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    instants[i] = tinstant_make(PointerGetDatum(gs), inst->t,
      type_oid(T_GEOMETRY));
    lwpoint_free(lwpoint);
    pfree(gs);
  }
  /* Construct the result */
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
}

/**
 * Affine transform a temporal point.
 *
 * @param[in] ts Temporal point
 * @param[in] a Affine transformation
 */
TSequenceSet *
tpointseqset_affine(const TSequenceSet *ts, const AFFINE *a)
{
  /* Singleton sequence set */
  if (ts->count == 1)
  {
    TSequence *seq = tpointseq_affine(tsequenceset_seq_n(ts, 0), a);
    TSequenceSet *result = tsequence_to_tsequenceset(seq);
    pfree(seq);
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    sequences[i] = tpointseq_affine(tsequenceset_seq_n(ts, i), a);
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/**
 * Affine transform a temporal point.
 */
Temporal *
tpoint_affine(const Temporal *temp, const AFFINE *a)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tpointinst_affine((TInstant *) temp, a);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tpointinstset_affine((TInstantSet *) temp, a);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tpointseq_affine((TSequence *) temp, a);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tpointseqset_affine((TSequenceSet *) temp, a);
  return result;
}

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

/*
 * Stick a temporal point to the given gridspec.
 */
TInstant *
tpointinst_grid(const TInstant *inst, const gridspec *grid)
{
  bool has_z = MOBDB_FLAGS_GET_Z(inst->flags);
  if (grid->xsize == 0 && grid->ysize == 0 && (has_z ? grid->zsize == 0 : 1))
    return tinstant_copy(inst);

  int srid = tpointinst_srid(inst);
  Datum value = tinstant_value(inst);

  /* Read and round point */
  POINT4D p;
  datum_get_point4d(&p, value);
  double x, y, z = 0;
  x = rint((p.x - grid->ipx) / grid->xsize) * grid->xsize + grid->ipx;
  y = rint((p.y - grid->ipy) / grid->ysize) * grid->ysize + grid->ipy;
  if (has_z)
    z = rint((p.z - grid->ipz) / grid->zsize) * grid->zsize + grid->ipz;

  /* Write rounded values into the next instant */
  LWPOINT *lwpoint = has_z ?
    lwpoint_make3dz(srid, x, y, z) : lwpoint_make2d(srid, x, y);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
  lwpoint_free(lwpoint);

  /* Construct the result */
  TInstant *result = tinstant_make(PointerGetDatum(gs), inst->t,
    type_oid(T_GEOMETRY));
  lwpoint_free(lwpoint);
  pfree(gs);
  return result;
}

/*
 * Stick a temporal point to the given gridspec.
 */
TInstantSet *
tpointinstset_grid(const TInstantSet *ti, const gridspec *grid)
{
  bool has_z = MOBDB_FLAGS_GET_Z(ti->flags);
  int srid = tpointinstset_srid(ti);
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    POINT4D p, p_out;
    double x, y, z = 0;
    const TInstant *inst = tinstantset_inst_n(ti, i);
    Datum value = tinstant_value(inst);

    /* Read and round point */
    datum_get_point4d(&p, value);

    if (grid->xsize > 0)
      x = rint((p.x - grid->ipx) / grid->xsize) * grid->xsize + grid->ipx;

    if (grid->ysize > 0)
      y = rint((p.y - grid->ipy) / grid->ysize) * grid->ysize + grid->ipy;

    if (has_z && grid->zsize > 0)
      z = rint((p.z - grid->ipz) / grid->zsize) * grid->zsize + grid->ipz;

    /* Skip duplicates */
    if (i > 1 && p_out.x == x && p_out.y == y && (has_z ? p_out.z == z : 1))
      continue;

    /* Write rounded values into the next instant */
    LWPOINT *lwpoint = has_z ?
      lwpoint_make3dz(srid, x, y, z) : lwpoint_make2d(srid, x, y);
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    instants[k++] = tinstant_make(PointerGetDatum(gs), inst->t,
      type_oid(T_GEOMETRY));
    lwpoint_free(lwpoint);
    pfree(gs);
    memcpy(&p_out, &p, sizeof(POINT4D));
  }
  /* Construct the result */
  return tinstantset_make_free(instants, k, MERGE_NO);
}

/*
 * Stick a temporal point to the given gridspec.
 */
TSequence *
tpointseq_grid(const TSequence *seq, const gridspec *grid)
{
  bool has_z = MOBDB_FLAGS_GET_Z(seq->flags);
  int srid = tpointseq_srid(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int k = 0;
  for (int i = 0; i < seq->count; i++)
  {
    POINT4D p, p_out;
    double x, y, z = 0;
    const TInstant *inst = tsequence_inst_n(seq, i);
    Datum value = tinstant_value(inst);

    /* Read and round point */
    datum_get_point4d(&p, value);

    if (grid->xsize > 0)
      x = rint((p.x - grid->ipx) / grid->xsize) * grid->xsize + grid->ipx;

    if (grid->ysize > 0)
      y = rint((p.y - grid->ipy) / grid->ysize) * grid->ysize + grid->ipy;

    if (has_z && grid->zsize > 0)
      z = rint((p.z - grid->ipz) / grid->zsize) * grid->zsize + grid->ipz;

    /* Skip duplicates */
    if (i > 1 && p_out.x == x && p_out.y == y && (has_z ? p_out.z == z : 1))
      continue;

    /* Write rounded values into the next instant */
    LWPOINT *lwpoint = has_z ?
      lwpoint_make3dz(srid, x, y, z) : lwpoint_make2d(srid, x, y);
    GSERIALIZED *gs = geo_serialize((LWGEOM *) lwpoint);
    instants[k++] = tinstant_make(PointerGetDatum(gs), inst->t,
      type_oid(T_GEOMETRY));
    lwpoint_free(lwpoint);
    pfree(gs);
    memcpy(&p_out, &p, sizeof(POINT4D));
  }
  /* Construct the result */
  return tsequence_make_free(instants, k, seq->period.lower_inc,
    k > 1 ? seq->period.upper_inc : true, MOBDB_FLAGS_GET_LINEAR(seq->flags),
    NORMALIZE);
}

/**
 * Stick a temporal point to the given gridspec.
 */
TSequenceSet *
tpointseqset_grid(const TSequenceSet *ts, const gridspec *grid)
{
  /* Singleton sequence set */
  if (ts->count == 1)
  {
    TSequence *seq = tpointseq_grid(tsequenceset_seq_n(ts, 0), grid);
    TSequenceSet *result = tsequence_to_tsequenceset(seq);
    pfree(seq);
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
    sequences[i] = tpointseq_grid(tsequenceset_seq_n(ts, i), grid);
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}


/**
 * Stick a temporal point to the given gridspec.
 *
 * Only the x, y, and possible z dimensions are gridded, the timestamp is
 * kept unmodified. Two consecutive instants falling on the same grid cell
 * are collapsed into one single instant.
 */
Temporal *
tpoint_grid(const Temporal *temp, const gridspec *grid)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tpointinst_grid((TInstant *) temp, grid);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tpointinstset_grid((TInstantSet *) temp, grid);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tpointseq_grid((TSequence *) temp, grid);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tpointseqset_grid((TSequenceSet *) temp, grid);
  return result;
}

/*****************************************************************************/

/**
 * Transform a temporal point into vector tile coordinate space.
 */
Temporal *
tpoint_mvt(const Temporal *tpoint, const STBOX *box, uint32_t extent,
  uint32_t buffer, bool clip_geom)
{
  AFFINE affine = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  gridspec grid = {0, 0, 0, 0, 1, 1, 0, 0};
  double width = box->xmax - box->xmin;
  double height = box->ymax - box->ymin;
  double resx, resy, res, fx, fy;

  resx = width / extent;
  resy = height / extent;
  res = (resx < resy ? resx : resy)/2;
  fx = extent / width;
  fy = -(extent / height);

  /* Remove all non-essential points (under the output resolution) */
  Temporal *tpoint1 = tpoint_remove_repeated_points(tpoint, res, 2);
  /* Epsilon speed is not taken into account, i.e., parameter set to 0 */
  Temporal *tpoint2 = tpoint_simplify_internal(tpoint1, res, 0);

  /* If geometry has disappeared, you're done */
  if (tpoint2 == NULL)
    return NULL;

  /* Transform to tile coordinate space */
  affine.afac = fx;
  affine.efac = fy;
  affine.ifac = 1;
  affine.xoff = -box->xmin * fx;
  affine.yoff = -box->ymax * fy;
  Temporal *tpoint3 = tpoint_affine(tpoint2, &affine);

  /* Snap to integer precision, removing duplicate points */
  Temporal *tpoint4 = tpoint_grid(tpoint3, &grid);

  pfree(tpoint1); pfree(tpoint2); pfree(tpoint3);
  return tpoint4;
}

PG_FUNCTION_INFO_V1(AsMVTGeom);
/**
 * Transform to tpoint to Mapbox Vector Tile format
 */
Datum
AsMVTGeom(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  if (PG_ARGISNULL(1))
  {
    elog(ERROR, "%s: Geometric bounds cannot be null", __func__);
    PG_RETURN_NULL();
  }
  STBOX *bounds = PG_GETARG_STBOX_P(1);
  if (bounds->xmax - bounds->xmin <= 0 || bounds->ymax - bounds->ymin <= 0)
  {
    elog(ERROR, "%s: Geometric bounds are too small", __func__);
    PG_RETURN_NULL();
  }

  int32_t extent = PG_ARGISNULL(2) ? 4096 : PG_GETARG_INT32(2);
  if (extent <= 0)
  {
    elog(ERROR, "%s: Extent must be greater than 0", __func__);
    PG_RETURN_NULL();
  }

  int32_t buffer = PG_ARGISNULL(3) ? 256 : PG_GETARG_INT32(3);
  bool clip_geom = PG_ARGISNULL(4) ? true : PG_GETARG_BOOL(4);

  Temporal *temp = PG_GETARG_TEMPORAL(0);

  /* Clip temporal point immediately, to reduce the number of instants to
   * be processed. In PostGIS this is done at the last step */

  Temporal *temp1;
  if (clip_geom)
  {
    temp1 = tpoint_at_stbox_internal(temp, bounds);
    if (temp1 == NULL)
    {
      PG_FREE_IF_COPY(temp, 0);
      PG_RETURN_NULL();
    }
  }
  else
    temp1 = temp;

  /* Bounding box test to drop geometries smaller than the resolution */
  STBOX box;
  temporal_bbox(&box, temp1);
  double tpoint_width = box.xmax - box.xmin;
  double tpoint_height = box.ymax - box.ymin;
  /* We use half of the square height and width as limit: We use this
   * and not area so it works properly with lines */
  double bounds_width = ((bounds->xmax - bounds->xmin) / extent) / 2.0;
  double bounds_height = ((bounds->ymax - bounds->ymin) / extent) / 2.0;
  if (tpoint_width < bounds_width && tpoint_height < bounds_height)
  {
    if (clip_geom)
      pfree(temp1);
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_NULL();
  }

  Temporal *result = tpoint_mvt(temp1, bounds, extent, buffer, clip_geom);

  if (clip_geom)
    pfree(temp1);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
