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
 * @brief Data generator for MobilityDB.
 *
 * These functions are used in the BerlinMOD data generator
 * https://github.com/MobilityDB/MobilityDB-BerlinMOD
 */

/* PostgreSQL */
#include <postgres.h>
#include <access/htup_details.h>
#include <access/tupdesc.h>    /* for * () */
#include <executor/executor.h>  /* for GetAttributeByName() */
#include <utils/typcache.h>
#include <utils/float.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
/* GSL */
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************/

/* Global variables */

bool _gsl_initizalized = false;
const gsl_rng_type *_rng_type;
gsl_rng *_rng;

/**
 * @brief Initialize the Gnu Scientific Library
 */
static void
initialize_gsl()
{
  gsl_rng_env_setup();
  _rng_type = gsl_rng_default;
  _rng = gsl_rng_alloc(_rng_type);
  _gsl_initizalized = true;
  return;
}

/**
 * @brief Return the angle in degrees between 3 points
 */
static double
pt_angle(POINT2D p1, POINT2D p2, POINT2D p3)
{
  double result, az1 = 0, az2 = 0; /* make compilier quiet */
  if (! azimuth_pt_pt(&p1, &p2, &az1) ||  ! azimuth_pt_pt(&p3, &p2, &az2))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot compute angle betwen equal points")));
  result = az2 - az1;
  result += (result < 0) * 2 * M_PI; /* we dont want negative angle */
  return result / RADIANS_PER_DEGREE;
}

/* Helper macro to add an instant containing the current position */
#define ADD_CURRENT_POSITION  \
  do {  \
      lwpoint = lwpoint_make2d(srid, curPos.x, curPos.y);  \
      point = PointerGetDatum(geo_serialize((LWGEOM *) lwpoint));  \
      lwpoint_free(lwpoint);  \
      instants[l++] = tinstant_make(point, T_TGEOMPOINT, t);  \
      pfree(DatumGetPointer(point));  \
  } while (0)

/**
 * @brief Create a trip using the BerlinMOD data generator
 */
