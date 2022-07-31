/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * gcc -Wall -g -I. -o meos_hello_world meos_hello_world.c -L/usr/local/bin -lmeos
 * @endcode
 */


/* meos_hello_world.c
 * To build the program run the following command
 * cc meos_hello_world.c -o meos_hello_world -l meos
 */

#include <stdio.h>  /* for printf */

/* Include the MEOS API header */
#include "meos.h"

int main()
{
  /* Initialize session_timezone */
  meos_initialize();

  /* Read WKT into geometry object */
  Temporal *inst = tgeompoint_in("POINT(1 1)@2000-01-01");
  Temporal *is = tgeompoint_in("{POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02}");
  Temporal *seq = tgeompoint_in("[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]");
  Temporal *ss = tgeompoint_in("{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02],"
    "[POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}");

  /* Convert result to MF-JSON */
  char *inst_wkt = temporal_as_mfjson(inst, true, 6, NULL);
  printf("Temporal instant:\n%s\n", inst_wkt);
  char *is_wkt = temporal_as_mfjson(is, true, 6, NULL);
  printf("Temporal instant set:\n%s\n", is_wkt);
  char *seq_wkt = temporal_as_mfjson(seq, true, 6, NULL);
  printf("Temporal sequence:\n%s\n", seq_wkt);
  char *ss_wkt = temporal_as_mfjson(ss, true, 6, NULL);
  printf("Temporal sequence set:\n%s\n", ss_wkt);

  /* Clean up allocated objects */
  free(inst); free(inst_wkt);
  free(is); free(is_wkt);
  free(seq); free(seq_wkt);
  free(ss); free(ss_wkt);

  /* Finalize MEOS */
  meos_finish();

  /* Return */
  return 0;
}
