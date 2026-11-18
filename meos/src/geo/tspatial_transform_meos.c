/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Cache of the PROJ transformations of MEOS, kept per thread, the systems read
 * from the file spatial_ref_sys.csv
 */

#include "geo/meos_transform.h"

/* C */
#include <string.h>
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "geo/tspatial_transform_cache.h"

/* Global variable to hold the Proj object cache */
static MEOS_TLS MEOSPROJSRSCache *MEOS_PROJ_CACHE = NULL;
/* File the systems of the Proj object cache were read from */
static MEOS_TLS char *MEOS_PROJ_CACHE_PATH = NULL;

/*****************************************************************************
 * Definitions for reading the spatial_ref_sys.csv file
 *****************************************************************************/

/* Maximum length in characters of a header record in the input CSV file */
#define MAX_LEN_HEADER 1024
/* Maximum length in characters of a geometry in the input data */
#define MAX_LEN_SRS_RECORD 5120
/* Location of the spatial_ref_sys.csv file, the one the build installs */
char *SPATIAL_REF_SYS_CSV = MEOS_DATAROOTDIR "/spatial_ref_sys.csv";
/* Copy of the path given to the last call of
 * `meos_set_spatial_ref_sys_csv()` */
static char *SPATIAL_REF_SYS_CSV_COPY = NULL;

/**
 * @ingroup meos_setup
 * @brief Set the location of the SPATIAL_REF_SYS_CSV files
 * @details The function keeps a copy of the path and frees the copy it kept
 * for an earlier call, as #meos_set_ways_csv does. A null path is refused and
 * the path kept before stays in use.
 * @param[in] path Full path to the spatial reference systems CSV file
 */
void
meos_set_spatial_ref_sys_csv(const char* path)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null(path))
    return;
  char *copy = malloc(strlen(path) + 1);
  if (! copy)
  {
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the path of the spatial reference systems file");
    return;
  }
  strcpy(copy, path);
  free(SPATIAL_REF_SYS_CSV_COPY);
  SPATIAL_REF_SYS_CSV_COPY = copy;
  SPATIAL_REF_SYS_CSV = copy;
}

/**
 * @brief Get the Proj cache entry from the global variable if one exists
 * @details If it doesn't exist, or holds the systems of a file other than
 * the one #meos_set_spatial_ref_sys_csv names last, make a new blank one and
 * return it, as GetWaysCache of ways_meos.c makes its cache anew: the systems
 * of a file answer for that file alone. The path of the file is held per
 * thread with the cache, so each thread's cache follows the file it reads.
 */
MEOSPROJSRSCache *
GetMEOSPROJSRSCache(void)
{
  if (MEOS_PROJ_CACHE && MEOS_PROJ_CACHE_PATH &&
      strcmp(MEOS_PROJ_CACHE_PATH, SPATIAL_REF_SYS_CSV) == 0)
    return MEOS_PROJ_CACHE;
  char *path = malloc(strlen(SPATIAL_REF_SYS_CSV) + 1);
  if (! path)
  {
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the path of the Proj object cache");
    return NULL;
  }
  strcpy(path, SPATIAL_REF_SYS_CSV);
  /* Free the systems and the path of another file */
  meos_finalize_projsrs();
  MEOS_PROJ_CACHE_PATH = path;
  MEOSPROJSRSCache* cache = MEOS_PROJ_CACHE;
  if (! cache)
  {
    /* Allocate memory */
    cache = palloc(sizeof(MEOSPROJSRSCache));
    if (! cache)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unable to allocate space for MEOSPROJSRSCache");
      return NULL;
    }
    cache->PROJSRSCacheCount = 0;
    MEOS_PROJ_CACHE = cache;
  }
  return cache;
}
/**
 * @ingroup meos_setup
 * @brief Destroy all the malloc'ed PROJ objects stored in the PROJSRSCache
 */
void
meos_finalize_projsrs(void)
{
  /* Idempotency: only destroy a live cache, and null the slot so a
   * second finalize call does not double-free the cache and its
   * stored projections. */
  free(MEOS_PROJ_CACHE_PATH);
  MEOS_PROJ_CACHE_PATH = NULL;
  MEOSPROJSRSCache *cache = MEOS_PROJ_CACHE;
  if (! cache)
    return;
  for (uint32_t i = 0; i < cache->PROJSRSCacheCount; i++)
  {
    if (cache->MEOSPROJSRSCache[i].projection)
      PROJSRSDestroyPJ(cache->MEOSPROJSRSCache[i].projection);
    if (cache->MEOSPROJSRSCache[i].authtext)
      pfree(cache->MEOSPROJSRSCache[i].authtext);
    if (cache->MEOSPROJSRSCache[i].urn)
      pfree(cache->MEOSPROJSRSCache[i].urn);
  }
  pfree(cache);
  MEOS_PROJ_CACHE = NULL;
  return;
}

