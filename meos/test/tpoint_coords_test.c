/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief A simple program that tests the coordinates and timestamps of the
 * instants of a temporal point
 * @details The program reads each instant of a temporal point a second way,
 * through its sequences, the public instants of each sequence, the temporal
 * floats of its coordinates and its start timestamp, dropping an instant that
 * repeats the last one of the previous sequence, and compares what it reads
 * with the arrays #tpoint_coords answers. The arrays of a sequence are given
 * back to #tpointseq_make_coords, which makes the sequence again. The
 * temporal points are generated, geometric and geodetic, with and without Z,
 * instants, discrete, linear and step sequences and sequence sets, among them
 * sequence sets whose sequences meet at an instant. The errors of the
 * function are read too.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tpoint_coords_test tpoint_coords_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of generated temporal points */
#define NCASES 2500

static int checks = 0, failures = 0;

static void
check(bool ok, const char *what, const char *value)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAILED: %s: %s\n", what, value);
  }
}

/* A generator of its own, so that the corpus is the same everywhere */
static uint64_t state = 0x9E3779B97F4A7C15ULL;
static int
uniform(int n)
{
  state ^= state << 13; state ^= state >> 7; state ^= state << 17;
  return (int) (state % (uint64_t) n);
}
static double
coord(double lo, double hi)
{
  /* Two decimals, so the text read back is the double written */
  return lo + (double) uniform((int) ((hi - lo) * 100)) / 100.0;
}

/* Append to buf the text of n instants from minute *t on */
static void
instants(char *buf, int n, bool geodetic, bool hasz, int *t)
{
  for (int i = 0; i < n; i++)
  {
    char inst[128];
    double x = geodetic ? coord(-179, 179) : coord(-1000, 1000);
    double y = geodetic ? coord(-89, 89) : coord(-1000, 1000);
    if (hasz)
      sprintf(inst, "%sPoint Z(%.2f %.2f %.2f)@2001-01-01 %02d:%02d:00+00",
        i ? ", " : "", x, y, coord(-100, 100), *t / 60, *t % 60);
    else
      sprintf(inst, "%sPoint(%.2f %.2f)@2001-01-01 %02d:%02d:00+00",
        i ? ", " : "", x, y, *t / 60, *t % 60);
    strcat(buf, inst);
    *t += 1 + uniform(5);
  }
}

/* Generate the text of a temporal point of kind k */
static void
generate(char *buf, int k, bool geodetic, bool hasz)
{
  int t = uniform(30);
  buf[0] = '\0';
  switch (k)
  {
    case 0: /* instant */
      instants(buf, 1, geodetic, hasz, &t);
      break;
    case 1: /* discrete sequence */
      strcat(buf, "{"); instants(buf, 1 + uniform(6), geodetic, hasz, &t);
      strcat(buf, "}");
      break;
    case 2: /* linear sequence */
      strcat(buf, "["); instants(buf, 2 + uniform(6), geodetic, hasz, &t);
      strcat(buf, "]");
      break;
    case 3: /* step sequence */
      strcat(buf, "Interp=Step;[");
      instants(buf, 2 + uniform(6), geodetic, hasz, &t);
      strcat(buf, "]");
      break;
    default: /* sequence set, with a gap between its sequences */
    {
      int nseqs = 2 + uniform(3);
      strcat(buf, "{");
      for (int s = 0; s < nseqs; s++)
      {
        strcat(buf, s ? ", [" : "[");
        instants(buf, 2 + uniform(4), geodetic, hasz, &t);
        strcat(buf, "]");
        t += 10;
      }
      strcat(buf, "}");
    }
  }
}

