/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @brief The time zone database compiled into standalone MEOS.
 *
 * PostgreSQL installs the zone files it compiles from its own copy of the
 * IANA data and reads them from its share directory. Standalone MEOS has no
 * installation directory of its own, so it carries the same data, compiled
 * from the tzdata.zi of the PostgreSQL release it tracks, inside the library:
 * a zone is found by name, case-insensitively as #pg_open_tzfile finds it,
 * and its bytes are the file zic writes for it. The data is the same on every
 * platform, whether or not the host has a zone directory.
 */

#include "postgres.h"

#include <string.h>

#include "pgtz.h"

typedef struct
{
  const char *name;  /* canonical spelling of the zone name */
  uint32 offset;     /* start of the zone in tzdata_blob */
  uint32 size;       /* size of the zone in tzdata_blob */
} TzdataEntry;

#include "data/tzdata_embedded.inc"

/* Compare two zone names ignoring ASCII case, the order of tzdata_entries */
static int
tzdata_name_cmp(const char *a, const char *b)
{
  for (;; a++, b++)
  {
    unsigned char ca = (unsigned char) *a;
    unsigned char cb = (unsigned char) *b;
    if (ca >= 'A' && ca <= 'Z')
      ca += 'a' - 'A';
    if (cb >= 'A' && cb <= 'Z')
      cb += 'a' - 'A';
    if (ca != cb)
      return (int) ca - (int) cb;
    if (ca == '\0')
      return 0;
  }
}

/**
 * @brief Copy the compiled zone of the given name into a buffer
 * @param[in] name Zone name, matched case-insensitively
 * @param[out] canonname If not NULL, receives the canonical spelling of the
 * name (the buffer must be larger than TZ_STRLEN_MAX bytes)
 * @param[out] buf Buffer receiving the zone
 * @param[in] bufsize Size of the buffer
 * @return The number of bytes copied, or -1 when the database has no zone of
 * that name or the zone does not fit in the buffer
 */
int
pg_tzdata_embedded_read(const char *name, char *canonname, char *buf,
  size_t bufsize)
{
  int lo = 0, hi = (int) lengthof(tzdata_entries) - 1;
  while (lo <= hi)
  {
    int mid = lo + (hi - lo) / 2;
    const TzdataEntry *entry = &tzdata_entries[mid];
    int cmp = tzdata_name_cmp(name, entry->name);
    if (cmp < 0)
      hi = mid - 1;
    else if (cmp > 0)
      lo = mid + 1;
    else
    {
      if (entry->size > bufsize)
        return -1;
      memcpy(buf, tzdata_blob + entry->offset, entry->size);
      if (canonname)
      {
        size_t len = strlen(entry->name);
        if (len > TZ_STRLEN_MAX)
          len = TZ_STRLEN_MAX;
        memcpy(canonname, entry->name, len);
        canonname[len] = '\0';
      }
      return (int) entry->size;
    }
  }
  return -1;
}

/**
 * @brief Return the number of zone names in the database
 */
int
pg_tzdata_embedded_count(void)
{
  return (int) lengthof(tzdata_entries);
}

/**
 * @brief Return the canonical spelling of the i-th zone name of the database
 */
const char *
pg_tzdata_embedded_name(int i)
{
  return tzdata_entries[i].name;
}