/**
 * @brief Make the entries of the Proj object cache, which MEOS allocates
 * where it allocates any value
 */
void *
proj_cache_enter(void)
{
  return NULL;
}

/**
 * @brief Go back after #proj_cache_enter, which MEOS needs not
 */
void
proj_cache_leave(void *state)
{
  (void) state;
  return;
}

/**
 * @brief Read one comma-separated field into a buffer and advance past it
 * @details A field carrying a comma is quoted and a quote inside such a field
 * is written twice, so a reader that stops at the next comma delivers part of
 * a field: a proj4 string states a datum shift as @p +towgs84=dx,dy,dz and
 * carries three commas of its own.
 * @param[in,out] pos Position in the row, left after the field's separator
 * @param[out] buf Buffer receiving the field, unquoted and terminated
 * @param[in] bufsize Size of @p buf
 * @return True if the field is read, false where it does not fit
 */
static bool
csv_read_field(char **pos, char *buf, size_t bufsize)
{
  char *p = *pos;
  size_t n = 0;
  bool quoted = (*p == '"');
  if (quoted)
    p++;
  while (*p)
  {
    if (quoted && *p == '"')
    {
      /* A doubled quote stands for one quote, a single one closes the field */
      if (*(p + 1) != '"')
      {
        p++;
        break;
      }
      p++;
    }
    else if (! quoted && (*p == ',' || *p == '\n' || *p == '\r'))
      break;
    if (n + 1 >= bufsize)
      return false;
    buf[n++] = *p++;
  }
  buf[n] = '\0';
  /* Leave the position after the comma that closes the field */
  while (*p == '\r')
    p++;
  if (*p == ',')
    p++;
  *pos = p;
  return true;
}

PjStrs
GetProjStringsSPI(int32_t srid)
{
  char header_buffer[MAX_LEN_HEADER];
  char auth_name[256];
  char auth_srid_text[256];
  int32_t auth_srid;
  char proj4text[2048];
  char srtext[2048];

  PjStrs strs; // TODO palloc ?
  memset(&strs, 0, sizeof(strs));

  /* Substitute the full file path in the first argument of fopen */
  FILE *file = fopen(SPATIAL_REF_SYS_CSV, "r");
  if (! file)
  {
    printf("Cannot open the spatial_ref_sys.csv file (reading from %s)\n", SPATIAL_REF_SYS_CSV);
    return strs;
  }

  /* Read and discard the first line of the file with the headers */
  int nread = fscanf(file, "%1023s\n", header_buffer);
  (void) nread;

  /* Continue reading the file */
  bool found = false;
  char line[MAX_LEN_SRS_RECORD];
  while (fgets(line, sizeof(line), file))
  {
    /* A record longer than the buffer would be read as two, and its halves
     * parsed as two malformed records, so discard the remainder of the line */
    size_t linelen = strlen(line);
    if (linelen == sizeof(line) - 1 && line[linelen - 1] != '\n')
    {
      int c;
      while ((c = fgetc(file)) != '\n' && c != EOF)
        ;
      continue;
    }

    /* Read each line from the file as the four fields of a record */
    char *pos = line;
    if (! csv_read_field(&pos, auth_name, sizeof(auth_name)) ||
        ! csv_read_field(&pos, auth_srid_text, sizeof(auth_srid_text)) ||
        ! csv_read_field(&pos, proj4text, sizeof(proj4text)) ||
        ! csv_read_field(&pos, srtext, sizeof(srtext)))
      continue;
    auth_srid = atoi(auth_srid_text);

    if (auth_srid == srid)
    {
      char tmp[MAX_PROJ_LEN];
      snprintf(tmp, MAX_PROJ_LEN, "%s:%d", auth_name, auth_srid);
      strs.authtext = strdup(tmp);
      strs.proj4text = strdup(proj4text);
      strs.srtext = strdup(srtext);
      found = true;
      break;
    }
  }

  if (ferror(file))
  {
    printf("Error reading the spatial_ref_sys.csv file");
    fclose(file);
    return strs;
  }

  if (! found)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Cannot find SRID (%d) in spatial_ref_sys", srid);
  }
  fclose(file);
  return strs;
}
