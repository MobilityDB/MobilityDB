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
 * @brief A program that tests the typed text input and output of the cell
 * index sets
 * @details Each set type gives its text form through a typed public function
 * that
 * tests its argument, as intset_out does for an integer set. The program
 * verifies that quadbinset_out, s2cellset_out and h3indexset_out answer the
 * text their input reads back with no error left behind, that each reports a
 * null set with MEOS_ERR_INVALID_ARG and an integer set with
 * MEOS_ERR_INVALID_ARG_TYPE, and that quadbinset_in, s2cellset_in and
 * h3indexset_in report a null string with MEOS_ERR_INVALID_ARG.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cellset_text_io_test cellset_text_io_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

typedef Set *(*text_in_fn)(const char *);
typedef char *(*text_out_fn)(const Set *);

/* Read a set, write it out, read the output back and write it again: the
 * two outputs agree, the first reading back what the input reads. Then the
 * output reports a null set and a set of another type, and the input a null
 * string. */
static void
check(const char *name, text_in_fn in, text_out_fn out, const char *text,
  const Set *other)
{
  Set *s = in(text);
  assert(s && meos_errno() == 0);
  char *str1 = out(s);
  Set *back = str1 ? in(str1) : NULL;
  char *str2 = back ? out(back) : NULL;
  printf("%s: %s, errno %d\n", name, str1 ? str1 : "NULL", meos_errno());
  assert(str1 && str2 && strcmp(str1, str2) == 0);
  assert(meos_errno() == 0);
  free(s); free(back); free(str1); free(str2);

  char *res = out(NULL);
  printf("%s(NULL): %s, errno %d\n", name, res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  res = out(other);
  printf("%s(intset): %s, errno %d\n", name, res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  Set *set = in(NULL);
  printf("%s input(NULL): %s, errno %d\n", name, set ? "a value" : "NULL",
    meos_errno());
  assert(set == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
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

  Set *intset = intset_in("{1, 2}");
  assert(intset);

  check("quadbinset_out", quadbinset_in, quadbinset_out,
    "{480fffffffffffff, 48427fffffffffff}", intset);
  check("s2cellset_out", s2cellset_in, s2cellset_out,
    "{47c3c3, 54b5c9}", intset);
  check("h3indexset_out", h3indexset_in, h3indexset_out,
    "{880326b885fffff, 880326b88dfffff}", intset);

  free(intset);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
