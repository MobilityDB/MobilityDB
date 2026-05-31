/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Round-trip every set, span and span set base type through the
 * Arrow C Data Interface and assert value identity. set, span and spanset
 * are the MEOS closure types (finite subsets of an infinite base domain);
 * this checks the Arrow interchange is lossless over all of them.
 *
 * @code
 * gcc -Wall -g -I/usr/local/include -o setspan_arrow_roundtrip setspan_arrow_roundtrip.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static int g_all_ok = 1;

/* Typed helpers keep the comparison printer identical for in and out. */
#define CHECK_SET(LABEL, INFN, OUTEXPR_S) do {                              \
  Set *s = INFN;                                                           \
  if (! s) { printf("FAIL: " LABEL " in NULL\n"); g_all_ok = 0; break; }   \
  Set *back = meos_set_arrow_roundtrip(s);                                 \
  if (! back) { printf("FAIL: " LABEL " roundtrip NULL\n"); g_all_ok = 0;  \
    free(s); break; }                                                      \
  char *a = OUTEXPR_S(s); char *b = OUTEXPR_S(back);                        \
  int ok = (strcmp(a, b) == 0); g_all_ok &= ok;                            \
  printf(LABEL " in : %s\n" LABEL " out: %s\n%s\n", a, b,                   \
    ok ? "PASS" : "FAIL");                                                  \
  free(a); free(b); free(s); free(back);                                   \
} while (0)

#define CHECK_SPAN(LABEL, INFN, OUTEXPR_S) do {                            \
  Span *s = INFN;                                                          \
  if (! s) { printf("FAIL: " LABEL " in NULL\n"); g_all_ok = 0; break; }   \
  Span *back = meos_span_arrow_roundtrip(s);                               \
  if (! back) { printf("FAIL: " LABEL " roundtrip NULL\n"); g_all_ok = 0;  \
    free(s); break; }                                                      \
  char *a = OUTEXPR_S(s); char *b = OUTEXPR_S(back);                        \
  int ok = (strcmp(a, b) == 0); g_all_ok &= ok;                            \
  printf(LABEL " in : %s\n" LABEL " out: %s\n%s\n", a, b,                   \
    ok ? "PASS" : "FAIL");                                                  \
  free(a); free(b); free(s); free(back);                                   \
} while (0)

#define CHECK_SS(LABEL, INFN, OUTEXPR_S) do {                              \
  SpanSet *s = INFN;                                                       \
  if (! s) { printf("FAIL: " LABEL " in NULL\n"); g_all_ok = 0; break; }   \
  SpanSet *back = meos_spanset_arrow_roundtrip(s);                         \
  if (! back) { printf("FAIL: " LABEL " roundtrip NULL\n"); g_all_ok = 0;  \
    free(s); break; }                                                      \
  char *a = OUTEXPR_S(s); char *b = OUTEXPR_S(back);                        \
  int ok = (strcmp(a, b) == 0); g_all_ok &= ok;                            \
  printf(LABEL " in : %s\n" LABEL " out: %s\n%s\n", a, b,                   \
    ok ? "PASS" : "FAIL");                                                  \
  free(a); free(b); free(s); free(back);                                   \
} while (0)

static char *f_set(const Set *s)      { return floatset_out(s, 15); }
static char *f_span(const Span *s)    { return floatspan_out(s, 15); }
static char *f_ss(const SpanSet *s)   { return floatspanset_out(s, 15); }
static char *g_set(const Set *s)      { return spatialset_as_text(s, 15); }

int main(void)
{
  meos_initialize();

  /* Sets: int, bigint, float, text, date, tstz, geometry, geography */
  CHECK_SET("intset    ", intset_in("{1, 2, 3, 5, 8}"), intset_out);
  CHECK_SET("bigintset ", bigintset_in("{10000000000, 20000000000}"),
    bigintset_out);
  CHECK_SET("floatset  ", floatset_in("{1.5, -3.25, 7, 42.125}"), f_set);
  CHECK_SET("textset   ", textset_in("{\"alpha\", \"beta\", \"gamma\"}"),
    textset_out);
  CHECK_SET("dateset   ",
    dateset_in("{2000-01-01, 2000-06-15, 2001-12-31}"), dateset_out);
  CHECK_SET("tstzset   ",
    tstzset_in("{2000-01-01 08:00:00+00, 2000-01-02 09:30:00+00}"),
    tstzset_out);
  CHECK_SET("geomset   ",
    geomset_in("{Point(1 1), Point(2 3), Point(4 5)}"), g_set);
  CHECK_SET("geomsrid  ",
    geomset_in("SRID=4326;{Point(1 1), Point(2 3)}"), g_set);
  CHECK_SET("geogset   ",
    geogset_in("{Point(1 1), Point(2 3)}"), g_set);

  /* Spans: int, bigint, float, date, tstz (no geometry span exists) */
  CHECK_SPAN("intspan   ", intspan_in("[1, 10)"), intspan_out);
  CHECK_SPAN("bigintspan",
    bigintspan_in("[100000000000, 200000000000]"), bigintspan_out);
  CHECK_SPAN("floatspan ", floatspan_in("[1.5, 9.75)"), f_span);
  CHECK_SPAN("datespan  ",
    datespan_in("(2000-01-01, 2001-01-01]"), datespan_out);
  CHECK_SPAN("tstzspan  ",
    tstzspan_in("[2000-01-01 00:00:00+00, 2000-12-31 23:59:59+00)"),
    tstzspan_out);

  /* Span sets: int, bigint, float, date, tstz */
  CHECK_SS("intss     ",
    intspanset_in("{[1, 3), [5, 8), [10, 12)}"), intspanset_out);
  CHECK_SS("bigintss  ",
    bigintspanset_in("{[1000000000, 2000000000], [9000000000, 9999999999]}"),
    bigintspanset_out);
  CHECK_SS("floatss   ",
    floatspanset_in("{[1.5, 2.5), (3, 4], [10.25, 11.75)}"), f_ss);
  CHECK_SS("datess    ",
    datespanset_in("{[2000-01-01, 2000-02-01), [2000-06-01, 2000-07-01)}"),
    datespanset_out);
  CHECK_SS("tstzss    ",
    tstzspanset_in("{[2000-01-01 00:00:00+00, 2000-01-02 00:00:00+00), "
      "[2000-03-01 00:00:00+00, 2000-04-01 00:00:00+00)}"),
    tstzspanset_out);

  meos_finalize();
  printf("\n%s\n", g_all_ok ? "OVERALL PASS" : "OVERALL FAIL");
  return g_all_ok ? 0 : 1;
}
