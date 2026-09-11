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
 * @brief The time zone abbreviations of standalone MEOS.
 *
 * A PostgreSQL server reads the abbreviations a timestamp may carry from the
 * file its timezone_abbreviations setting names, Default unless set otherwise.
 * Standalone MEOS has neither that setting nor a share directory, so it carries
 * PostgreSQL's Default set inside the library and installs it, as the server
 * does, through #ConvertTimeZoneAbbrevs and #InstallTimeZoneAbbrevs. The table
 * is shared by every thread and installed once, by the first thread that
 * initializes its time zone.
 */

#include "postgres.h"

#include "utils/datetime.h"
#include "utils/tzparser.h"
#include "pgtz.h"

#include "data/tznames_default.inc"

/* 0: not installed, 1: being installed, 2: installed */
static int tznames_state = 0;

/**
 * @brief Install PostgreSQL's Default time zone abbreviations, once per process
 * @return False when the table cannot be allocated
 */
bool
pg_tznames_install_default(void)
{
  if (__atomic_load_n(&tznames_state, __ATOMIC_ACQUIRE) == 2)
    return true;
  int expected = 0;
  if (__atomic_compare_exchange_n(&tznames_state, &expected, 1, false,
        __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    /* This thread installs the table; concurrent threads wait below */
    TimeZoneAbbrevTable *tbl = ConvertTimeZoneAbbrevs(tznames_default,
      (int) lengthof(tznames_default));
    if (! tbl)
    {
      __atomic_store_n(&tznames_state, 0, __ATOMIC_RELEASE);
      return false;
    }
    InstallTimeZoneAbbrevs(tbl);
    __atomic_store_n(&tznames_state, 2, __ATOMIC_RELEASE);
    return true;
  }
  /* Another thread is installing; the installation is short-lived */
  while ((expected = __atomic_load_n(&tznames_state, __ATOMIC_ACQUIRE)) == 1)
    ;
  return expected == 2;
}
