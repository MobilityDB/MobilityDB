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
 * @brief A program that tests how the functions of the MEOS API taking a
 * text value beside a temporal text report a null text under the noexit
 * error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. The internal forms of these functions read
 * the text as a datum of any base type, so the public function is the one
 * that can test it.
 *
 * The program verifies that the temporal comparisons #teq_text_ttext,
 * #tne_text_ttext, #tlt_text_ttext, #tle_text_ttext, #tgt_text_ttext,
 * #tge_text_ttext and their forms with the text second, the restrictions
 * #ttext_at_value and #ttext_minus_value, and the constructor
 * #ttextseq_from_base_tstzspan report a null text by answering NULL and
 * setting #meos_errno, and that each answers a valid text with no error left
 * behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ttext_base_validity_test ttext_base_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Check that a call reports a null text and answers a valid one */
#define CHECK_TEXT(call_null, call_valid) \
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

  Temporal *temp = ttext_in("[AAA@2001-01-01, BBB@2001-01-02]");
  Span *s = tstzspan_in("[2001-01-01, 2001-01-02]");
  text *txt = text_in("AAA");
  assert(temp); assert(s); assert(txt);

  CHECK_TEXT(teq_text_ttext(NULL, temp), teq_text_ttext(txt, temp));
  CHECK_TEXT(tne_text_ttext(NULL, temp), tne_text_ttext(txt, temp));
  CHECK_TEXT(tlt_text_ttext(NULL, temp), tlt_text_ttext(txt, temp));
  CHECK_TEXT(tle_text_ttext(NULL, temp), tle_text_ttext(txt, temp));
  CHECK_TEXT(tgt_text_ttext(NULL, temp), tgt_text_ttext(txt, temp));
  CHECK_TEXT(tge_text_ttext(NULL, temp), tge_text_ttext(txt, temp));
  CHECK_TEXT(teq_ttext_text(temp, NULL), teq_ttext_text(temp, txt));
  CHECK_TEXT(tne_ttext_text(temp, NULL), tne_ttext_text(temp, txt));
  CHECK_TEXT(tlt_ttext_text(temp, NULL), tlt_ttext_text(temp, txt));
  CHECK_TEXT(tle_ttext_text(temp, NULL), tle_ttext_text(temp, txt));
  CHECK_TEXT(tgt_ttext_text(temp, NULL), tgt_ttext_text(temp, txt));
  CHECK_TEXT(tge_ttext_text(temp, NULL), tge_ttext_text(temp, txt));
  CHECK_TEXT(ttext_at_value(temp, NULL), ttext_at_value(temp, txt));
  CHECK_TEXT(ttext_minus_value(temp, NULL), ttext_minus_value(temp, txt));
  CHECK_TEXT(ttextseq_from_base_tstzspan(NULL, s),
    ttextseq_from_base_tstzspan(txt, s));

  free(txt);
  free(s);
  free(temp);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
