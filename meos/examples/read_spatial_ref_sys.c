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
 * @brief A simple program that reads the `spatial_ref_sys.csv` file obtained
 * by exporting the PostGIS `spatial_ref_sys` table in CSV format
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o read_spatial_ref_sys read_spatial_ref_sys.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <string.h>   /* for memset */
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* for free */
/* Include the MEOS API header */
#include <meos.h>
#include <meos_geo.h>

#define MAX_PROJ_LEN  512

/*****************************************************************************
 * Definitions for reading the spatial_ref_sys.csv file
 *****************************************************************************/

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_HEADER 1024
/* Location of the spatial_ref_sys.csv file */
#define SPATIAL_REF_SYS "/usr/local/share/spatial_ref_sys.csv"

/**
 * @brief Utility structure to get many potential string representations
 * from spatial_ref_sys query.
 */
typedef struct
{
  char *authtext; /* auth_name:auth_srid */
  char *srtext;
  char *proj4text;
} PjStrs;

/**
 * @brief 
 */
static PjStrs
GetProjStringsSPI(int32_t srid)
{
  char header_buffer[MAX_LEN_HEADER];
  char auth_name[256];
  int32_t auth_srid;
  char proj4text[2048];
  char srtext[2048];

  PjStrs strs;
  memset(&strs, 0, sizeof(strs));

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(SPATIAL_REF_SYS, "r");

  if (! file)
  {
    printf("Cannot open the spatial_ref_sys.csv file");
    return strs;
  }

  bool found = false;

  /* Read the first line of the file with the headers */
  fscanf(file, "%1023s\n", header_buffer);

  /* Continue reading the file */
  do
  {
    /* Read each line from the file */
    int read = fscanf(file, "%255[^,^\n],%d,%2047[^,^\n],%2047[^\n]\n",
      auth_name, &auth_srid, proj4text, srtext);
    if (ferror(file))
    {
      printf("Error reading the spatial_ref_sys.csv file");
      return strs;
    }

    /* Ignore the records with NULL values */
    if (read == 4 && auth_srid == srid)
    {
      char tmp[MAX_PROJ_LEN];
      snprintf(tmp, MAX_PROJ_LEN, "%s:%d", auth_name, auth_srid);
      strs.authtext = strdup(tmp);
      strs.proj4text = strdup(proj4text);
      strs.srtext = strdup(srtext);
      found = true;
      break;
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);
  
  if (found)
    printf("SRID (%d) found in spatial_ref_sys !\n", srid);
  else
    printf("Cannot find SRID (%d) in spatial_ref_sys\n", srid);

  return strs;
}

int main()
{
  /* Initialize MEOS */
  meos_initialize();
  int32_t srid = 3857;
  PjStrs srs = GetProjStringsSPI(srid);

  /* Output the information found */
  if (strlen(srs.authtext))
  {
    printf("authtext: %s\n", srs.authtext);
    printf("srtext: %s\n", srs.srtext);
    printf("proj4text: %s\n", srs.proj4text);
  }

  /* Finalize MEOS */
  meos_finalize();

  /* Free memory */
  free(srs.authtext);
  free(srs.proj4text);
  free(srs.srtext);

  /* Return */
  return EXIT_SUCCESS;
}

/*****************************************************************************/