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
 * arrunion_head -- print the head's geom_array_union of each corpus row, one
 * answer per line, members of a row separated by ';'. A row the head does not
 * answer prints DECLINED and the error it reported.
 *   usage: arrunion_head < corpus > answers
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

#define MAXM 64

static char msg[512];
static void
handler(int level, int code, const char *m)
{
  (void) level;
  meos_errno_set(code);
  snprintf(msg, sizeof msg, "%s", m ? m : "");
}

int
main(void)
{
  meos_initialize();
  meos_initialize_error_handler(handler);
  char *line = NULL;
  size_t cap = 0;
  while (getline(&line, &cap, stdin) > 0)
  {
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';
    if (! *line || *line == '#')
      continue;
    GSERIALIZED *arr[MAXM];
    int n = 0;
    msg[0] = '\0';
    bool unreadable = false;
    for (char *tok = strtok(line, ";"); tok && n < MAXM; tok = strtok(NULL, ";"))
    {
      arr[n] = geom_in(tok, -1);
      if (! arr[n])
        unreadable = true;
      else
        n++;
    }
    if (unreadable)
    {
      printf("UNREADABLE %s\n", msg);
      for (int i = 0; i < n; i++)
        free(arr[i]);
      continue;
    }
    GSERIALIZED *u = geom_array_union(arr, n);
    if (! u)
      printf("DECLINED %s\n", msg);
    else
    {
      char *t = geo_as_text(u, 17);
      printf("%s\n", t);
      free(t);
      free(u);
    }
    for (int i = 0; i < n; i++)
      free(arr[i]);
  }
  free(line);
  meos_finalize();
  return 0;
}
