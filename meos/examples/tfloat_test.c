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
 * @brief A simple program that uses the MEOS library for creating some
 * temporal values and output them in MF-JSON format.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tfloat_test tfloat_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for free */

/* Include the MEOS API header */
#include <meos.h>

extern double DatumGetFloat8(Datum);

int main()
{
  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Input temporal points in WKT format */
  Temporal *tf1 = tfloat_in("1@aaaa");
  if (!tf1)
  {
    printf("Input error");
    goto finalize;
  }
  /* Print result in WKT */
  char *tf1_wkt = tfloat_out(tf1, 3);
  printf("\nsl: %s\n", tf1_wkt);

  /* Clean up allocated objects */
  free(tf1); free(tf1_wkt);

finalize:
  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
