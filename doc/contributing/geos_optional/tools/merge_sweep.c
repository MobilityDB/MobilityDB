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

/* The merge of two temporal instants sharing a timestamp, over every ordered
   pair of 15 geometry types as temporal geometries, and of the types a
   temporal geography reads as temporal geographies. Each case runs in a
   process of its own, so a case that ends the process is recorded as CRASH
   and the sweep goes on. A case either ANSWERS, is REFUSED with an error
   under an error handler that returns, or CRASHES. The handler mirrors
   #handler of temporal_reach.c in this directory. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <meos.h>
#include <meos_geo.h>

static char msg[512];
static void handler(int l, int c, const char *m) { (void) l; meos_errno_set(c); snprintf(msg, sizeof msg, "%s", m ? m : ""); }

static const char *S[][2] = {
  {"POINT", "POINT(2 2)"},
  {"MULTIPOINT", "MULTIPOINT((1 1),(3 3))"},
  {"LINE", "LINESTRING(0 2,4 2)"},
  {"MULTILINE", "MULTILINESTRING((0 1,4 1),(0 3,4 3))"},
  {"POLYGON", "POLYGON((0 0,4 0,4 4,0 4,0 0))"},
  {"MULTIPOLYGON", "MULTIPOLYGON(((0 0,2 0,2 2,0 2,0 0)),((3 3,4 3,4 4,3 4,3 3)))"},
  {"TRIANGLE", "TRIANGLE((0 0,4 0,2 4,0 0))"},
  {"CIRCSTRING", "CIRCULARSTRING(0 2,2 4,4 2)"},
  {"COMPOUND", "COMPOUNDCURVE((0 2,0 0),CIRCULARSTRING(0 0,2 2,4 0))"},
  {"MULTICURVE", "MULTICURVE(CIRCULARSTRING(0 2,2 4,4 2),(0 0,4 0))"},
  {"CURVEPOLY", "CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2))"},
  {"MULTISURFACE", "MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 2,2 4,4 2,2 0,0 2)),((3 3,4 3,4 4,3 4,3 3)))"},
  {"TIN", "TIN(((0 0,4 0,2 4,0 0)),((0 0,2 4,0 4,0 0)))"},
  {"POLYHEDRAL", "POLYHEDRALSURFACE(((0 0,4 0,4 4,0 4,0 0)))"},
  {"COLLECTION", "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(0 2,4 2))"},
};
enum { N = sizeof(S) / sizeof(S[0]) };

/* 0 answered, 1 refused, 2 unreadable */
static int
one_case(int geodetic, int i, int j)
{
  char a[512], b[512];
  snprintf(a, sizeof a, "%s@2000-01-01", S[i][1]);
  snprintf(b, sizeof b, "%s@2000-01-01", S[j][1]);
  meos_initialize();
  meos_initialize_error_handler(handler);
  Temporal *t1 = geodetic ? tgeography_in(a) : tgeometry_in(a);
  Temporal *t2 = geodetic ? tgeography_in(b) : tgeometry_in(b);
  if (! t1 || ! t2)
    return 2;
  msg[0] = 0;
  Temporal *r = temporal_merge(t1, t2);
  return r ? 0 : 1;
}

int
main(void)
{
  for (int geodetic = 0; geodetic < 2; geodetic++)
  {
    int count[4] = {0, 0, 0, 0};
    for (int i = 0; i < N; i++)
      for (int j = 0; j < N; j++)
      {
        fflush(stdout);
        pid_t pid = fork();
        if (pid == 0)
          _exit(one_case(geodetic, i, j));
        int status;
        waitpid(pid, &status, 0);
        int outcome = WIFEXITED(status) ? WEXITSTATUS(status) : 3;
        if (outcome < 0 || outcome > 3)
          outcome = 3;
        count[outcome]++;
        if (outcome == 3)
          printf("  CRASH %s %s x %s\n", geodetic ? "tgeography" : "tgeometry",
            S[i][0], S[j][0]);
      }
    printf("%s: answered %d, refused %d, unreadable %d, crash %d of %d pairs\n",
      geodetic ? "tgeography" : "tgeometry", count[0], count[1], count[2],
      count[3], N * N);
  }
  return 0;
}
