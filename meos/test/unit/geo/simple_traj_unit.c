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
 * @brief Test of the simplicity of a temporal point, #tpoint_is_simple and
 * #tpoint_make_simple
 * @details Each trajectory is built so that its answer is known: a path whose
 * first and third segments cross at (1, 1) is not simple and splits in two; a
 * zigzag of three parallel or meeting segments is simple; a hook that comes
 * back to pass 1/4 beside its first segment is simple, and the same hook
 * passing 1/4 beyond it crosses it once and splits in two. Each runs at the
 * scales 1, 2^-14 and 2^-30, which are exact on doubles, so an answer depends
 * on no magnitude: at 2^-14, segments about 10^-4 long, about 11 metres in
 * degrees.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>

static int failures = 0;
static int checks = 0;

typedef struct
{
  const char *name;
  double pts[6][2];
  int n;
  int parts;  /* the number of simple pieces, 1 for a simple path */
} SimpleCase;

static const SimpleCase cases[] =
{
  {"crossing path", {{0, 0}, {2, 2}, {2, 0}, {0, 2}}, 4, 2},
  {"zigzag", {{0, 0}, {2, 0}, {0, 1}, {2, 1}}, 4, 1},
  {"hook passing beside", {{0, 0}, {4, 0}, {4, 1}, {2, 1}, {2, 0.25},
    {1, 0.25}}, 6, 1},
  {"hook crossing", {{0, 0}, {4, 0}, {4, 1}, {2, 1}, {2, -0.25},
    {1, -0.25}}, 6, 2},
};

static void
simple_case(const SimpleCase *c, double s)
{
  char buf[1024];
  int off = snprintf(buf, sizeof(buf), "[");
  for (int i = 0; i < c->n; i++)
    off += snprintf(buf + off, sizeof(buf) - (size_t) off,
      "%sPoint(%.17g %.17g)@2001-01-01 00:00:%02d+00", i ? ", " : "",
      c->pts[i][0] * s, c->pts[i][1] * s, i);
  snprintf(buf + off, sizeof(buf) - (size_t) off, "]");
  Temporal *temp = tgeompoint_in(buf);

  checks++;
  bool simple = tpoint_is_simple(temp);
  if (simple != (c->parts == 1))
  {
    failures++;
    printf("FAIL %s at scale %g: isSimple %d, expected %d\n", c->name, s,
      simple, c->parts == 1);
  }

  checks++;
  int count = 0;
  Temporal **pieces = tpoint_make_simple(temp, &count);
  if (count != c->parts)
  {
    failures++;
    printf("FAIL %s at scale %g: makeSimple gives %d pieces, expected %d\n",
      c->name, s, count, c->parts);
  }
  for (int i = 0; i < count; i++)
    free(pieces[i]);
  free(pieces);
  free(temp);
  return;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  const double scales[] = {1.0, 0x1p-14, 0x1p-30};
  for (size_t k = 0; k < sizeof(scales) / sizeof(scales[0]); k++)
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
      simple_case(&cases[i], scales[k]);
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
