/*****************************************************************************
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
 * @brief Implementation of the Gauss-Krueger projection used in Secondo.
 *
 * @note This projection does not correspond to any standard projection in
 * http://www.epsg.org/. This projection is provided to enable the comparison
 * of MobilityDB and Secondo.
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/meos_catalog.h"
#include "general/temporaltypes.h"
#include "general/lifting.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_point/postgis.h"


double Pi   = 3.1415926535897932384626433832795028841971693993751058209749445923078164;
double awgs = 6378137.0;
double bwgs =  6356752.314;
double abes = 6377397.155;     /* Bessel Semi-Major Axis = Equatorial Radius in meters */
double bbes = 6356078.962;     /* Bessel Semi-Minor Axis = Polar Radius in meters */
double cbes = 111120.6196;     /* Bessel latitude to Gauss-Krueger meters */
double dx   = -585.7;          /* Translation Parameter 1 */
double dy   = -87.0;           /* Translation Parameter 2 */
double dz   = -409.2;          /* Translation Parameter 3 */
double rotx = 2.540423689E-6;  /* Rotation Parameter 1 */
double roty = 7.514612057E-7;  /* Rotation Parameter 2 */
double rotz = -1.368144208E-5; /* Rotation Parameter 3 */
double sc   = 0.99999122;      /* Scaling Factor */
double h1   = 0;
double eqwgs = 0;
double eqbes = 0;
double MDC = 2.0;    /* standard in Hagen, zone=2 */

/**
 * @brief Transform to Gauss-Krueger projection
 */
static POINT2D
BesselBLToGaussKrueger(double b, double ll)
{
  POINT2D result;
  double l0 = 3.0 * MDC;
  l0 = Pi * l0 / 180.0;
  double l = ll - l0;
  double k = cos(b);
  double t = sin(b) / k;
  double eq = eqbes;
  double Vq = 1.0 + eq * k * k;
  double v = sqrt(Vq);
  double Ng = abes * abes / (bbes * v);
  double nk =(abes - bbes) / (abes + bbes);
  double X = ((Ng * t * k * k * l * l) / 2) +
    ((Ng * t * (9 * Vq - t * t - 4) * k * k * k * k * l * l * l * l) / 24);
  double gg = b + (((-3.0 * nk / 2.0) + (9.0 * nk * nk * nk / 16.0)) *
    sin(2 * b) + 15 * nk * nk * sin(4 * b) / 16 - 35 * nk * nk * nk * sin(6 * b) / 48);
  double SS = gg * 180.0 * cbes / Pi;
  double Ho = (SS + X);
  double Y = Ng * k * l + Ng * (Vq - t * t) * k * k * k * l * l * l / 6 + Ng *
    (5 - 18 * t * t + t * t * t * t) * k * k * k * k * k * l * l * l * l * l / 120;
  double kk = 500000;
  double RVV = MDC;
  double Re = RVV * 1000000.0 + kk + Y;
  result.x = Re;
  result.y = Ho;
  return result;
}

/**
 * @brief Perform Helmert Transformation
 */
static POINT3D
HelmertTransformation(double x, double y, double z)
{
  POINT3D p;
  p.x = dx + (sc * (1 * x + rotz * y - roty * z));
  p.y = dy + (sc * (-rotz * x + 1 * y + rotx * z));
  p.z = dz + (sc * (roty * x - rotx * y + 1 * z));
  return p;
}

/**
 *
 */
static double
newF(double f, double x, double y, double p)
{
  double zw;
  double nnq;
  zw = abes / sqrt(1 - eqbes * sin(f) * sin(f));
  nnq = 1 - (eqbes * zw / (sqrt(x * x + y * y) / cos(f)));
  return (atan(p / nnq));
}

/**
 *
 */
static POINT3D
BLRauenberg (double x, double y, double z)
{
  POINT3D result;
  double f = Pi * 50 / 180.0;
  double p = z / sqrt(x * x + y * y);
  double f1, f2;
  do
  {
    f1 = newF(f, x, y, p);
    f2 = f;
    f = f1;
  } while ((fabs(f2 - f1) >= (10E-10)));

  result.x = f;
  result.y = atan(y / x);
  result.z = sqrt(x * x + y * y) / cos(f1) -
    (abes / sqrt(1 - eqbes * sin(f1) * sin(f1)));
  return result;
}

/**
 * @brief Get Datum from 2D point
 */