static TSequence *
create_trip_internal(LWLINE **lines, const double *maxSpeeds, const int *categories,
  uint32_t noEdges, TimestampTz startTime, bool disturbData, int verbosity)
{
  /* CONSTANT PARAMETERS */

  /* Speed in km/h which is considered as a stop and thus only an
   * accelaration event can be applied */
  double P_EPSILON_SPEED = 1.0;
  /* Used for determining whether the distance is almost equal to 0.0 */
  double P_EPSILON = 0.0001;

  /* The probability of an event is proportional to (P_EVENT_C)/Vmax.
   * The probability for an event being a forced stop is given by
   * 0.0 <= 'P_EVENT_P' <= 1.0 (the balance, 1-P, is meant to trigger
   * deceleration events). */
  double P_EVENT_C = 1.0;
  double P_EVENT_P = 0.1;

  /* Sampling distance in meters at which an acceleration/deceleration/stop
   * event may be generated. */
  double P_EVENT_LENGTH = 5.0;
  /* Constant speed edgess in meters/second, simplification of the accelaration */
  double P_EVENT_ACC = 12.0;

  /* Probabilities for forced stops at crossings by road type transition
   * defined by a matrix where rows and columns are ordered by side road (S),
   * main road (M), freeway (F). The OSM highway types must be mapped to one
   * of these categories using the function berlinmod_roadCategory */
  double P_DEST_STOPPROB[3][3] =
    {{0.33, 0.66, 1.00}, {0.33, 0.50, 0.66}, {0.10, 0.33, 0.05}};
  /* Mean waiting time in seconds using an exponential distribution.
   * Increasing/decreasing this parameter allows us to slow down or speed up
   * the trips. Could be think of as a measure of network congestion.
   * Given a specific path, fine-tuning this parameter enable us to obtain
   * an average travel time for this path that is the same as the expected
   * travel time computed, e.g., by Google Maps. */
  double P_DEST_EXPMU = 1.0;
  /* Parameters for measuring errors (only required for P_DISTURB_DATA = TRUE)
   * Maximum total deviation from the real position (default = 100.0)
   * and maximum deviation per step (default = 1.0) both in meters. */
  double P_GPS_TOTALMAXERR = 100.0;
  double P_GPS_STEPMAXERR = 1.0;

  /* Variables */

  /* SRID of the geometries being manipulated */
  int srid;
  /* Number of points in an edge, number of fractions of size
   * P_EVENT_LENGTH in a segment */
  uint32_t noPoints, noFracs;
  /* Loop variables */
  uint32_t i, j, k;
  /* Number of instants generated so far */
  uint32_t l = 0;
  /* Current speed and distance of the moving object */
  double curSpeed, curDist;
  /* Time to wait when the speed is almost 0.0 */
  double waitTime;
  /* Time to travel the fraction given the current speed */
  double travelTime;
  /* Angle between the current segment and the next one */
  double alpha = 0; /* make compiler quiet */
  /* Maximum speed when approaching a turn between two segments */
  double maxSpeedTurn = 0; /* make compiler quiet */
  /* Maximum speed and new speed of the car */
  double maxSpeed, newSpeed;
  /* Number in [0,1] used for determining the next point */
  double fraction;
  /* Disturbance of the coordinates of a point and total accumulated
   * error in the coordinates of an edge. Used when disturbing the position
   * of an object to simulate GPS errors */
  double dx, dy;
  double errx = 0.0, erry = 0.0;
  /* Length of a segment */
  double segLength;
  /* Points */
  POINT2D p1, p2, p3, curPos;
  /* Current position of the moving object */
  LWPOINT *lwpoint;
  Datum point;
  /* Current timestamp of the moving object */
  TimestampTz t;
  /* Instants of the result being constructed */
  TInstant **instants;
  /* Number of instants of the result */
  int noInstants = 0;
  /* Statistics about the trip */
  int noAccel = 0, noDecel = 0, noStop = 0;
  double twSumSpeed = 0.0, totalTravelTime = 0.0, totalWaitTime = 0.0;

  if (!_gsl_initizalized)
    initialize_gsl();

  /* First Pass: Compute the number of instants of the result */

  for (i = 0; i < noEdges; i++)
  {
    noPoints = lines[i]->points->npoints;
    p1 = getPoint2d(lines[i]->points, 0);
    for (j = 1; j < noPoints; j++)
    {
      p2 = getPoint2d(lines[i]->points, j);
      segLength = hypot(p1.x - p2.x, p1.y - p2.y);
      if (segLength < P_EPSILON)
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
          errmsg("Segment %d of edge %d has zero length", j, i)));
      /* At every fraction there could be a stop event */
      noInstants += (int) ceil(segLength / P_EVENT_LENGTH) * 2;
      p1 = p2;
    }
  }
  instants = palloc(sizeof(TInstant *) * noInstants);

  /* Second Pass: Compute the result */
  srid = lines[0]->srid;
  p1 = getPoint2d(lines[0]->points, 0);
  curPos = p1;
  t = startTime;
  curSpeed = 0;
  ADD_CURRENT_POSITION;
  /* Loop for every edge */
  for (i = 0; i < noEdges; i++)
  {
    if (verbosity == 3)
      ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Edge %d", i + 1)));
    /* Get the information about the current edge */
    double maxSpeedEdge = maxSpeeds[i];
    int category = categories[i];
    noPoints = lines[i]->points->npoints;
    /* Loop for every segment of the current edge */
    for (j = 1; j < noPoints; j++)
    {
      if (verbosity == 3 && noPoints > 2)
        ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
        errmsg("        Segment %d", j)));
      p2 = getPoint2d(lines[i]->points, j);
      /* If there is a segment ahead in the current edge
       * compute the angle of the turn */
      if (j < noPoints - 1)
      {
        p3 = getPoint2d(lines[i]->points, j + 1);
        /* Compute the angle α between the current segment and the next one */
        alpha = pt_angle(p1, p2, p3);
        /* Compute the maximum speed at the turn by multiplying the
         * maximum speed by a factor proportional to the angle so that
         * the factor is 1.00 at 0/360° and is 0.0 at 180°, e.g.
         * 0° -> 1.00, 5° 0.97, 45° 0.75, 90° 0.50, 135° 0.25, 175° 0.03
         * 180° 0.00, 185° 0.03, 225° 0.25, 270° 0.50, 315° 0.75, 355° 0.97, 360° 0.00 */
        if (fabs(fmod(alpha, 360.0)) < P_EPSILON)
          maxSpeedTurn = maxSpeedEdge;
        else
          maxSpeedTurn = fmod(fabs(alpha - 180.0), 180.0) / 180.0 * maxSpeedEdge;
      }
      segLength = hypot(p1.x - p2.x, p1.y - p2.y);
      /* We have already tested that the segment lenght is not 0.0 */
      fraction = P_EVENT_LENGTH / segLength;
      noFracs = (uint32_t) ceil(segLength / P_EVENT_LENGTH);
      /* Loop for every fraction of the current segment */
      k = 0;
      while (k < noFracs)
      {
        /* FIRST SECTION: Determine the next speed */

        /* If the current speed is considered as a stop, apply an
         * acceleration event where the new speed is bounded by the
         * maximum speed of either the segment or the turn */
        if (curSpeed <= P_EPSILON_SPEED)
        {
          noAccel++;
          /* If we are not approaching a turn */
          if (k < noFracs)
            curSpeed = Min(P_EVENT_ACC, maxSpeedEdge);
          else
            curSpeed = Min(P_EVENT_ACC, maxSpeedTurn);
          if (verbosity == 3)
            ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
            errmsg("          Acceleration after stop -> Speed = %.3f", curSpeed)));
        }
        else
        {
          /* If the current speed is not considered as a stop, with
           * a probability proportional to 1/maxSpeedEdge apply a
           * deceleration event (p=90%) or a stop event (p=10%) */
          if (gsl_rng_uniform(_rng) <= P_EVENT_C / maxSpeedEdge)
          {
            if (gsl_rng_uniform(_rng) <= P_EVENT_P)
            {
              /* Apply stop event */
              curSpeed = 0.0;
              noStop++;
              if (verbosity == 3)
                ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("          Stop -> Speed = %.3f", curSpeed)));
            }
            else
            {
              /* Apply deceleration event */
              curSpeed = curSpeed * gsl_ran_binomial(_rng, 0.5, 20) / 20.0;
              noDecel++;
              if (verbosity == 3)
                ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("          Deceleration -> Speed = %.3f", curSpeed)));
            }
          }
          else
          {
            /* Adapt the speed to a turn (if we are in the last
             * fraction of an inner segment) and apply an
             * acceleration event (if possible). */
            if (k == noFracs - 1 && j < noPoints - 1)
            {
              maxSpeed = maxSpeedTurn;
              if (verbosity == 3)
                ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("          Turn -> Angle = %.3f, Maximum speed at turn = %.3f",
                  alpha, maxSpeedTurn)));
            }
            else
              maxSpeed = maxSpeedEdge;
            newSpeed = Min(curSpeed + P_EVENT_ACC, maxSpeed);
            if (curSpeed < newSpeed)
            {
              noAccel++;
              if (verbosity == 3)
                ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("          Acceleration -> Speed = %.3f", newSpeed)));
            }
            else if (curSpeed > newSpeed)
            {
              noDecel++;
              if (verbosity == 3)
                ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
                errmsg("          Deceleration -> Speed = %.3f", newSpeed)));
            }
            curSpeed = newSpeed;
          }
        }

        /* SECOND SECTION: Determine the next location and time */

        /* If speed is zero add a wait time */
        if (curSpeed < P_EPSILON_SPEED)
        {
          waitTime = gsl_ran_exponential(_rng, P_DEST_EXPMU);
          if (waitTime < P_EPSILON)
            waitTime = P_DEST_EXPMU;
          t = t + (int) (waitTime * 1e6); /* microseconds */
          totalWaitTime += waitTime;
          if (verbosity == 3)
            ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
            errmsg("          Waiting for %.3f seconds", waitTime)));
        }
        else
        {
          /* Otherwise, move current position P_EVENT_LENGTH meters
           * towards p2 or to p2 if it is the last fraction */
          if (k < noFracs - 1)
          {
            curPos.x = p1.x + ((p2.x - p1.x) * fraction * (k + 1));
            curPos.y = p1.y + ((p2.y - p1.y) * fraction * (k + 1));
            if (disturbData)
            {
              dx = (2.0 * P_GPS_STEPMAXERR * gsl_rng_uniform(_rng)) -
                P_GPS_STEPMAXERR;
              dy = (2.0 * P_GPS_STEPMAXERR * gsl_rng_uniform(_rng)) -
                P_GPS_STEPMAXERR;
              errx += dx;
              erry += dy;
              if (errx > P_GPS_TOTALMAXERR)
                errx = P_GPS_TOTALMAXERR;
              if (errx < - 1 * P_GPS_TOTALMAXERR)
                errx = -1 * P_GPS_TOTALMAXERR;
              if (erry > P_GPS_TOTALMAXERR)
                erry = P_GPS_TOTALMAXERR;
              if (erry < -1 * P_GPS_TOTALMAXERR)
                erry = -1 * P_GPS_TOTALMAXERR;
              curPos.x += dx;
              curPos.y += dy;
            }
            curDist = P_EVENT_LENGTH;
          }
          else
          {
            curPos = p2;
            curDist = segLength - (segLength * fraction * k);
          }
          travelTime = curDist / (curSpeed / 3.6);
          if (travelTime < P_EPSILON)
            travelTime = P_DEST_EXPMU;
          t = t + (int) (travelTime * 1e6); /* microseconds */
          totalTravelTime += travelTime;
          twSumSpeed += travelTime * curSpeed;
          k++;
        }
        ADD_CURRENT_POSITION;
      }
      p1 = p2;
    }
    /* If we are not already in a stop and not at the last edge, apply a
     * stop event with a probability depending on the categories of the
     * current edge and the next one */
    if (curSpeed > P_EPSILON_SPEED && i < noEdges - 1)
    {
      int nextCategory = categories[i + 1];
      if (gsl_rng_uniform(_rng) <= P_DEST_STOPPROB[category][nextCategory])
      {
        curSpeed = 0.0;
        waitTime = gsl_ran_exponential(_rng, P_DEST_EXPMU);
        if (waitTime < P_EPSILON)
          waitTime = P_DEST_EXPMU;
        t = t + (int) (waitTime * 1e6); /* microseconds */
        if (verbosity == 3)
          ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
          errmsg("        Stop at crossing -> Waiting for %.3f secs.", waitTime)));
        ADD_CURRENT_POSITION;
      }
    }
  }
  TSequence *result = tsequence_make((const TInstant **) instants, l,
    true, true, LINEAR, NORMALIZE);

  /* Display the statistics of the trip */
  if (verbosity >= 2)
  {
    if (verbosity >= 3)
      ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
        errmsg("    ------------------------------------------")));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Number of edges: %d", noEdges)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Number of acceleration events: %u", noAccel)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Number of deceleration events: %u", noDecel)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Number of stop events: %u", noStop)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Total travel time: %.3f secs.", totalTravelTime)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Total waiting time: %.3f secs.", totalWaitTime)));
    ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
      errmsg("      Time-weighted average speed: %.3f Km/h",
        twSumSpeed / (totalTravelTime + totalWaitTime))));
    if (verbosity >= 3)
      ereport(INFO, (errcode(ERRCODE_SUCCESSFUL_COMPLETION),
        errmsg("    ------------------------------------------")));
  }

  for (i = 0; i < noEdges; i++)
    lwgeom_free(lwline_as_lwgeom(lines[i]));
  pfree(lines);
  return result;
}

