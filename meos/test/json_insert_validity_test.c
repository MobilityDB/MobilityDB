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
 * @brief A program that tests how the functions inserting a value into a
 * temporal JSONB and into a JSONB set of the MEOS API report a null value
 * under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. The value to insert is such a pointer, as
 * it is for #tjsonb_set and #jsonbset_set, while #tjsonb_set_lax and
 * #jsonbset_set_lax take a null value and read what it means from their
 * null handling argument.
 *
 * The program verifies that #tjsonb_insert and #jsonbset_insert report a
 * null value by answering NULL and setting #meos_errno, and that each
 * inserts a valid value with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_insert_validity_test json_insert_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

/* Check that a call reports a null value and inserts a valid one */
#define CHECK_INSERT(call_null, call_valid) \
  do { \
    const void *none = (const void *) (call_null); \
    printf("%s: %s, errno %d\n", #call_null, none ? "a value" : "NULL", \
      meos_errno()); \
    assert(none == NULL); \
    assert(meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    const void *some = (const void *) (call_valid); \
    printf("%s: %s, errno %d\n", #call_valid, some ? "a value" : "NULL", \
      meos_errno()); \
    assert(some != NULL); \
    assert(meos_errno() == 0); \
    free((void *) some); \
  } while (0)

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp = tjsonb_in("[{\"a\":1}@2001-01-01, {\"a\":2}@2001-01-02]");
  Set *set = jsonbset_in("{\"{\\\"a\\\":1}\"}");
  Jsonb *jb = jsonb_in("2");
  text *key = text_in("b");
  assert(temp); assert(set); assert(jb); assert(key);

  CHECK_INSERT(tjsonb_insert(temp, &key, 1, NULL, false),
    tjsonb_insert(temp, &key, 1, jb, false));
  CHECK_INSERT(jsonbset_insert(set, &key, 1, NULL, false),
    jsonbset_insert(set, &key, 1, jb, false));

  free(key);
  free(jb);
  free(set);
  free(temp);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
