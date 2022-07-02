/*****************************************************************************
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
 * @brief Spatial functions for PostGIS geography.
 *
 * These functions are supposed to be included in a forthcoming version of
 * PostGIS, to be proposed as a PR. This still remains to be done.
 * These functions are not needed in MobilityDB.
 */

#include "point/geography_funcs.h"

/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/array.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeom_pg.h>
#include <lwgeodetic_tree.h>
/* MobilityDB */
#include "point/tpoint_spatialfuncs.h"

/***********************************************************************
 * Closest point and closest line functions for geographies.
 ***********************************************************************/

static LWGEOM *
geography_tree_closestpoint(const GSERIALIZED* g1, const GSERIALIZED* g2, double threshold)
{
  CIRC_NODE* circ_tree1 = NULL;
  CIRC_NODE* circ_tree2 = NULL;
  LWGEOM* lwgeom1 = NULL;
  LWGEOM* lwgeom2 = NULL;
  double min_dist = FLT_MAX;
  double max_dist = FLT_MAX;
  GEOGRAPHIC_POINT closest1, closest2;
  LWGEOM *result;
  POINT4D p;

  lwgeom1 = lwgeom_from_gserialized(g1);
  lwgeom2 = lwgeom_from_gserialized(g2);
  circ_tree1 = lwgeom_calculate_circ_tree(lwgeom1);
  circ_tree2 = lwgeom_calculate_circ_tree(lwgeom2);

  /* Quietly decrease the threshold just a little to avoid cases where */
  /* the actual spheroid distance is larger than the sphere distance */
  /* causing the return value to be larger than the threshold value */
  // double threshold_radians = 0.95 * threshold / spheroid->radius;
  double threshold_radians = threshold / WGS84_RADIUS;

  circ_tree_distance_tree_internal(circ_tree1, circ_tree2, threshold_radians,
    &min_dist, &max_dist, &closest1, &closest2);

  p.x = rad2deg(closest1.lon);
  p.y = rad2deg(closest1.lat);
  result = (LWGEOM *)lwpoint_make2d(gserialized_get_srid(g1), p.x, p.y);

  circ_tree_free(circ_tree1);
  circ_tree_free(circ_tree2);
  lwgeom_free(lwgeom1);
  lwgeom_free(lwgeom2);
  return result;
}

/**
Returns the point in first input geography that is closest to the second input geography in 2d
*/