/* Read a temporal point the second way and compare it with the arrays */
static void
compare(const char *text, bool geodetic, interpType interp)
{
  Temporal *temp = geodetic ? tgeogpoint_in(text) : tgeompoint_in(text);
  if (! temp)
  {
    check(false, "reading", text);
    meos_errno_reset();
    return;
  }
  bool hasz = strstr(text, "Point Z") != NULL;

  double *ys, *zs;
  TimestampTz *times;
  int count;
  double *xs = tpoint_coords(temp, &ys, &zs, &times, &count);
  check(xs && ys && times && (hasz ? zs != NULL : zs == NULL),
    "arrays answered", text);
  if (! xs || ! ys || ! times)
  {
    free(temp);
    return;
  }

  /* The instants read through the sequences, an instant and a discrete
   * sequence, which have no sequences, being read as one part */
  int nseqs;
  TSequence **seqs = temporal_sequences(temp, &nseqs);
  if (! seqs)
  {
    meos_errno_reset();
    nseqs = 1;
  }
  int n = 0;
  Temporal *last = NULL;
  bool same = true;
  for (int s = 0; s < nseqs; s++)
  {
    int ninsts;
    TInstant **insts = temporal_instants(seqs ? (Temporal *) seqs[s] : temp,
      &ninsts);
    for (int i = 0; i < ninsts; i++)
    {
      Temporal *inst = (Temporal *) insts[i];
      /* An instant repeating the last one of the previous sequence */
      if (last && i == 0 && temporal_eq(last, inst))
      {
        free(inst);
        continue;
      }
      if (n < count)
      {
        Temporal *x = tpoint_get_x(inst), *y = tpoint_get_y(inst);
        same &= xs[n] == tfloat_start_value(x);
        same &= ys[n] == tfloat_start_value(y);
        free(x); free(y);
        if (hasz)
        {
          Temporal *z = tpoint_get_z(inst);
          same &= zs[n] == tfloat_start_value(z);
          free(z);
        }
        same &= times[n] == temporal_start_timestamptz(inst);
      }
      free(last);
      last = inst;
      n++;
    }
    free(insts);
    if (seqs)
      free(seqs[s]);
  }
  free(last);
  free(seqs);
  check(n == count, "number of instants", text);
  check(same, "coordinates and timestamps", text);

  /* A sequence with inclusive bounds is made again from its arrays */
  if (interp != INTERP_NONE)
  {
    TSequence *seq = tpointseq_make_coords(xs, ys, zs, times, count,
      tspatial_srid(temp), geodetic, true, true, interp, false);
    check(seq && temporal_eq((Temporal *) seq, temp),
      "the sequence made again from its arrays", text);
    free(seq);
  }
  free(xs); free(ys); free(zs); free(times); free(temp);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_initialize_timezone("UTC");

  /* The generated temporal points */
  char buf[8192];
  for (int c = 0; c < NCASES; c++)
  {
    int k = c % 5;
    bool geodetic = (c / 5) % 2, hasz = (c / 10) % 2;
    generate(buf, k, geodetic, hasz);
    compare(buf, geodetic, k == 1 ? DISCRETE : k == 2 ? LINEAR :
      k == 3 ? STEP : INTERP_NONE);
  }

  /* Two sequences meeting at an instant they share, and two meeting at a
   * timestamp with two values */
  compare("{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02), "
    "[Point(2 2)@2001-01-02, Point(3 3)@2001-01-03]}", false, INTERP_NONE);
  compare("Interp=Step;{[Point(1 1)@2001-01-01, Point(1 1)@2001-01-02), "
    "[Point(3 3)@2001-01-02, Point(4 4)@2001-01-03]}", false, INTERP_NONE);

  /* The arrays follow the instants in the order of time */
  Temporal *temp = tgeompoint_in("{Point(5 6)@2001-01-01, "
    "Point(3 4)@2001-01-02, Point(1 2)@2001-01-03}");
  double *ys, *zs;
  TimestampTz *times;
  int count;
  double *xs = tpoint_coords(temp, &ys, &zs, &times, &count);
  check(count == 3 && xs[0] == 5 && ys[0] == 6 && xs[2] == 1 && ys[2] == 2 &&
    zs == NULL && times[0] < times[1] && times[1] < times[2],
    "the order of the instants", "Point(5 6), Point(3 4), Point(1 2)");
  free(xs); free(ys); free(times); free(temp);

  /* The errors: no temporal value, a temporal value that is not a point, and
   * no place for an array or the count */
  ys = zs = (double *) &count;
  times = (TimestampTz *) &count;
  count = 7;
  xs = tpoint_coords(NULL, &ys, &zs, &times, &count);
  check(! xs && ! ys && ! zs && ! times && count == 0 && meos_errno() != 0,
    "no temporal value", "NULL");
  meos_errno_reset();
  temp = tfloat_in("[1@2001-01-01, 2@2001-01-02]");
  xs = tpoint_coords(temp, &ys, &zs, &times, &count);
  check(! xs && ! ys && ! zs && ! times && count == 0 && meos_errno() != 0,
    "a temporal float", "[1@2001-01-01, 2@2001-01-02]");
  meos_errno_reset();
  check(! tpoint_coords(temp, NULL, &zs, &times, &count) &&
    meos_errno() != 0, "no place for the y coordinates", "NULL");
  meos_errno_reset();
  check(! tpoint_coords(temp, &ys, NULL, &times, &count) &&
    meos_errno() != 0, "no place for the z coordinates", "NULL");
  meos_errno_reset();
  check(! tpoint_coords(temp, &ys, &zs, NULL, &count) &&
    meos_errno() != 0, "no place for the timestamps", "NULL");
  meos_errno_reset();
  check(! tpoint_coords(temp, &ys, &zs, &times, NULL) &&
    meos_errno() != 0, "no place for the count", "NULL");
  meos_errno_reset();
  free(temp);

  printf("tpoint_coords_test: %d checks, %d failed\n", checks, failures);
  meos_finalize();
  return failures ? 1 : 0;
}
