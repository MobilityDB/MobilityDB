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
 * @brief A simple program that changes the IntervalStyle parameter of the MEOS
 * library.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o intervalstyle intervalstyle.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header */
#include <meos.h>

int main()
{
  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  Interval *i = pg_interval_in("-1 year -2 mons +3 days -04:05:06", -1);

  char *intervalstyle = meos_get_intervalstyle();
  printf("Default IntervalStyle: %s\n", intervalstyle);
  free(intervalstyle);

  char *i_out = pg_interval_out(i);
  printf("Interval: %s\n", i_out);
  free(i_out);

  bool ok = meos_set_intervalstyle("sql_standard", NULL);
  if (! ok)
  {
    printf("Error while setting the IntervalStyle\n");
    return 0;
  }
  intervalstyle = meos_get_intervalstyle();
  printf("New IntervalStyle: %s\n", intervalstyle);
  free(intervalstyle);
  
  i_out = pg_interval_out(i);
  printf("Interval: %s\n", i_out);
  free(i_out);
  free(i);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
