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
 * gcc -Wall -g -I/usr/local/include -o meos_hello_world meos_hello_world.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */

/* Include the MEOS API header */
#include "meos.h"

int main()
{
  /* Initialize MEOS */
  meos_initialize();

  /* Input temporal points in WKT format */
  char *inst_wkt = "POINT(1 1)@2000-01-01";
  char *is_wkt = "{POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02}";
  char *seq_wkt = "[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]";
  char *ss_wkt = "{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02],"
    "[POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}";

  /* Read WKT into temporal point object */
  Temporal *inst = tgeompoint_in(inst_wkt);
  Temporal *is = tgeompoint_in(is_wkt);
  Temporal *seq = tgeompoint_in(seq_wkt);
  Temporal *ss = tgeompoint_in(ss_wkt);

  /* Convert result to MF-JSON */
  char *inst_mfjson = temporal_as_mfjson(inst, true, 3, 6, NULL);
  printf("\n"
    "--------------------\n"
    "| Temporal Instant |\n"
    "--------------------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", inst_wkt, inst_mfjson);
  char *is_mfjson = temporal_as_mfjson(is, true, 3, 6, NULL);
  printf("\n"
    "------------------------\n"
    "| Temporal Instant Set |\n"
    "------------------------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", is_wkt, is_mfjson);
  char *seq_mfjson = temporal_as_mfjson(seq, true, 3, 6, NULL);
  printf("\n"
    "---------------------\n"
    "| Temporal Sequence |\n"
    "---------------------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", seq_wkt, seq_mfjson);
  char *ss_mfjson = temporal_as_mfjson(ss, true, 3, 6, NULL);
  printf("\n"
    "-------------------------\n"
    "| Temporal Sequence Set |\n"
    "-------------------------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", ss_wkt, ss_mfjson);

  /* Clean up allocated objects */
  free(inst); free(inst_mfjson);
  free(is); free(is_mfjson);
  free(seq); free(seq_mfjson);
  free(ss); free(ss_mfjson);

  /* Finalize MEOS */
  meos_finish();

  /* Return */
  return 0;
}
