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
 * @brief Round-trip a linear temporal float sequence through the Arrow C
 * Data Interface and assert value identity
 *
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_arrow_roundtrip temporal_arrow_roundtrip.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <arrow_c_data_interface.h>

int main(void)
{
  meos_initialize();

  const char *inputs[] = {
    /* instant */
    "42.5@2000-01-01",
    /* linear sequences */
    "[1.5@2000-01-01, 2.5@2000-01-02, 1@2000-01-03)",
    "(0@2000-01-01, -3.25@2000-01-02, 7@2000-01-03)",
    "[42@2000-01-01]",
    "[1@2000-01-01, 1@2000-01-02, 2@2000-01-03, 2@2000-01-04]",
    /* discrete sequence */
    "{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}",
    /* step sequence */
    "Interp=Step;[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]",
    /* linear sequence set */
    "{[1@2000-01-01, 2@2000-01-02), [3@2000-01-03, 4@2000-01-04]}",
    /* step sequence set */
    "Interp=Step;{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 9@2000-01-04]}",
  };
  int nin = (int) (sizeof(inputs) / sizeof(inputs[0]));
  int all_ok = 1;

  for (int k = 0; k < nin; k++)
  {
    Temporal *temp = tfloat_in(inputs[k]);
    if (! temp) { printf("FAIL: tfloat_in NULL for %s\n", inputs[k]); return 1; }

    struct ArrowSchema schema;
    struct ArrowArray array;
    if (! meos_temporal_to_arrow(temp, &schema, &array))
    { printf("FAIL: to_arrow for %s\n", inputs[k]); return 1; }

    Temporal *back = meos_temporal_from_arrow(&schema, &array);
    if (! back) { printf("FAIL: from_arrow for %s\n", inputs[k]); return 1; }

    char *s1 = tfloat_out(temp, 6);
    char *s2 = tfloat_out(back, 6);
    int ok = (strcmp(s1, s2) == 0);
    all_ok &= ok;
    printf("in : %s\nout: %s\n%s\n", s1, s2, ok ? "PASS" : "FAIL");

    free(s1); free(s2);
    schema.release(&schema);
    array.release(&array);
    free(temp); free(back);
  }

  meos_finalize();
  return all_ok ? 0 : 1;
}