PGDLLEXPORT Datum create_trip(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(create_trip);
/**
 * @brief Create a trip using the BerlinMOD data generator.
 *
 * @note This function is equivalent to the PL/pgSQL function
 * CreateTrip in the BerlinMOD generator but is written in C
 * to speed up the generation.
 */
Datum
create_trip(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  if (ARR_NDIM(array) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("1-dimensional array needed")));
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool disturbData = PG_GETARG_BOOL(2);
  text *messages = PG_GETARG_TEXT_PP(3);
  char *msgstr = text2cstring(messages);
  int32 msg = 0; /* 'minimal' by default */
  Datum *datums;
  bool *nulls;
  int count;
  int16 elemWidth;
  Oid elmeTypid = ARR_ELEMTYPE(array);
  bool elemTypeByVal, isNull;
  char elemAlignmentCode;
  HeapTupleHeader td;
  Form_pg_attribute att;

  get_typlenbyvalalign(elmeTypid, &elemWidth, &elemTypeByVal, &elemAlignmentCode);
  deconstruct_array(array, elmeTypid, elemWidth, elemTypeByVal,
    elemAlignmentCode, &datums, &nulls, &count);

  td = DatumGetHeapTupleHeader(datums[0]);

  /* Extract rowtype info and find a tupdesc */
  Oid tupType = HeapTupleHeaderGetTypeId(td);
  int32 tupTypmod = HeapTupleHeaderGetTypMod(td);
  TupleDesc tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);
  /* Verify the type of the attributes */
  att = TupleDescAttr(tupdesc, 0);
  if (att->atttypid != type_oid(T_GEOMETRY))
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("First element of the record must be of type geometry")));
  }
  att = TupleDescAttr(tupdesc, 1);
  if (att->atttypid != FLOAT8OID)
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Second element of the record must be of type double precision")));
  }
  att = TupleDescAttr(tupdesc, 2);
  if (att->atttypid != INT4OID)
  {
    PG_FREE_IF_COPY(array, 0);
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Third element of the record must be of type integer")));
  }
  ReleaseTupleDesc(tupdesc);

  LWLINE **lines = palloc(sizeof(LWLINE *) * count);
  double *maxSpeeds = palloc(sizeof(double) * count);
  int *categories = palloc(sizeof(int) * count);
  for (int i = 0; i < count; i++)
  {
    if (nulls[i])
    {
      PG_FREE_IF_COPY(array, 0);
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Elements of the array cannot be NULL")));
    }
    else
    {
      td = DatumGetHeapTupleHeader(datums[i]);
      /* First Attribute: Linestring */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(GetAttributeByNum(td, 1, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
      if (gserialized_get_type(gs) != LINETYPE)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Geometry must be a linestring")));
      }
      lines[i] = lwgeom_as_lwline(lwgeom_from_gserialized(gs));
      /* Second Attribute: Maximum Speed */
      maxSpeeds[i] = DatumGetFloat8(GetAttributeByNum(td, 2, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
      /* Third Attribute: Category */
      categories[i] = DatumGetInt32(GetAttributeByNum(td, 3, &isNull));
      if (isNull)
      {
        PG_FREE_IF_COPY(array, 0);
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Elements of the record cannot be NULL")));
      }
    }
  }

  if (strcmp(msgstr, "minimal") == 0)
    msg = 0;
  else if (strcmp(msgstr, "medium") == 0)
    msg = 1;
  else if (strcmp(msgstr, "verbose") == 0)
    msg = 2;
  else if (strcmp(msgstr, "debug") == 0)
    msg = 3;
  pfree(msgstr);

  TSequence *result = create_trip_internal(lines, maxSpeeds, categories,
    (uint32_t) count, t, disturbData, msg);

  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
