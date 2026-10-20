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
 * @brief Test the average value of a temporal number
 * @details A program that tests the public function #tnumber_avg_value, which
 * answers the SQL function `avgValue`, under the noexit error handler.
 *
 * The average value of a temporal instant is its value, that of a temporal
 * sequence the average of the values of its instants, and that of a temporal
 * sequence set the average of the averages of its sequences. The program
 * verifies, for the temporal integers, big integers and floats of the
 * regression tests of `avgValue`, in every subtype and interpolation, that
 * the function answers the value the internal functions of each subtype
 * answer, and that a null value and a temporal Boolean are refused with the
 * error value DBL_MAX and the error codes #MEOS_ERR_INVALID_ARG and
 * #MEOS_ERR_INVALID_ARG_TYPE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o avg_value_test avg_value_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_internal.h>

static int failures = 0;

/* Report a check and count it when it fails */
static void
check(const char *what, bool ok)
{
  printf("%s: %s\n", ok ? "ok  " : "FAIL", what);
  if (! ok)
    failures++;
  meos_errno_reset();
}

/* The average value answered by the internal function of the subtype */
static double
internal_avg_value(const Temporal *temp)
{
  if (temp->subtype == TINSTANT)
  {
    if (temp->temptype == T_TINT)
      return (double) tint_start_value(temp);
    if (temp->temptype == T_TBIGINT)
      return (double) tbigint_start_value(temp);
    return tfloat_start_value(temp);
  }
  if (temp->subtype == TSEQUENCE)
    return tnumberseq_avg_val((const TSequence *) temp);
  return tnumberseqset_avg_val((const TSequenceSet *) temp);
}

/* A temporal number whose average value must be the one of the internal
 * functions and the expected one */
static void
check_avg(const char *str, Temporal *temp, double expected)
{
  double avg = temp ? tnumber_avg_value(temp) : DBL_MAX;
  char what[256];
  snprintf(what, sizeof(what), "tnumber_avg_value(%s)", str);
  check(what, temp && meos_errno() == MEOS_SUCCESS &&
    avg == internal_avg_value(temp) && avg == expected);
  free(temp);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The temporal numbers of 022_temporal.test.sql */
  const char *tint_str[] = {
    "1@2001-01-01",
    "{1@2001-01-01, 2@2001-01-02, 1@2001-01-03}",
    "[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]",
    "{[1@2001-01-01, 2@2001-01-02, 1@2001-01-03],[3@2001-01-04, 3@2001-01-05]}"
  };
  const double tint_avg[] = {1.0, 4.0 / 3.0, 4.0 / 3.0,
    (4.0 / 3.0 + 3.0) / 2.0};
  const char *tfloat_str[] = {
    "1.5@2001-01-01",
    "{1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03}",
    "[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]",
    "Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03]",
    "{[1.5@2001-01-01, 2.5@2001-01-02, 1.5@2001-01-03],[3.5@2001-01-04, 3.5@2001-01-05]}"
  };
  const double tfloat_avg[] = {1.5, 5.5 / 3.0, 5.5 / 3.0, 5.5 / 3.0,
    (5.5 / 3.0 + 3.5) / 2.0};

  for (int i = 0; i < 4; i++)
  {
    char what[128];
    snprintf(what, sizeof(what), "tint '%s'", tint_str[i]);
    check_avg(what, tint_in(tint_str[i]), tint_avg[i]);
    snprintf(what, sizeof(what), "tbigint '%s'", tint_str[i]);
    check_avg(what, tbigint_in(tint_str[i]), tint_avg[i]);
  }
  for (int i = 0; i < 5; i++)
  {
    char what[128];
    snprintf(what, sizeof(what), "tfloat '%s'", tfloat_str[i]);
    check_avg(what, tfloat_in(tfloat_str[i]), tfloat_avg[i]);
  }

  /* A null value and a temporal Boolean are refused */
  double avg = tnumber_avg_value(NULL);
  check("tnumber_avg_value(NULL) is refused",
    avg == DBL_MAX && meos_errno() == MEOS_ERR_INVALID_ARG);
  Temporal *tbool = tbool_in("[t@2001-01-01, f@2001-01-02, t@2001-01-03]");
  avg = tnumber_avg_value(tbool);
  check("tnumber_avg_value(tbool) is refused",
    avg == DBL_MAX && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  free(tbool);

  meos_finalize();

  printf("%d check(s) failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