static Datum
point2d_get_datum(const POINT2D *p2d)
{
  LWPOINT *lwpoint = lwpoint_make2d(4326, p2d->x, p2d->y);
  GSERIALIZED *result = geo_serialize((LWGEOM *) lwpoint);
  return PointerGetDatum(result);
}

/**
 * @brief Transform a point into the Gauss-Kruger projection used in Secondo
 */
static Datum
gk(Datum point)
{
  eqwgs = (awgs * awgs - bwgs * bwgs) / (awgs * awgs);
  eqbes = (abes * abes - bbes * bbes) / (abes * abes);
  const POINT2D *p2d = DATUM_POINT2D_P(point);
  POINT2D result;
  double x = p2d->x;
  double y = p2d->y;
  double a = (x / 180) * Pi;
  double b = (y / 180) * Pi;
  double l1 = a;
  double b1 = b;

  a = awgs;

  double eq = eqwgs;
  double N = a / sqrt(1 - eq * sin(b1) * sin(b1));
  double Xq = (N + h1) * cos(b1) * cos(l1);
  double Yq = (N + h1) * cos(b1) * sin(l1);
  double Zq = ((1 - eq) * N + h1) * sin(b1);

  POINT3D p = HelmertTransformation(Xq, Yq, Zq);
  double X = p.x;
  double Y = p.y;
  double Z = p.z;

  p = BLRauenberg(X, Y, Z);
  double b2 = p.x;
  double l2 = p.y;
  result = BesselBLToGaussKrueger(b2, l2);
  return point2d_get_datum(&result);
}

/**
 * @brief Transform a geometry into the Gauss-Kruger projection used in Secondo
 */
GSERIALIZED *
geometry_transform_gk(const GSERIALIZED *gs)
{
  GSERIALIZED *result = NULL; /* keep compiler quiet */
  int geotype = gserialized_get_type(gs);
  if (geotype == POINTTYPE)
  {
    LWPOINT *lwpoint;
    if (gserialized_is_empty(gs))
      lwpoint = lwpoint_construct_empty(0, false, false);
    else
    {
      const POINT2D *p2d = GSERIALIZED_POINT2D_P(gs);
      Datum geom = gk(point2d_get_datum(p2d));
      p2d  = DATUM_POINT2D_P(geom);
      lwpoint = lwpoint_make2d(4326, p2d->x, p2d->y);
    }
    result = geo_serialize((LWGEOM *)lwpoint);
    lwpoint_free(lwpoint);
  }
  else if (geotype == LINETYPE)
  {
    LWLINE *line;
    if (gserialized_is_empty(gs))
    {
      line = lwline_construct_empty(0, false, false);
      result = geo_serialize((LWGEOM *) line);
    }
    else
    {
      LWPOINT *lwpoint = NULL;
      line = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
      uint32_t numPoints = line->points->npoints;
      LWPOINT **points = palloc(sizeof(LWPOINT *) * numPoints);
      for (uint32_t i = 0; i < numPoints; i++)
      {
        lwpoint = lwline_get_lwpoint(line, i);
        Datum point2d_datum = PointerGetDatum(geo_serialize((LWGEOM *) lwpoint));
        Datum geom = gk(point2d_datum);
        const POINT2D *p2d  = DATUM_POINT2D_P(geom);
        points[i] = lwpoint_make2d(4326, p2d->x, p2d->y);
      }

      line = lwline_from_ptarray(4326, numPoints, points);
      result = geo_serialize((LWGEOM *) line);
      lwline_free(line); lwpoint_free(lwpoint);
      for (uint32_t i = 0; i < numPoints; i++)
        lwpoint_free(points[i]);
      pfree(points);
    }
  }
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Component geometry/geography must be of type Point(Z)M or LineString")));

  return result;
}

/**
 * @brief Transform a temporal point into the Gauss-Krueger projection used in
 * Secondo
 */
Temporal *
tgeompoint_transform_gk(const Temporal *temp)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &gk;
  lfinfo.numparam = 0;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

PGDLLEXPORT Datum Geometry_transform_gk(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geometry_transform_gk);
/**
 * @brief Transform a geometry into the Gauss-Krueger projection used in Secondo
 */
Datum
Geometry_transform_gk(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = NULL;
  result = geometry_transform_gk(gs);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tgeompoint_transform_gk(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_transform_gk);
/**
 * @brief Transform a temporal point into the Gauss-Krueger projection used in
 * Secondo
 */
Datum
Tgeompoint_transform_gk(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_transform_gk(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
