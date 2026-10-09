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

/*
 * zlift_head -- the head's geom_array_union of each row of a cgal_zlift corpus,
 * in the shape that judge reads: "U|<union>", or "U|-" where it declines.
 * A row is "A|GEOMETRYCOLLECTION Z(B1,...,Bn)" and stands for the array
 * [A, B1, ..., Bn], which is what is unioned.
 *   usage: zlift_head < corpus > answers
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

#define MAXM 64

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();
  char *line = NULL;
  size_t cap = 0;
  while (getline(&line, &cap, stdin) > 0)
  {
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';
    if (! *line)
      continue;
    char *bar = strchr(line, '|');
    if (! bar)
    {
      printf("U|-\n");
      continue;
    }
    *bar = '\0';
    GSERIALIZED *arr[MAXM];
    int n = 0;
    arr[n++] = geom_in(line, -1);
    GSERIALIZED *coll = geom_in(bar + 1, -1);
    int ncoll = coll ? geo_num_geos(coll) : 0;
    for (int i = 1; i <= ncoll && n < MAXM; i++)
      arr[n++] = geo_geo_n(coll, i);
    GSERIALIZED *u = (arr[0] && coll) ? geom_array_union(arr, n) : NULL;
    if (! u)
      printf("U|-\n");
    else
    {
      /* Every digit, so the judge reads the doubles the engine computed */
      char *t = geo_as_text(u, 17);
      printf("U|%s\n", t);
      free(t);
      free(u);
    }
    for (int i = 0; i < n; i++)
      free(arr[i]);
    free(coll);
  }
  free(line);
  meos_finalize();
  return 0;
}
