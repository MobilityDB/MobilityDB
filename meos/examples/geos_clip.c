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
 * @brief A program that shows the connection of MEOS and GEOS. It computes the
 * the intersection of a MEOS temporal point and a prepared GEOS polygon
 * @note This program is based on the GEOS C API example 2 at the address
 * https://github.com/libgeos/geos/blob/main/examples/capi_prepared.c
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geos_clip geos_clip.c -L/usr/local/lib -lmeos -lgeos_c
 * @endcode
*/

/* To print to stdout */
#include <stdio.h>
#include <stdlib.h>

/* Input the GEOS headers */
#include <geos_c.h>

/* Input the MEOS headers */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>

int main()
{
  /* Initialize MEOS */
  meos_initialize();

  /* Retrieve the per-thread GEOS context handle owned by MEOS.  All GEOS
   * operations use the reentrant GEOSXxx_r API with this handle so the
   * example is thread-safe and consistent with the MEOS spatial helpers. */
  GEOSContextHandle_t ctx = geos_get_context();

  /* A (normalized) temporal point */
  const char *tpoint_wkt = "[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, "
    "Point(3 3)@2000-01-04, Point(4 4)@2000-01-06, Point(4.5 4.5)@2000-01-08, Point(6 6)@2000-01-10]";
  /* Read the WKT into a temporal point object */
  TSequence *tpoint = (TSequence *) tgeompoint_in(tpoint_wkt);

  /* Check for parse success */
  if (! tpoint)
  {
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* A polygon (actually a rectangle) */
  const char *poly_wkt = "POLYGON ((3 3,3 5,5 5,5 3,3 3))";
  /* Read the WKT into a geometry object */
  GEOSWKTReader *reader = GEOSWKTReader_create_r(ctx);
  GEOSGeometry *poly = GEOSWKTReader_read_r(ctx, reader, poly_wkt);
  GEOSWKTReader_destroy_r(ctx, reader);

  /* Check for parse success */
  if (! poly)
  {
    meos_finalize();
    return EXIT_FAILURE;
  }

  /* Prepare the geometry */
  const GEOSPreparedGeometry *prep_poly = GEOSPrepare_r(ctx, poly);

  /* Place to hold points to output */
  GEOSGeometry **geoms = (GEOSGeometry **) malloc(sizeof(GEOSGeometry *) *
    tpoint->count);
  size_t ngeoms = 0;

  /*
  * Test all the points in the temporal point
  * and only keep those that intersect the polygon
  */
  const TInstant *inst1 = TSEQUENCE_INST_N(tpoint, 0);
  const POINT2D *pt1 = DATUM_POINT2D_P(&inst1->value);
  int i;
  for (i = 1; i < tpoint->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(tpoint, i);
    const POINT2D *pt2 = DATUM_POINT2D_P(&inst2->value);
    /* Make a GEOS point */
    GEOSCoordSequence *seq = GEOSCoordSeq_create_r(ctx, 2, 2);
    GEOSCoordSeq_setXY_r(ctx, seq, 0, pt1->x, pt1->y);
    GEOSCoordSeq_setXY_r(ctx, seq, 1, pt2->x, pt2->y);
    GEOSGeometry *line = GEOSGeom_createLineString_r(ctx, seq);
    /* Check if the point and polygon intersect */
    if (GEOSPreparedIntersects_r(ctx, prep_poly, line))
    {
      /* Save the ones that do */
      GEOSGeometry *clip = GEOSIntersection_r(ctx, poly, line);
      geoms[ngeoms++] = clip;
    }
    /* Clean up the ones that don't */
    GEOSGeom_destroy_r(ctx, line);
  }

  /* Put the successful geoms inside a geometry for WKT output */
  GEOSGeometry *result = GEOSGeom_createCollection_r(ctx,
    GEOS_GEOMETRYCOLLECTION, geoms, ngeoms);

  /*
  * The GEOSGeom_createCollection() only takes ownership of the
  * geometries, not the array container, so we can free the container
  * now
  */
  free(geoms);

  /* Convert result to WKT */
  GEOSWKTWriter *writer = GEOSWKTWriter_create_r(ctx);
  /* Trim trailing zeros off output */
  GEOSWKTWriter_setTrim_r(ctx, writer, 1);
  GEOSWKTWriter_setRoundingPrecision_r(ctx, writer, 3);
  char *wkt_result = GEOSWKTWriter_write_r(ctx, writer, result);
  GEOSWKTWriter_destroy_r(ctx, writer);

  /* Print answer */
  printf("Input Temporal Point:\n");
  printf("%s\n\n", tpoint_wkt);
  printf("Input Polygon:\n");
  printf("%s\n\n", poly_wkt);
  printf("Segments clipped:\n");
  printf("%s\n\n", wkt_result);

  /* Clean up everything we allocated */
  free(tpoint);
  GEOSPreparedGeom_destroy_r(ctx, prep_poly);
  GEOSGeom_destroy_r(ctx, poly);
  GEOSGeom_destroy_r(ctx, result);
  GEOSFree_r(ctx, wkt_result);

  /* Finalize MEOS, which releases the per-thread GEOS context */
  meos_finalize();

  /* Return */
  return EXIT_SUCCESS;
}
