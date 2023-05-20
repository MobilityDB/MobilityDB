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
 * gcc -Wall -g -I/usr/local/include -o 01_meos_hello_world 01_meos_hello_world.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header */
#include <meos.h>

int main()
{
  /* Initialize MEOS */
  meos_initialize(NULL);

  /* Input temporal points in WKT format */
  char *inst_wkt = "POINT(1 1)@2000-01-01";
  char *seq_disc_wkt = "{POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02}";
  char *seq_linear_wkt = "[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]";
  char *seq_step_wkt = "Interp=Step;[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02]";
  char *ss_linear_wkt = "{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02],"
    "[POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}";
  char *ss_step_wkt = "Interp=Step;{[POINT(1 1)@2000-01-01, POINT(2 2)@2000-01-02],"
    "[POINT(3 3)@2000-01-03, POINT(3 3)@2000-01-04]}";

  /* Read WKT into temporal point object */
  Temporal *inst = tgeompoint_in(inst_wkt);
  Temporal *seq_disc = tgeompoint_in(seq_disc_wkt);
  Temporal *seq_linear = tgeompoint_in(seq_linear_wkt);
  Temporal *seq_step = tgeompoint_in(seq_step_wkt);
  Temporal *ss_linear = tgeompoint_in(ss_linear_wkt);
  Temporal *ss_step = tgeompoint_in(ss_step_wkt);

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
  char *seq_disc_mfjson = temporal_as_mfjson(seq_disc, true, 3, 6, NULL);
  printf("\n"
    "-------------------------------------------------\n"
    "| Temporal Sequence with Discrete Interpolation |\n"
    "-------------------------------------------------\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", seq_disc_wkt, seq_disc_mfjson);
  char *seq_linear_mfjson = temporal_as_mfjson(seq_linear, true, 3, 6, NULL);
  printf("\n"
    "-----------------------------------------------\n"
    "| Temporal Sequence with Linear Interpolation |\n"
    "-----------------------------------------------\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", seq_linear_wkt, seq_linear_mfjson);
  char *seq_step_mfjson = temporal_as_mfjson(seq_step, true, 3, 6, NULL);
  printf("\n"
    "--------------------------------------------\n"
    "| Temporal Sequence with Step Interpolation |\n"
    "--------------------------------------------\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", seq_step_wkt, seq_step_mfjson);
  char *ss_linear_mfjson = temporal_as_mfjson(ss_linear, true, 3, 6, NULL);
  printf("\n"
    "---------------------------------------------------\n"
    "| Temporal Sequence Set with Linear Interpolation |\n"
    "---------------------------------------------------\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", ss_linear_wkt, ss_linear_mfjson);
  char *ss_step_mfjson = temporal_as_mfjson(ss_step, true, 3, 6, NULL);
  printf("\n"
    "------------------------------------------------\n"
    "| Temporal Sequence Set with Step Interpolation |\n"
    "------------------------------------------------\n"
    "WKT:\n"
    "----\n%s\n\n"
    "MF-JSON:\n"
    "--------\n%s\n", ss_step_wkt, ss_step_mfjson);

  /* Clean up allocated objects */
  free(inst); free(inst_mfjson);
  free(seq_disc); free(seq_disc_mfjson);
  free(seq_linear); free(seq_linear_mfjson);
  free(seq_step); free(seq_step_mfjson);
  free(ss_linear); free(ss_linear_mfjson);
  free(ss_step); free(ss_step_mfjson);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
