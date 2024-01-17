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
 * @brief A simple program that transform an AIS trip obtained from the file
 * `aisdk-2023-08-01.csv` with MMSI 205718000 that only constains a few points 
 * from SRID 4326 to SRID 25832
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ais_transform ais_transform.c -L/usr/local/lib -lproj -lmeos
 * @endcode
 */

/*****************************************************************************/

/* C */
#include <stdio.h>
#include <stdlib.h>
/* PROJ */
#include <proj.h>
/* C */
#include <meos.h>
#include <meos_internal.h>

int main(void)
{
  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Input trip in SRID 4326 */
  char *trip_str =
    "SRID=4326;[POINT(6.369747 55.209627)@2023-01-08 18:21:44+01,"
    "POINT(6.368953 55.210777)@2023-01-08 18:22:55+01,"
    "POINT(6.368603 55.211165)@2023-01-08 18:23:15+01,"
    "POINT(6.367535 55.212192)@2023-01-08 18:23:44+01,"
    "POINT(6.36672 55.21301)@2023-01-08 18:24:05+01,"
    "POINT(6.355373 55.22781)@2023-01-08 18:30:15+01,"
    "POINT(6.35513 55.228128)@2023-01-08 18:30:25+01," 
    "POINT(6.352437 55.23207)@2023-01-08 18:32:54+01,"
    "POINT(6.352097 55.232548)@2023-01-08 18:33:15+01,"
    "POINT(6.351793 55.23299)@2023-01-08 18:33:34+01]";
  Temporal *trip = tgeompoint_in(trip_str);
  char *str_out = tpoint_as_ewkt(trip, 6);
  printf("----------------------------\n");
  printf(" Original trip in SRID 4326\n");
  printf("----------------------------\n%s\n", str_out);
  free(str_out);

  Temporal *trip_out = tpoint_transform(trip, 25832);
  str_out = tpoint_as_ewkt(trip_out, 6);
  printf("--------------------------------\n");
  printf(" Transformed trip in SRID 25832\n");
  printf("--------------------------------\n%s\n", str_out);
  free(str_out);

  /* Clean up */
  meos_finalize();
  return 0;
}

/*****************************************************************************/
