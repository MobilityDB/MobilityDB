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
 * @brief A simple program that uses the MEOS library for creating integer and
 * float spanset values and convert them between float and integer spansets
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o spanset_conv spanset_conv.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header */
#include <meos.h>

int main()
{
  /* Initialize MEOS */
  meos_initialize(NULL, NULL);

  /* Input spansets in WKT format */
  char *iss_in = "{[1,3], [4,7]}";
  char *fss_in = "{[1.5,3.5], [4.5,7.5]}";

  /* Read WKT into temporal point object */
  SpanSet *iss = intspanset_in(iss_in);
  SpanSet *fss = floatspanset_in(fss_in);
  char *iss_out = intspanset_out(iss);
  char *fss_out = floatspanset_out(fss, 3);
  printf("Input integer span set: %s\n", iss_out);
  printf("Input float span set: %s\n", fss_out);

  /* Convert from int <-> float*/
  SpanSet *iss_conv = intspanset_floatspanset(iss);
  SpanSet *fss_conv = floatspanset_intspanset(fss);
  char *iss_conv_out = floatspanset_out(iss_conv, 3);
  char *fss_conv_out = intspanset_out(fss_conv);
  printf("Integer span set converted to float span set: %s\n", iss_conv_out);
  printf("Float span set converted to integer span: %s\n", fss_conv_out);

  /* Clean up allocated objects */
  free(iss_out); free(fss_out);
  free(iss_conv_out); free(fss_conv_out);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
