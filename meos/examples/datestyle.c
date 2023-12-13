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
 * @brief A simple program that changes the DateStyle parameter of the MEOS
 * library.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o datestyle datestyle.c -L/usr/local/lib -lmeos
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

  DateADT d = pg_date_in("2020-01-01");

  char *datestyle = meos_get_datestyle();
  printf("Default DateStyle: %s\n", datestyle);
  free(datestyle);

  char *d_out = pg_date_out(d);
  printf("Date: %s\n", d_out);
  free(d_out);

  bool ok = meos_set_datestyle("Postgres, YMD", NULL);
  if (! ok)
  {
    printf("Error while setting the DateStyle\n");
    return 0;
  }
  datestyle = meos_get_datestyle();
  printf("New DateStyle: %s\n", datestyle);
  free(datestyle);
  
  d_out = pg_date_out(d);
  printf("Date: %s\n", d_out);
  free(d_out);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return 0;
}