PG_FUNCTION_INFO_V1(geography_closestpoint);
Datum geography_closestpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED* g1 = NULL;
  GSERIALIZED* g2 = NULL;
  LWGEOM *point;
  GSERIALIZED* result;

  /* Get our geography objects loaded into memory. */
  g1 = PG_GETARG_GSERIALIZED_P(0);
  g2 = PG_GETARG_GSERIALIZED_P(1);

  /* Return NULL on empty arguments. */
  if ( gserialized_is_empty(g1) || gserialized_is_empty(g2) )
  {
    PG_FREE_IF_COPY(g1, 0);
    PG_FREE_IF_COPY(g2, 1);
    PG_RETURN_NULL();
  }

  ensure_same_srid(gserialized_get_srid(g1), gserialized_get_srid(g2));

  point = geography_tree_closestpoint(g1, g2, FP_TOLERANCE);

  if (lwgeom_is_empty(point))
    PG_RETURN_NULL();

  result = geography_serialize(point);
  lwgeom_free(point);

  PG_FREE_IF_COPY(g1, 0);
  PG_FREE_IF_COPY(g2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
Returns the point in first input geography that is closest to the second
input geography in 2d
*/
PG_FUNCTION_INFO_V1(geography_shortestline);
Datum geography_shortestline(PG_FUNCTION_ARGS)
{
  /* Get our geography objects loaded into memory. */
  GSERIALIZED *g1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *g2 = PG_GETARG_GSERIALIZED_P(1);

  /* Read calculation type */
  bool use_spheroid = true;
  if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
    use_spheroid = PG_GETARG_BOOL(2);

  GSERIALIZED *result = geography_shortestline_internal(g1, g2, use_spheroid);

  PG_FREE_IF_COPY(g1, 0);
  PG_FREE_IF_COPY(g2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/***********************************************************************
 * ST_LineSubstring for geographies
 ***********************************************************************/

static double
ptarray_length_sphere(const POINTARRAY *pa)
{
  GEOGRAPHIC_POINT a, b;
  POINT4D p;
  uint32_t i;
  double length = 0.0;

  /* Return zero on non-sensical inputs */
  if ( ! pa || pa->npoints < 2 )
    return 0.0;

  /* Initialize first point */
  getPoint4d_p(pa, 0, &p);
  geographic_point_init(p.x, p.y, &a);

  /* Loop and sum the length for each segment */
  for ( i = 1; i < pa->npoints; i++ )
  {
    getPoint4d_p(pa, i, &p);
    geographic_point_init(p.x, p.y, &b);
    /* Add this segment length to the total */
    length +=  sphere_distance(&a, &b);
  }
  return length;
}

static POINTARRAY *
geography_substring(POINTARRAY *ipa, double from, double to,
  double tolerance)
{
  POINTARRAY *dpa;
  POINT4D pt;
  POINT4D p1, p2;
  POINT3D q1, q2;
  GEOGRAPHIC_POINT g1, g2;
  int nsegs, i;
  double length, slength, tlength;
  int state = 0; /* 0 = before, 1 = inside */

  /*
   * Create a dynamic pointarray with an initial capacity
   * equal to full copy of input points
   */
  dpa = ptarray_construct_empty((char) FLAGS_GET_Z(ipa->flags),
    (char) FLAGS_GET_M(ipa->flags), ipa->npoints);

  /* Compute total line length */
  length = ptarray_length_sphere(ipa);

  /* Get 'from' and 'to' lengths */
  from = length * from;
  to = length * to;
  tlength = 0;
  getPoint4d_p(ipa, 0, &p1);
  geographic_point_init(p1.x, p1.y, &g1);
  nsegs = ipa->npoints - 1;
  for (i = 0; i < nsegs; i++)
  {
    double dseg;
    getPoint4d_p(ipa, (uint32_t) i+1, &p2);
    geographic_point_init(p2.x, p2.y, &g2);

    /* Find the length of this segment */
    slength = sphere_distance(&g1, &g2);

    /*
     * We are before requested start.
     */
    if (state == 0) /* before */
    {
      if (fabs ( from - ( tlength + slength ) ) <= tolerance)
      {
        /*
         * Second point is our start
         */
        ptarray_append_point(dpa, &p2, LW_FALSE);
        state = 1; /* we're inside now */
        goto END;
      }
      else if (fabs(from - tlength) <= tolerance)
      {
        /*
         * First point is our start
         */
        ptarray_append_point(dpa, &p1, LW_FALSE);
        /*
         * We're inside now, but will check
         * 'to' point as well
         */
        state = 1;
      }
      /*
       * Didn't reach the 'from' point,
       * nothing to do
       */
      else if (from > tlength + slength)
        goto END;
      else  /* tlength < from < tlength+slength */
      {
        /*
         * Our start is between first and second point
         */
        dseg = (from - tlength) / slength;
        geog2cart(&g1, &q1);
        geog2cart(&g2, &q2);
        interpolate_point4d_sphere(&q1, &q2, &p1, &p2, dseg, &pt);
        ptarray_append_point(dpa, &pt, LW_FALSE);
        /*
         * We're inside now, but will check 'to' point as well
         */
        state = 1;
      }
    }

    if (state == 1) /* inside */
    {
      /*
       * 'to' point is our second point.
       */
      if (fabs(to - ( tlength + slength ) ) <= tolerance )
      {
        ptarray_append_point(dpa, &p2, LW_FALSE);
        break; /* substring complete */
      }
      /*
       * 'to' point is our first point.
       * (should only happen if 'to' is 0)
       */
      else if (fabs(to - tlength) <= tolerance)
      {
        ptarray_append_point(dpa, &p1, LW_FALSE);
        break; /* substring complete */
      }
      /*
       * Didn't reach the 'end' point,
       * just copy second point
       */
      else if (to > tlength + slength)
      {
        ptarray_append_point(dpa, &p2, LW_FALSE);
        goto END;
      }
      /*
       * 'to' point falls on this segment
       * Interpolate and break.
       */
      else if (to < tlength + slength )
      {
        dseg = (to - tlength) / slength;
        geog2cart(&g1, &q1);
        geog2cart(&g2, &q2);
        interpolate_point4d_sphere(&q1, &q2, &p1, &p2, dseg, &pt);
        ptarray_append_point(dpa, &pt, LW_FALSE);
        break;
      }
    }
END:
    tlength += slength;
    memcpy(&p1, &p2, sizeof(POINT4D));
  }

  return dpa;
}

PG_FUNCTION_INFO_V1(geography_line_substring);
Datum geography_line_substring(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double from_fraction = PG_GETARG_FLOAT8(1);
  double to_fraction = PG_GETARG_FLOAT8(2);
  LWLINE *lwline;
  LWGEOM *lwresult;
  POINTARRAY* opa;
  GSERIALIZED *result;

  /* Return NULL on empty argument. */
  if ( gserialized_is_empty(gs) )
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  if ( from_fraction < 0 || from_fraction > 1 )
  {
    elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }
  if ( to_fraction < 0 || to_fraction > 1 )
  {
    elog(ERROR,"line_interpolate_point: 3rd arg isn't within [0,1]");
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }
  if ( from_fraction > to_fraction )
  {
    elog(ERROR, "2nd arg must be smaller then 3rd arg");
    PG_RETURN_NULL();
  }
  if ( gserialized_get_type(gs) != LINETYPE )
  {
    elog(ERROR,"line_substring: 1st arg isn't a line");
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  opa = geography_substring(lwline->points, from_fraction, to_fraction,
    FP_TOLERANCE);

  lwgeom_free(lwline_as_lwgeom(lwline));
  PG_FREE_IF_COPY(gs, 0);

  if (opa->npoints <= 1)
  {
    lwresult = lwpoint_as_lwgeom(lwpoint_construct(lwline->srid, NULL, opa));
  } else {
    lwresult = lwline_as_lwgeom(lwline_construct(lwline->srid, NULL, opa));
  }

  lwgeom_set_geodetic(lwresult, true);
  result = geography_serialize(lwresult);
  lwgeom_free(lwresult);

  PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Interpolate a point along a geographic line.
 ***********************************************************************/

static POINTARRAY *
geography_interpolate_points(const LWLINE *line,
  double length_fraction, const SPHEROID *s, char repeat)
{
  POINT4D pt;
  uint32_t i;
  uint32_t points_to_interpolate;
  uint32_t points_found = 0;
  double length;
  double length_fraction_increment = length_fraction;
  double length_fraction_consumed = 0;
  char has_z = (char) lwgeom_has_z(lwline_as_lwgeom(line));
  char has_m = (char) lwgeom_has_m(lwline_as_lwgeom(line));
  const POINTARRAY* ipa = line->points;
  POINTARRAY* opa;
  POINT4D p1, p2;
  POINT3D q1, q2;
  GEOGRAPHIC_POINT g1, g2;

  /* Empty.InterpolatePoint == Point Empty */
  if ( lwline_is_empty(line) )
  {
    return ptarray_construct_empty(has_z, has_m, 0);
  }

  /* If distance is one of the two extremes, return the point on that
   * end rather than doing any computations
   */
  if ( length_fraction == 0.0 || length_fraction == 1.0 )
  {
    if ( length_fraction == 0.0 )
      getPoint4d_p(ipa, 0, &pt);
    else
      getPoint4d_p(ipa, ipa->npoints-1, &pt);

    opa = ptarray_construct(has_z, has_m, 1);
    ptarray_set_point4d(opa, 0, &pt);

    return opa;
  }

  /* Interpolate points along the line */
  length = ptarray_length_spheroid(ipa, s);
  points_to_interpolate = repeat ? (uint32_t) floor(1 / length_fraction) : 1;
  opa = ptarray_construct(has_z, has_m, points_to_interpolate);

  getPoint4d_p(ipa, 0, &p1);
  geographic_point_init(p1.x, p1.y, &g1);
  for ( i = 0; i < ipa->npoints - 1 && points_found < points_to_interpolate; i++ )
  {
    getPoint4d_p(ipa, i+1, &p2);
    geographic_point_init(p2.x, p2.y, &g2);
    double segment_length_frac = spheroid_distance(&g1, &g2, s) / length;

    /* If our target distance is before the total length we've seen
     * so far. create a new point some distance down the current
     * segment.
     */
    while ( length_fraction < length_fraction_consumed + segment_length_frac && points_found < points_to_interpolate )
    {
      geog2cart(&g1, &q1);
      geog2cart(&g2, &q2);
      double segment_fraction = (length_fraction - length_fraction_consumed) / segment_length_frac;
      interpolate_point4d_sphere(&q1, &q2, &p1, &p2, segment_fraction, &pt);
      ptarray_set_point4d(opa, points_found++, &pt);
      length_fraction += length_fraction_increment;
    }

    length_fraction_consumed += segment_length_frac;

    p1 = p2;
    g1 = g2;
  }

  /* Return the last point on the line. This shouldn't happen, but
   * could if there's some floating point rounding errors. */
  if (points_found < points_to_interpolate) {
    getPoint4d_p(ipa, ipa->npoints - 1, &pt);
    ptarray_set_point4d(opa, points_found, &pt);
  }

  return opa;
}

PG_FUNCTION_INFO_V1(geography_line_interpolate_point);
Datum geography_line_interpolate_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double distance_fraction = PG_GETARG_FLOAT8(1);
  /* Read calculation type */
  bool use_spheroid = true;
  if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
    use_spheroid = PG_GETARG_BOOL(2);
  /* Read repeat mode */
  bool repeat = PG_NARGS() > 3 && PG_GETARG_BOOL(3);
  int srid = gserialized_get_srid(gs);
  LWLINE* lwline;
  LWGEOM* lwresult;
  POINTARRAY* opa;
  SPHEROID s;
  GSERIALIZED *result;

  /* Return NULL on empty argument. */
  if ( gserialized_is_empty(gs) )
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  if ( distance_fraction < 0 || distance_fraction > 1 )
  {
    elog(ERROR,"line_interpolate_point: 2nd arg isn't within [0,1]");
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  if ( gserialized_get_type(gs) != LINETYPE )
  {
    elog(ERROR,"line_interpolate_point: 1st arg isn't a line");
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(fcinfo, srid, &s);
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
  opa = geography_interpolate_points(lwline, distance_fraction, &s, repeat);

  lwgeom_free(lwline_as_lwgeom(lwline));
  PG_FREE_IF_COPY(gs, 0);

  if (opa->npoints <= 1)
  {
    lwresult = lwpoint_as_lwgeom(lwpoint_construct(srid, NULL, opa));
  } else {
    lwresult = lwmpoint_as_lwgeom(lwmpoint_construct(srid, opa));
  }

  lwgeom_set_geodetic(lwresult, true);
  result = geography_serialize(lwresult);
  lwgeom_free(lwresult);

  PG_RETURN_POINTER(result);
}

/***********************************************************************
 * Locate a point along a geographic line.
 ***********************************************************************/

static double
ptarray_locate_point_spheroid(const POINTARRAY *pa, const POINT4D *p4d,
  const SPHEROID *s, double tolerance, double *mindistout, POINT4D *proj4d)
{
  GEOGRAPHIC_EDGE e;
  GEOGRAPHIC_POINT a, b, nearest;
  POINT4D p1, p2;
  const POINT2D *p;
  POINT2D proj;
  uint32_t i, seg = 0;
  int use_sphere = (s->a == s->b ? 1 : 0);
  int hasz;
  double za = 0.0, zb = 0.0;
  double distance,
    length,   /* Used for computing lengths */
    seglength = 0, /* length of the segment where the closest point is located */
    partlength, /* length from the beginning of the point array to the closest point */
    totlength;  /* length of the point array */

  /* Initialize our point */
  geographic_point_init(p4d->x, p4d->y, &a);

  /* Handle point/point case here */
  if ( pa->npoints <= 1)
  {
    if ( pa->npoints == 1 )
    {
      p = getPoint2d_cp(pa, 0);
      geographic_point_init(p->x, p->y, &b);
      /* Sphere special case, axes equal */
      *mindistout = s->radius * sphere_distance(&a, &b);
      /* If close or greater than tolerance, get the real answer to be sure */
      if ( ! use_sphere || *mindistout > 0.95 * tolerance )
        *mindistout = spheroid_distance(&a, &b, s);
    }
    return 0.0;
  }

  /* Make result really big, so that everything will be smaller than it */
  distance = FLT_MAX;

  /* Initialize start of line */
  p = getPoint2d_cp(pa, 0);
  geographic_point_init(p->x, p->y, &(e.start));

  /* Iterate through the edges in our line */
  for ( i = 1; i < pa->npoints; i++ )
  {
    double d;
    p = getPoint2d_cp(pa, i);
    geographic_point_init(p->x, p->y, &(e.end));
    /* Get the spherical distance between point and edge */
    d = s->radius * edge_distance_to_point(&e, &a, &b);
    /* New shortest distance! Record this distance / location / segment */
    if ( d < distance )
    {
      distance = d;
      nearest = b;
      seg = i - 1;
    }
    /* We've gotten closer than the tolerance... */
    if ( d < tolerance )
    {
      /* Working on a sphere? The answer is correct, return */
      if ( use_sphere )
      {
        break;
      }
      /* Far enough past the tolerance that the spheroid calculation won't change things */
      else if ( d < tolerance * 0.95 )
      {
        break;
      }
      /* On a spheroid and near the tolerance? Confirm that we are *actually* closer than tolerance */
      else
      {
        d = spheroid_distance(&a, &nearest, s);
        /* Yes, closer than tolerance, return! */
        if ( d < tolerance )
          break;
      }
    }
    e.start = e.end;
  }

  if ( mindistout ) *mindistout = distance;

  /* See if we have a third dimension */
  hasz = (bool) FLAGS_GET_Z(pa->flags);

  /* Initialize first point of array */
  getPoint4d_p(pa, 0, &p1);
  geographic_point_init(p1.x, p1.y, &a);
  if ( hasz )
    za = p1.z;

  partlength = 0.0;
  totlength = 0.0;

  /* Loop and sum the length for each segment */
  for ( i = 1; i < pa->npoints; i++ )
  {
    getPoint4d_p(pa, i, &p1);
    geographic_point_init(p1.x, p1.y, &b);
    if ( hasz )
      zb = p1.z;

    /* Special sphere case */
    if ( s->a == s->b )
      length = s->radius * sphere_distance(&a, &b);
    /* Spheroid case */
    else
      length = spheroid_distance(&a, &b, s);

    /* Add in the vertical displacement if we're in 3D */
    if ( hasz )
      length = sqrt( (zb-za)*(zb-za) + length*length );

    /* Add this segment length to the total length */
    totlength += length;

    /* Add this segment length to the partial length */
    if (i - 1 < seg)
      partlength += length;
    else if (i - 1 == seg)
      /* Save segment length for computing the final value of partlength */
      seglength = length;

    /* B gets incremented in the next loop, so we save the value here */
    a = b;
    za = zb;
  }

  /* Copy nearest into 2D/4D holder */
  proj4d->x = proj.x = rad2deg(nearest.lon);
  proj4d->y = proj.y = rad2deg(nearest.lat);

  /* Compute distance from beginning of the segment to closest point */

  /* Start of the segment */
  getPoint4d_p(pa, seg, &p1);
  geographic_point_init(p1.x, p1.y, &a);

  /* Closest point */
  geographic_point_init(proj4d->x, proj4d->y, &b);

  /* Special sphere case */
  if ( s->a == s->b )
    length = s->radius * sphere_distance(&a, &b);
  /* Spheroid case */
  else
    length = spheroid_distance(&a, &b, s);

  if ( hasz )
  {
    /* Compute Z and M values for closest point */
    double f = length / seglength;
    getPoint4d_p(pa, seg + 1, &p2);
    proj4d->z = p1.z + ((p2.z - p1.z) * f);
    proj4d->m = p1.m + ((p2.m - p1.m) * f);
    /* Add in the vertical displacement if we're in 3D */
    za = p1.z;
    zb = proj4d->z;
    length = sqrt( (zb-za)*(zb-za) + length*length );
  }

  /* Add this segment length to the total */
  partlength += length;

  /* Location of any point on a zero-length line is 0 */
  /* See http://trac.osgeo.org/postgis/ticket/1772#comment:2 */
  if ( totlength == 0 )
    return 0;

  /* For robustness, force 1 when closest point == endpoint */
  p = getPoint2d_cp(pa, pa->npoints - 1);
  if ( (seg >= (pa->npoints-2)) && p2d_same(&proj, p) )
    return 1.0;

  return partlength / totlength;
}

PG_FUNCTION_INFO_V1(geography_line_locate_point);
Datum geography_line_locate_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs2 = PG_GETARG_GSERIALIZED_P(1);
  bool use_spheroid = true;
  /* Read our calculation type */
  if ( PG_NARGS() > 2 && ! PG_ARGISNULL(2) )
    use_spheroid = PG_GETARG_BOOL(2);
  double tolerance = FP_TOLERANCE;
  SPHEROID s;
  LWLINE *lwline;
  LWPOINT *lwpoint;
  POINTARRAY *pa;
  POINT4D p, p_proj;
  double ret;

  /* Return NULL on empty argument. */
  if ( gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
  {
    PG_FREE_IF_COPY(gs1, 0);
    PG_FREE_IF_COPY(gs2, 1);
    PG_RETURN_NULL();
  }

  /* Initialize spheroid */
  /* We currently cannot use the following statement since PROJ4 API is not
   * available directly to MobilityDB. */
  // spheroid_init_from_srid(fcinfo, gserialized_get_srid(gs1), &s);
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  if ( gserialized_get_type(gs1) != LINETYPE )
  {
    elog(ERROR,"line_locate_point: 1st arg isn't a line");
    PG_RETURN_NULL();
  }
  if ( gserialized_get_type(gs2) != POINTTYPE )
  {
    elog(ERROR,"line_locate_point: 2st arg isn't a point");
    PG_RETURN_NULL();
  }

  /* User requests spherical calculation, turn our spheroid into a sphere */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;
  else
    /* Initialize spheroid */
    /* We cannot use the following statement since PROJ4 API is not
     * available directly to MobilityDB. */
    // spheroid_init_from_srid(fcinfo, srid, &s);
    spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  ensure_same_srid(gserialized_get_srid(gs1), gserialized_get_srid(gs2));

  lwline = lwgeom_as_lwline(lwgeom_from_gserialized(gs1));
  lwpoint = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs2));

  pa = lwline->points;
  lwpoint_getPoint4d_p(lwpoint, &p);

  ret = ptarray_locate_point_spheroid(pa, &p, &s, tolerance, NULL, &p_proj);

  PG_RETURN_FLOAT8(ret);
}

/*****************************************************************************/
