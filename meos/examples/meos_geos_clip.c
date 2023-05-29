/**
 * @brief A program that shows the connection of MEOS and GEOS. It computes the
 * the intersection of a MEOS temporal point and a prepared GEOS polygon
 * @note This program is based on the GEOS C API example 2
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_geos_clip meos_geos_clip.c -L/usr/local/lib -lmeos -lgeos_c
 * @endcode
*/

/* To print to stdout */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* Input the GEOS headers */
#include <geos_c.h>

/* Input the MEOS headers */
#include <meos.h>
#include <meos_internal.h>

/*
* GEOS requires two message handlers to return
* error and notice message to the calling program.
*
*   typedef void(* GEOSMessageHandler) (const char *fmt,...)
*
* Here we stub out an example that just prints the
* messages to stdout.
*/
static void
geos_message_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

int main()
{
  /* Initialize MEOS */
  meos_initialize(NULL);

  /* Send notice and error messages to our stdout handler */
  initGEOS(geos_message_handler, geos_message_handler);

  /* A (normalized) temporal point */
  const char *tpoint_wkt = "[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03, "
    "Point(3 3)@2000-01-04, Point(4 4)@2000-01-06, Point(4.5 4.5)@2000-01-08, Point(6 6)@2000-01-10]";
  /* Read the WKT into a temporal point object */
  TSequence *tpoint = (TSequence *) tgeompoint_in(tpoint_wkt);

  /* Check for parse success */
  if (! tpoint)
  {
    meos_finalize();
    finishGEOS();
    return 1;
  }

  /* A polygon (actually a rectangle) */
  const char *poly_wkt = "POLYGON ((3 3,3 5,5 5,5 3,3 3))";
  /* Read the WKT into a geometry object */
  GEOSWKTReader *reader = GEOSWKTReader_create();
  GEOSGeometry *poly = GEOSWKTReader_read(reader, poly_wkt);
  GEOSWKTReader_destroy(reader);

  /* Check for parse success */
  if (! poly)
  {
    meos_finalize();
    finishGEOS();
    return 1;
  }

  /* Prepare the geometry */
  const GEOSPreparedGeometry *prep_poly = GEOSPrepare(poly);

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
    GEOSCoordSequence *seq = GEOSCoordSeq_create(2, 2);
    GEOSCoordSeq_setXY(seq, 0, pt1->x, pt1->y);
    GEOSCoordSeq_setXY(seq, 1, pt2->x, pt2->y);
    GEOSGeometry *line = GEOSGeom_createLineString(seq);
    /* Check if the point and polygon intersect */
    if (GEOSPreparedIntersects(prep_poly, line))
    {
      /* Save the ones that do */
      GEOSGeometry *clip = GEOSIntersection(poly, line);
      geoms[ngeoms++] = clip;
    }
    /* Clean up the ones that don't */
    GEOSGeom_destroy(line);
  }

  /* Put the successful geoms inside a geometry for WKT output */
  GEOSGeometry *result = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION,
    geoms, ngeoms);

  /*
  * The GEOSGeom_createCollection() only takes ownership of the
  * geometries, not the array container, so we can free the container
  * now
  */
  free(geoms);

  /* Convert result to WKT */
  GEOSWKTWriter *writer = GEOSWKTWriter_create();
  /* Trim trailing zeros off output */
  GEOSWKTWriter_setTrim(writer, 1);
  GEOSWKTWriter_setRoundingPrecision(writer, 3);
  char *wkt_result = GEOSWKTWriter_write(writer, result);
  GEOSWKTWriter_destroy(writer);

  /* Print answer */
  printf("Input Temporal Point:\n");
  printf("%s\n\n", tpoint_wkt);
  printf("Input Polygon:\n");
  printf("%s\n\n", poly_wkt);
  printf("Segments clipped:\n");
  printf("%s\n\n", wkt_result);

  /* Clean up everything we allocated */
  free(tpoint);
  GEOSPreparedGeom_destroy(prep_poly);
  GEOSGeom_destroy(poly);
  GEOSGeom_destroy(result);
  GEOSFree(wkt_result);

  /* Clean up the global context */
  finishGEOS();

  /* Finalize MEOS */
  meos_finalize();

  /* Done */
  return 0;
}
