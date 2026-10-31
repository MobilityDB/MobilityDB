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
 * @brief A program that checks that the nearest approach distance of a
 * temporal number and a temporal box refuses every invalid pair of arguments
 * @details Each of #nad_tint_tbox, #nad_tbigint_tbox and #nad_tfloat_tbox
 * receives a valid pair, which it answers with no error, and the invalid ones:
 * a NULL box, a box with no value dimension, a box of another number, and a
 * temporal number of another type; each of these must report an error and
 * answer the error value of the function. Every call runs in a child process,
 * so a call that crashes is counted as a failure instead of ending the run.
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tnumber_tbox_validity_test tnumber_tbox_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <float.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <meos.h>

enum fn { FN_TINT, FN_TBIGINT, FN_TFLOAT };
static const char *FN_NAME[] = { "nad_tint_tbox", "nad_tbigint_tbox",
  "nad_tfloat_tbox" };

static int failures;

/* Return 1 when the call reports an error and answers the error value of the
 * function, 0 when it answers with no error, 2 otherwise */
static int
call(enum fn f, const Temporal *temp, const TBox *box)
{
  meos_errno_reset();
  bool errval;
  switch (f)
  {
    case FN_TINT:
      errval = nad_tint_tbox(temp, box) == INT_MAX;
      break;
    case FN_TBIGINT:
      errval = nad_tbigint_tbox(temp, box) == INT64_MAX;
      break;
    default:
      errval = nad_tfloat_tbox(temp, box) == DBL_MAX;
  }
  bool error = meos_errno() != 0;
  if (! error)
    return 0;
  return errval ? 1 : 2;
}

/* Run a call in a child process and count it as a failure unless it answers
 * as expected: an error with the error value, or no error */
static void
check(const char *what, enum fn f, const Temporal *temp, const TBox *box,
  bool expect_error)
{
  fflush(stdout);
  pid_t pid = fork();
  if (pid == 0)
    _exit(call(f, temp, box));
  int status;
  waitpid(pid, &status, 0);
  const char *answer;
  bool ok;
  if (! WIFEXITED(status))
  {
    answer = "crashed";
    ok = false;
  }
  else if (WEXITSTATUS(status) == 1)
  {
    answer = "error with the error value";
    ok = expect_error;
  }
  else if (WEXITSTATUS(status) == 0)
  {
    answer = "no error";
    ok = ! expect_error;
  }
  else
  {
    answer = "error without the error value";
    ok = false;
  }
  printf("%s(%s): %s%s\n", FN_NAME[f], what, answer, ok ? "" : "  FAILED");
  if (! ok)
    failures++;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp[] = {
    tint_in("[0@2001-01-01, 5@2001-01-03]"),
    tbigint_in("[0@2001-01-01, 5@2001-01-03]"),
    tfloat_in("[0@2001-01-01, 5@2001-01-03]") };
  TBox *own[] = {
    tbox_in("TBOXINT XT([10,12),[2001-01-01,2001-01-03])"),
    tbox_in("TBOXBIGINT XT([10,12),[2001-01-01,2001-01-03])"),
    tbox_in("TBOXFLOAT XT([10,12],[2001-01-01,2001-01-03])") };
  TBox *timeonly = tbox_in("TBOX T([2001-01-01,2001-01-03])");

  for (int f = FN_TINT; f <= FN_TFLOAT; f++)
  {
    /* Another number, for the box and for the temporal number */
    int other = (f + 1) % 3;
    check("valid", f, temp[f], own[f], false);
    check("NULL box", f, temp[f], NULL, true);
    check("box without value dimension", f, temp[f], timeonly, true);
    check("box of another number", f, temp[f], own[other], true);
    check("temporal number of another type", f, temp[other], own[other],
      true);
  }

  for (int i = 0; i < 3; i++)
  {
    free(temp[i]);
    free(own[i]);
  }
  free(timeonly);

  printf("%d failures\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
