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
 * @brief A program that tests that a lifted operation answers NULL when the
 * function it applies raises an error for one of the values
 * @details Under #meos_initialize_noexit_error_handler a function that raises
 * returns its failure value, and the operation lifting it over a temporal
 * value or a set must not build its answer from that value. Each operation
 * is called once with a value for which the function raises, which must
 * answer NULL and report an error through meos_errno, and once with values
 * for which it does not, which must answer and report none: the exponential
 * of a temporal float, the product of a temporal float and a float, the
 * product of two temporal floats, the length of the arrays of a JSONB set
 * and the match of a JSON path over a JSONB set. The sum of a temporal
 * integer and an integer overflowing in a sequence, in a step sequence with
 * an exclusive upper bound and in a sequence set must report the code of the
 * overflow. An answer also leaves an error raised before the call to be read.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o lifting_error_test lifting_error_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

static int failures;

/* Report whether a call answered and reported an error, and count an
 * unexpected answer */
static void
liftcheck_report(const char *what, void *result, bool expect_error)
{
  bool error = meos_errno() != 0;
  printf("%s: %s, %s\n", what, result ? "answer" : "no answer",
    error ? "error reported" : "no error");
  if (error != expect_error || (result == NULL) != expect_error)
  {
    printf("  expected %s\n", expect_error ? "no answer and an error" :
      "an answer and no error");
    failures++;
  }
  free(result);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  meos_errno_reset();

  /* A function applied to each instant */
  Temporal *expbig = tfloat_in("[1@2001-01-01, 1000@2001-01-02]");
  Temporal *expsmall = tfloat_in("[1@2001-01-01, 2@2001-01-02]");
  liftcheck_report("exp of a sequence reaching 1000", tfloat_exp(expbig),
    true);
  liftcheck_report("exp of a sequence below it", tfloat_exp(expsmall), false);

  /* A function applied to each instant and a base value */
  Temporal *floatbig = tfloat_in("[1@2001-01-01, 1e308@2001-01-02]");
  Temporal *floatsmall = tfloat_in("[1@2001-01-01, 10@2001-01-02]");
  liftcheck_report("sequence * 10 overflowing", mul_tfloat_float(floatbig, 10),
    true);
  liftcheck_report("sequence * 10", mul_tfloat_float(floatsmall, 10), false);

  /* A function applied to each pair of synchronized instants */
  liftcheck_report("sequence * sequence overflowing",
    mul_tnumber_tnumber(floatbig, floatsmall), true);
  liftcheck_report("sequence * sequence",
    mul_tnumber_tnumber(floatsmall, floatsmall), false);

  /* A function applied to each element of a set */
  Set *scalar = jsonbset_in("{\"[1,2]\", \"3\"}");
  Set *arrays = jsonbset_in("{\"[1,2]\", \"[3]\"}");
  liftcheck_report("array length of a set holding a scalar",
    jsonbset_array_length(scalar), true);
  liftcheck_report("array length of a set of arrays",
    jsonbset_array_length(arrays), false);

  /* A predicate applied to each element of a set */
  JsonPath *root = jsonpath_in("$");
  Jsonb *novars = jsonb_in("{}");
  Set *mixed = jsonbset_in("{\"true\", \"3\"}");
  Set *booleans = jsonbset_in("{\"true\", \"false\"}");
  int count = 0;
  liftcheck_report("path match of a set holding a number",
    jsonbset_path_match(mixed, root, novars, false, false, &count), true);
  liftcheck_report("path match of a set of Booleans",
    jsonbset_path_match(booleans, root, novars, false, false, &count), false);

  /* The error reported is the one the function raised: an integer sum
   * overflowing in a sequence, in a step sequence whose exclusive upper bound
   * repeats the value overflowing and in a sequence set */
  const char *intbig[] = {
    "[1@2001-01-01, 2147483640@2001-01-02]",
    "[1@2001-01-01, 2147483640@2001-01-02, 2147483640@2001-01-03)",
    "{[1@2001-01-01], [2147483640@2001-01-02]}"};
  for (int i = 0; i < 3; i++)
  {
    Temporal *intseq = tint_in(intbig[i]);
    Temporal *intsum = add_tint_int(intseq, 100);
    int code = meos_errno();
    printf("%s + 100: %s, errno %d\n", intbig[i],
      intsum ? "answer" : "no answer", code);
    if (intsum || code != MEOS_ERR_VALUE_OUT_OF_RANGE)
      failures++;
    free(intseq); free(intsum);
    meos_errno_reset();
  }
  Temporal *intsmall = tint_in("[1@2001-01-01, 5@2001-01-02]");
  liftcheck_report("integer sequence + 100", add_tint_int(intsmall, 100),
    false);

  /* An answer leaves an error raised before the call to be read */
  meos_errno_set(MEOS_ERR_INVALID_ARG);
  Temporal *exppending = tfloat_exp(expsmall);
  int pending = meos_errno();
  printf("exp with an error pending: %s, errno %d\n",
    exppending ? "answer" : "no answer", pending);
  if (! exppending || pending != MEOS_ERR_INVALID_ARG)
    failures++;
  free(exppending);
  meos_errno_reset();

  free(expbig); free(expsmall); free(floatbig); free(floatsmall);
  free(scalar); free(arrays); free(root); free(novars); free(mixed);
  free(booleans); free(intsmall);

  printf("%d unexpected answers\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
