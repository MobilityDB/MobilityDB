/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief A simple program that uses the MEOS library for uses the set_union
 * aggregate function
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o set_agg set_agg.c -L/usr/local/lib -lmeos
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

  /* Add some value to aggregate */
  Set *agg = NULL;
  for (int i = 10; i >= 0; i--)
  {
    agg = int_union_transfn(agg, i * 2);
    agg = int_union_transfn(agg, i * 2);
  }
  char *agg_out = intset_out(agg);
  printf("Result of the transition function: %s\n", agg_out);

  Set *result = set_union_finalfn(agg);
  char *result_out = intset_out(result);
  printf("Result of the set union aggregation: %s\n", result_out);

  /* Clean up allocated objects */
  free(agg); free(result); free(result_out);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
