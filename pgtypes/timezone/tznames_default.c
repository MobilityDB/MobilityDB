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
 * @brief The time zone abbreviations of standalone MEOS.
 *
 * A PostgreSQL server reads the abbreviations a timestamp may carry from the
 * file its timezone_abbreviations setting names, Default unless set otherwise.
 * Standalone MEOS has neither that setting nor a share directory, so it carries
 * PostgreSQL's Default set inside the library and installs it, as the server
 * does, through #ConvertTimeZoneAbbrevs and #InstallTimeZoneAbbrevs. The table
 * is shared by every thread: the first thread that initializes its time zone
 * installs it, and the last thread that finalizes its time zone frees it,
 * through #UninstallTimeZoneAbbrevs.
 */

#include "postgres.h"

#include "utils/datetime.h"
#include "utils/tzparser.h"
#include "pgtz.h"
#include "../../meos/include/meos_tls.h"  /* MEOS: MEOS_TLS */

#include "data/tznames_default.inc"

/* Lock over the count below and the table, held for an installation or a
 * release, both short-lived */
static int32 tznames_lock = 0;
/* Number of threads holding the table */
static int32 tznames_users = 0;
/* Whether this thread holds the table, so that initializing or finalizing
 * its time zone twice counts once */
static MEOS_TLS bool tznames_held = false;

static void
tznames_lock_acquire(void)
{
  int32 expected = 0;
  while (! __atomic_compare_exchange_n(&tznames_lock, &expected, 1, false,
      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
    expected = 0;
}

static void
tznames_lock_release(void)
{
  __atomic_store_n(&tznames_lock, 0, __ATOMIC_RELEASE);
}

/**
 * @brief Install PostgreSQL's Default time zone abbreviations for this thread,
 * the first thread installing the table every thread shares
 * @return False when the table cannot be allocated
 */
bool
pg_tznames_install_default(void)
{
  if (tznames_held)
    return true;
  tznames_lock_acquire();
  if (tznames_users == 0)
  {
    TimeZoneAbbrevTable *tbl = ConvertTimeZoneAbbrevs(tznames_default,
      (int) lengthof(tznames_default));
    if (! tbl)
    {
      tznames_lock_release();
      return false;
    }
    InstallTimeZoneAbbrevs(tbl);
  }
  tznames_users++;
  tznames_lock_release();
  tznames_held = true;
  return true;
}

/**
 * @brief Release the time zone abbreviations this thread holds, the last
 * thread freeing the table every thread shares
 */
void
pg_tznames_release_default(void)
{
  if (! tznames_held)
    return;
  /* This thread's cache of abbreviations points into the table */
  ClearTimeZoneAbbrevCache();
  tznames_lock_acquire();
  if (--tznames_users == 0)
    UninstallTimeZoneAbbrevs();
  tznames_lock_release();
  tznames_held = false;
  return;
}
