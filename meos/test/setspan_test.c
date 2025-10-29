/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief A simple program that tests the set and span functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o setspan_test setspan_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */

  bool b1 = bool_in("true");
  char *b1_out = bool_out(b1);

  int32 int32_in1 = 1;
  int32 int32_in2 = 3;

  int64 int64_in1 = 1;
  int64 int64_in2 = 3;

  size_t size, iset1_wkb_size, ispan1_wkb_size, ispanset1_wkb_size;

  double float8_in1 = 1;
  double float8_in2 = 3;

  char *text1_in = "abcdef";
  text *text1 = text_in(text1_in);
  char *text1_out = text_out(text1);
  char *text2_in = "ghijkl";
  text *text2 = text_in(text2_in);
  char *text2_out = text_out(text2);

  text *text_null = text_in("NULL");

  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstz2_in = "2001-01-03";
  TimestampTz tstz2 = timestamptz_in(tstz2_in, -1);
  char *tstz2_out = timestamptz_out(tstz2);

  char *date1_in = "2001-01-01";
  DateADT date1 = date_in(date1_in);
  char *date1_out = date_out(date1);
  char *date2_in = "2001-01-03";
  DateADT date2 = date_in(date2_in);
  char *date2_out = date_out(date2);

  char *interv1_in = "3 days";
  Interval *interv1 = interval_in(interv1_in, -1);
  char *interv1_out = interval_out(interv1);
  char *interv2_in = "3 hours";
  Interval *interv2 = interval_in(interv2_in, -1);
  char *interv2_out = interval_out(interv2);

  char *iset1_in = "{1,3,5}";
  Set *iset1 = intset_in(iset1_in);
  char *iset1_out = intset_out(iset1);
  char *iset1_hexwkb = set_as_hexwkb(iset1, 1, &size);
  uint8_t *iset1_wkb = set_as_wkb(iset1, 1, &iset1_wkb_size);
  char *ispan1_in = "[1,3]";
  Span *ispan1 = intspan_in(ispan1_in);
  char *ispan1_out = intspan_out(ispan1);
  char *ispan1_hexwkb = span_as_hexwkb(ispan1, 1, &size);
  uint8_t *ispan1_wkb = span_as_wkb(ispan1, 1, &ispan1_wkb_size);
  char *ispanset1_in = "{[1,3],[4,6]}";
  SpanSet *ispanset1 = intspanset_in(ispanset1_in);
  char *ispanset1_out = intspanset_out(ispanset1);
  char *ispanset1_hexwkb = spanset_as_hexwkb(ispanset1, 1, &size);
  uint8_t *ispanset1_wkb = spanset_as_wkb(ispanset1, 1, &ispanset1_wkb_size);

  char *iset2_in = "{2,4,6}";
  Set *iset2 = intset_in(iset2_in);
  char *iset2_out = intset_out(iset2);
  char *ispan2_in = "[2,4]";
  Span *ispan2 = intspan_in(ispan2_in);
  char *ispan2_out = intspan_out(ispan2);
  char *ispanset2_in = "{[2,4],[5,7]}";
  SpanSet *ispanset2 = intspanset_in(ispanset2_in);
  char *ispanset2_out = intspanset_out(ispanset2);

  char *bset1_in = "{1,3,5}";
  Set *bset1 = bigintset_in(bset1_in);
  char *bset1_out = bigintset_out(bset1);
  char *bspan1_in = "[1,3]";
  Span *bspan1 = bigintspan_in(bspan1_in);
  char *bspan1_out = bigintspan_out(bspan1);
  char *bspanset1_in = "{[1,3],[4,6]}";
  SpanSet *bspanset1 = bigintspanset_in(bspanset1_in);
  char *bspanset1_out = bigintspanset_out(bspanset1);

  char *bset2_in = "{2,4,6}";
  Set *bset2 = bigintset_in(bset2_in);
  char *bset2_out = bigintset_out(bset2);
  char *bspan2_in = "[2,3]";
  Span *bspan2 = bigintspan_in(bspan2_in);
  char *bspan2_out = bigintspan_out(bspan2);
  char *bspanset2_in = "{[2,4],[5,7]}";
  SpanSet *bspanset2 = bigintspanset_in(bspanset2_in);
  char *bspanset2_out = bigintspanset_out(bspanset2);

  char *fset1_in = "{1,3,5}";
  Set *fset1 = floatset_in(fset1_in);
  char *fset1_out = floatset_out(fset1, 6);
  char *fspan1_in = "[1,3]";
  Span *fspan1 = floatspan_in(fspan1_in);
  char *fspan1_out = floatspan_out(fspan1, 6);
  char *fspanset1_in = "{[1,3],[4,6]}";
  SpanSet *fspanset1 = floatspanset_in(fspanset1_in);
  char *fspanset1_out = floatspanset_out(fspanset1, 6);

  char *fset2_in = "{2,4,6}";
  Set *fset2 = floatset_in(fset2_in);
  char *fset2_out = floatset_out(fset2, 6);
  char *fspan2_in = "[2,3]";
  Span *fspan2 = floatspan_in(fspan2_in);
  char *fspan2_out = floatspan_out(fspan2, 6);
  char *fspanset2_in = "{[2,4],[5,7]}";
  SpanSet *fspanset2 = floatspanset_in(fspanset2_in);
  char *fspanset2_out = floatspanset_out(fspanset2, 6);

  char *textset1_in = "{a,b,c}";
  Set *textset1 = textset_in(textset1_in);
  char *textset1_out = textset_out(textset1);

  char *dset1_in = "{2001-01-01, 2001-01-03, 2001-01-05}";
  Set *dset1 = dateset_in(dset1_in);
  char *dset1_out = dateset_out(dset1);
  char *dspan1_in = "[2001-01-01, 2001-01-03]";
  Span *dspan1 = datespan_in(dspan1_in);
  char *dspan1_out = datespan_out(dspan1);
  char *dspanset1_in = "{[2001-01-01, 2001-01-03],[2001-01-04, 2001-01-06]}";
  SpanSet *dspanset1 = datespanset_in(dspanset1_in);
  char *dspanset1_out = datespanset_out(dspanset1);

  char *dset2_in = "{2001-01-02, 2001-01-04, 2001-01-06}";
  Set *dset2 = dateset_in(dset2_in);
  char *dset2_out = dateset_out(dset2);
  char *dspan2_in = "[2001-01-02, 2001-01-04]";
  Span *dspan2 = datespan_in(dspan2_in);
  char *dspan2_out = datespan_out(dspan2);
  char *dspanset2_in = "{[2001-01-02, 2001-01-04],[2001-01-05, 2001-01-07]}";
  SpanSet *dspanset2 = datespanset_in(dspanset2_in);
  char *dspanset2_out = datespanset_out(dspanset2);

  char *tstzset1_in = "{2001-01-01, 2001-01-03, 2001-01-05}";
  Set *tstzset1 = tstzset_in(tstzset1_in);
  char *tstzset1_out = tstzset_out(tstzset1);
  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);
  char *tstzspanset1_in = "{[2001-01-01, 2001-01-03],[2001-01-04, 2001-01-06]}";
  SpanSet *tstzspanset1 = tstzspanset_in(tstzspanset1_in);
  char *tstzspanset1_out = tstzspanset_out(tstzspanset1);

  char *tstzset2_in = "{2001-01-02, 2001-01-04, 2001-01-06}";
  Set *tstzset2 = tstzset_in(tstzset2_in);
  char *tstzset2_out = tstzset_out(tstzset2);
  char *tstzspan2_in = "[2001-01-02, 2001-01-04]";
  Span *tstzspan2 = tstzspan_in(tstzspan2_in);
  char *tstzspan2_out = tstzspan_out(tstzspan2);
  char *tstzspanset2_in = "{[2001-01-02, 2001-01-04],[2001-01-05, 2001-01-07]}";
  SpanSet *tstzspanset2 = tstzspanset_in(tstzspanset2_in);
  char *tstzspanset2_out = tstzspanset_out(tstzspanset2);

  /* Create the result types for the functions of the API */

  int64 int64_result;
  double float8_result;
  DateADT date_result;
  TimestampTz tstz_result;
  Interval *interv_result;

  Set *bset_result;
  Set *iset_result;
  Set *fset_result;
  Set *textset_result;
  Set *dset_result;
  Set *tstzset_result;
  Span *bspan_result;
  Span *ispan_result;
  Span *fspan_result;
  Span *dspan_result;
  Span *tstzspan_result;
  SpanSet *bspanset_result;
  SpanSet *ispanset_result;
  SpanSet *fspanset_result;
  SpanSet *dspanset_result;
  SpanSet *tstzspanset_result;
  bool bool_result;
  int32 int32_result;
  uint32 uint32_result;
  uint64 uint64_result;
  char *char_result;
  text *text_result;
  int count;

  uint8_t *binchar_result;

  int32 *int32array_result;
  int64 *int64array_result;
  double *float8array_result;
  text **textarray_result;
  DateADT *datearray_result;
  TimestampTz *tstzarray_result;
  Span **ispanarray_result;
  Span *dspanarray_result;
  Span *ispanvector_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Set and span types *\n");
  printf("****************************************************************\n");

  /*****************************************************************************
   * Input/output functions for set and span types
   *****************************************************************************/

  /* Set *bigintset_in(const char *str); */
  bset_result = bigintset_in(bset1_in);
  char_result = bigintset_out(bset_result);
  printf("bigintset_in(%s): %s\n", bset1_in, char_result);
  free(bset_result); free(char_result);

  /* char *bigintset_out(const Set *set); */
  char_result = bigintset_out(bset1);
  printf("bigintset_out(%s): %s\n", bset1_out, char_result);
  free(char_result);

  /* Span *bigintspan_in(const char *str); */
  bspan_result = bigintspan_in(bspan1_in);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspan_in(%s): %s\n", bspan1_in, char_result);
  free(bspan_result); free(char_result);

  /* char *bigintspan_out(const Span *s); */
  char_result = bigintspan_out(bspan1);
  printf("bigintspan_out(%s): %s\n", bspan1_out, char_result);
  free(char_result);

  /* SpanSet *bigintspanset_in(const char *str); */
  bspanset_result = bigintspanset_in(bspanset1_in);
  char_result = bigintspanset_out(bspanset_result);
  printf("bigintspanset_in(%s): %s\n", bspanset1_in, char_result);
  free(bspanset_result); free(char_result);

  /* char *bigintspanset_out(const SpanSet *ss); */
  char_result = bigintspanset_out(bspanset1);
  printf("bigintspanset_out(%s): %s\n", bspanset1_out, char_result);
  free(char_result);

  /* Set *dateset_in(const char *str); */
  dset_result = dateset_in(dset1_in);
  char_result = dateset_out(dset_result);
  printf("dateset_in(%s): %s\n", dset1_in, char_result);
  free(dset_result); free(char_result);

  /* char *dateset_out(const Set *s); */
  char_result = dateset_out(dset1);
  printf("dateset_out(%s): %s\n", dset1_out, char_result);
  free(char_result);

  /* Span *datespan_in(const char *str); */
  dspan_result = datespan_in(dspan1_in);
  char_result = datespan_out(dspan_result);
  printf("datespan_in(%s): %s\n", dspan1_in, char_result);
  free(dspan_result); free(char_result);

  /* char *datespan_out(const Span *s); */
  char_result = datespan_out(dspan1);
  printf("datespan_out(%s): %s\n", dspan1_out, char_result);
  free(char_result);

  /* SpanSet *datespanset_in(const char *str); */
  dspanset_result = datespanset_in(dspanset1_in);
  char_result = datespanset_out(dspanset_result);
  printf("datespanset_in(%s): %s\n", dspanset1_in, char_result);
  free(dspanset_result); free(char_result);

  /* char *datespanset_out(const SpanSet *ss); */
  char_result = datespanset_out(dspanset1);
  printf("datespanset_out(%s): %s\n", dspanset1_out, char_result);
  free(char_result);

  /* Set *floatset_in(const char *str); */
  fset_result = floatset_in(fset1_in);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_in(%s): %s\n", fset1_in, char_result);
  free(fset_result); free(char_result);

  /* char *floatset_out(const Set *set, int maxdd); */
  char_result = floatset_out(fset1, 6);
  printf("floatset_out(%s, 6): %s\n", fset1_out, char_result);
  free(char_result);

  /* Span *floatspan_in(const char *str); */
  fspan_result = floatspan_in(fspan1_in);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_in(%s): %s\n", fspan1_in, char_result);
  free(fspan_result); free(char_result);

  /* char *floatspan_out(const Span *s, int maxdd); */
  char_result = floatspan_out(fspan1, 6);
  printf("floatspan_out(%s, 6): %s\n", fspan1_out, char_result);
  free(char_result);

  /* SpanSet *floatspanset_in(const char *str); */
  fspanset_result = floatspanset_in(fspanset1_in);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_in(%s): %s\n", fspanset1_in, char_result);
  free(fspanset_result); free(char_result);

  /* char *floatspanset_out(ispanset1, int maxdd); */
  char_result = floatspanset_out(fspanset1, 6);
  printf("floatspanset_out(%s, 6): %s\n", fspanset1_out, char_result);
  free(char_result);

  /* Set *intset_in(const char *str); */
  iset_result = intset_in(iset1_in);
  char_result = intset_out(iset_result);
  printf("intset_in(%s): %s\n", iset1_in, char_result);
  free(iset_result); free(char_result);

  /* char *intset_out(const Set *set); */
  char_result = intset_out(iset1);
  printf("intset_out(%s): %s\n", iset1_out, char_result);
  free(char_result);

  /* Span *intspan_in(const char *str); */
  ispan_result = intspan_in(ispan1_in);
  char_result = intspan_out(ispan_result);
  printf("intspan_in(%s): %s\n", ispan1_in, char_result);
  free(ispan_result); free(char_result);

  /* char *intspan_out(const Span *s); */
  char_result = intspan_out(ispan1);
  printf("intspan_out(%s): %s\n", ispan1_out, char_result);
  free(char_result);

  /* SpanSet *intspanset_in(const char *str); */
  ispanset_result = intspanset_in(ispanset1_in);
  char_result = intspanset_out(ispanset_result);
  printf("intspanset_in(%s): %s\n", ispanset1_in, char_result);
  free(ispanset_result); free(char_result);

  /* char *intspanset_out(const SpanSet *ss); */
  char_result = intspanset_out(ispanset1);
  printf("intspanset_out(%s): %s\n", ispanset1_out, char_result);
  free(char_result);

  /* char *set_as_hexwkb(const Set *s, uint8_t variant, size_t *size_out); */
  char_result = set_as_hexwkb(iset1, 1, &size);
  printf("set_as_hexwkb(%s, 1, &size): %s\n", iset1_out, char_result);
  free(char_result);

  /* uint8_t *set_as_wkb(const Set *s, uint8_t variant, size_t *size_out); */
  binchar_result = set_as_wkb(iset1, 1, &size);
  printf("set_as_wkb(%s, 1, %ld): ", iset1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* Set *set_from_hexwkb(const char *hexwkb); */
  iset_result = set_from_hexwkb(iset1_hexwkb);
  char_result = intset_out(iset_result);
  printf("set_from_hexwkb(%s): %s\n", iset1_hexwkb, char_result);
  free(iset_result); free(char_result);

  /* Set *set_from_wkb(const uint8_t *wkb, size_t size); */
  iset_result = set_from_wkb(iset1_wkb, iset1_wkb_size);
  char_result = intset_out(iset_result);
  printf("set_from_wkb(");
  fwrite(iset1_wkb, iset1_wkb_size, 1, stdout);
  printf("): %s\n", char_result);
  free(iset_result); free(char_result);

  /* char *span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out); */
  char_result = span_as_hexwkb(ispan1, 1, &size);
  printf("span_as_hexwkb(%s 1, &size): %s\n", ispan1_out, char_result);
  free(char_result);

  /* uint8_t *span_as_wkb(const Span *s, uint8_t variant, size_t *size_out); */
  binchar_result = span_as_wkb(ispan1, 1, &size);
  printf("span_as_wkb(%s, 1, %ld): ", ispan1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* Span *span_from_hexwkb(const char *hexwkb); */
  ispan_result = span_from_hexwkb(ispan1_hexwkb);
  char_result = intspan_out(ispan_result);
  printf("span_from_hexwkb(%s): %s\n", ispan1_hexwkb, char_result);
  free(ispan_result); free(char_result);

  /* Span *span_from_wkb(const uint8_t *wkb, size_t size); */
  ispan_result = span_from_wkb(ispan1_wkb, ispan1_wkb_size);
  char_result = intspan_out(ispan_result);
  printf("span_from_wkb(");
  fwrite(ispan1_wkb, ispan1_wkb_size, 1, stdout);
  printf("): %s\n", char_result);
  free(ispan_result); free(char_result);

  /* char *spanset_as_hexwkb(const SpanSet *ss, uint8_t variant, size_t *size_out); */
  char_result = spanset_as_hexwkb(ispanset1, 1, &size);
  printf("spanset_as_hexwkb(%s, 1, &size): %s\n", ispanset1_out, char_result);
  free(char_result);

  /* uint8_t *spanset_as_wkb(const SpanSet *ss, uint8_t variant, size_t *size_out); */
  binchar_result = spanset_as_wkb(ispanset1, 1, &size);
  printf("spanset_as_wkb(%s, 1, %ld): ", ispanset1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* SpanSet *spanset_from_hexwkb(const char *hexwkb); */
  ispanset_result = spanset_from_hexwkb(ispanset1_hexwkb);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_from_hexwkb(%s): %s\n", ispanset1_hexwkb, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *spanset_from_wkb(const uint8_t *wkb, size_t size); */
  ispanset_result = spanset_from_wkb(ispanset1_wkb, ispanset1_wkb_size);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_from_wkb(");
  fwrite(ispanset1_wkb, ispanset1_wkb_size, 1, stdout);
  printf("): %s\n", char_result);
  free(ispanset_result); free(char_result);

  /* Set *textset_in(const char *str); */
  textset_result = textset_in(textset1_in);
  char_result = textset_out(textset_result);
  printf("textset_in(%s): %s\n", textset1_in, char_result);
  free(textset_result); free(char_result);

  /* char *textset_out(const Set *set); */
  char_result = textset_out(textset1);
  printf("textset_out(%s): %s\n", textset1_out, char_result);
  free(char_result);

  /* Set *tstzset_in(const char *str); */
  tstzset_result = tstzset_in(tstzset1_in);
  char_result = tstzset_out(tstzset_result);
  printf("tstzset_in(%s): %s\n", tstzset1_in, char_result);
  free(tstzset_result); free(char_result);

  /* char *tstzset_out(const Set *set); */
  char_result = tstzset_out(tstzset1);
  printf("tstzset_out(%s): %s\n", tstzset1_out, char_result);
  free(char_result);

  /* Span *tstzspan_in(const char *str); */
  tstzspan_result = tstzspan_in(tstzspan1_in);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_in(%s): %s\n", tstzspan1_in, char_result);
  free(tstzspan_result); free(char_result);

  /* char *tstzspan_out(const Span *s); */
  char_result = tstzspan_out(tstzspan1);
  printf("tstzspan_out(%s): %s\n", tstzspan1_out, char_result);
  free(char_result);

  /* SpanSet *tstzspanset_in(const char *str); */
  tstzspanset_result = tstzspanset_in(tstzspanset1_in);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("tstzspanset_in(%s): %s\n", tstzspanset1_in, char_result);
  free(tstzspanset_result); free(char_result);

  /* char *tstzspanset_out(const SpanSet *ss); */
  char_result = tstzspanset_out(tstzspanset1);
  printf("tstzspanset_out(%s): %s\n", tstzspanset1_out, char_result);
  free(char_result);

  /*****************************************************************************
   * Constructor functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Set *bigintset_make(const int64 *values, int count); */
  int64 bigintarray[2];
  bigintarray[0] = int64_in1;
  bigintarray[1] = int64_in2;
  bset_result = bigintset_make(bigintarray, 2);
  char_result = bigintset_out(bset_result);
  printf("bigintset_make({%lu, %lu} 2): %s\n", int64_in1, int64_in2, char_result);
  free(bset_result); free(char_result);

  /* Span *bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc); */
  bspan_result = bigintspan_make(int64_in1, int64_in2, true, false);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspan_make(%ld, %ld, true, false): %s\n", int64_in1, int64_in2, char_result);
  free(bspan_result); free(char_result);

  /* Set *dateset_make(const DateADT *values, int count); */
  DateADT datearray[2];
  datearray[0] = date1;
  datearray[1] = date2;
  dset_result = dateset_make(datearray, 2);
  char_result = dateset_out(dset_result);
  printf("dateset_make({%s, %s}, 2): %s\n", date1_out, date2_out, char_result);
  free(dset_result); free(char_result);

  /* Span *datespan_make(DateADT lower, DateADT upper, bool lower_inc, bool upper_inc); */
  dspan_result = datespan_make(date1, date2, true, false);
  char_result = datespan_out(dspan_result);
  printf("datespan_make(%s, %s): %s\n", date1_out, date2_out, char_result);
  free(dspan_result); free(char_result);

  /* Set *floatset_make(const double *values, int count); */
  double floatarray[2];
  floatarray[0] = float8_in1;
  floatarray[1] = float8_in2;
  fset_result = floatset_make(floatarray, 2);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_make({%lf, %lf}): %s\n", float8_in1, float8_in2, char_result);
  free(fset_result); free(char_result);

  /* Span *floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc); */
  fspan_result = floatspan_make(float8_in1, float8_in2, true, false);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_make(%lf, %lf, true, false): %s\n", float8_in1, float8_in2, char_result);
  free(fspan_result); free(char_result);

  /* Set *intset_make(const int *values, int count); */
  int intarray[2];
  intarray[0] = int32_in1;
  intarray[1] = int32_in2;
  iset_result = intset_make(intarray, 2);
  char_result = intset_out(iset_result);
  printf("intset_make({%d, %d}): %s\n", int32_in1, int32_in2, char_result);
  free(iset_result); free(char_result);

  /* Span *intspan_make(int lower, int upper, bool lower_inc, bool upper_inc); */
  ispan_result = intspan_make(int32_in1, int32_in2, true, false);
  char_result = intspan_out(ispan_result);
  printf("intspan_make(%d, %d, true, false): %s\n", int32_in1, int32_in1, char_result);
  free(ispan_result); free(char_result);

  /* Set *set_copy(const Set *s); */
  iset_result = set_copy(iset1);
  char_result = intset_out(iset_result);
  printf("set_copy(%s): %s\n", iset1_out, char_result);
  free(iset_result); free(char_result);

  /* Span *span_copy(const Span *s); */
  ispan_result = span_copy(ispan1);
  char_result = intspan_out(ispan_result);
  printf("span_copy(%s): %s\n", ispan1_out, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *spanset_copy(const SpanSet *ss); */
  ispanset_result = spanset_copy(ispanset1);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_copy(%s): %s\n", ispanset1_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *spanset_make(Span *spans, int count); */
  Span ispanarray[2];
  ispanarray[0] = *ispan1;
  ispanarray[1] = *ispan2;
  ispanset_result = spanset_make(ispanarray, 2);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_make({%s, %s}): %s\n", ispan1_out, ispan2_out, char_result);
  free(ispanset_result); free(char_result);

  /* Set *textset_make(text **values, int count); */
  text *textarray[2];
  textarray[0] = text1;
  textarray[1] = text2;
  textset_result = textset_make(textarray, 2);
  char_result = textset_out(textset_result);
  printf("textset_make({%s, %s}): %s\n", text1_out, text2_out, char_result);
  free(textset_result); free(char_result);

  /* Set *tstzset_make(const TimestampTz *values, int count); */
  TimestampTz tstzarray[2];
  tstzarray[0] = tstz1;
  tstzarray[1] = tstz2;
  tstzset_result = tstzset_make(tstzarray, 2);
  char_result = tstzset_out(tstzset_result);
  printf("tstzset_make({%s, %s}): %s\n", tstz1_out, tstz2_out, char_result);
  free(tstzset_result); free(char_result);

  /* Span *tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc); */
  tstzspan_result = tstzspan_make(tstz1, tstz2, true, false);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_make(%s, %s, true, false): %s\n", tstz1_out, tstz2_out, char_result);
  free(tstzspan_result); free(char_result);

  /*****************************************************************************
   * Conversion functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Set *bigint_to_set(int64 i); */
  bset_result = bigint_to_set(int64_in1);
  char_result = bigintset_out(bset_result);
  printf("bigint_to_set(%lu): %s\n", int64_in1, char_result);
  free(bset_result); free(char_result);

  /* Span *bigint_to_span(int i); */
  bspan_result = bigint_to_span(int64_in1);
  char_result = bigintspan_out(bspan_result);
  printf("bigint_to_span(%lu): %s\n", int64_in1, char_result);
  free(bspan_result); free(char_result);

  /* SpanSet *bigint_to_spanset(int i); */
  bspanset_result = bigint_to_spanset(int64_in1);
  char_result = bigintspanset_out(bspanset_result);
  printf("bigint_to_spanset(%ld): %s\n", int64_in1, char_result);
  free(bspanset_result); free(char_result);

  /* Set *date_to_set(DateADT d); */
  dset_result = date_to_set(date1);
  char_result = dateset_out(dset_result);
  printf("date_to_set(%s): %s\n", date1_out, char_result);
  free(dset_result); free(char_result);

  /* Span *date_to_span(DateADT d); */
  dspan_result = date_to_span(date1);
  char_result = datespan_out(dspan_result);
  printf("date_to_span(%s): %s\n", date1_out, char_result);
  free(dspan_result); free(char_result);

  /* SpanSet *date_to_spanset(DateADT d); */
  dspanset_result = date_to_spanset(date1);
  char_result = datespanset_out(dspanset_result);
  printf("date_to_spanset(%s): %s\n", date1_out, char_result);
  free(dspanset_result); free(char_result);

  /* Set *dateset_to_tstzset(const Set *s); */
  tstzset_result = dateset_to_tstzset(dset1);
  char_result = tstzset_out(tstzset_result);
  printf("dateset_to_tstzset(%s): %s\n", dset1_out, char_result);
  free(tstzset_result); free(char_result);

  /* Span *datespan_to_tstzspan(const Span *s); */
  tstzspan_result = datespan_to_tstzspan(dspan1);
  char_result = tstzspan_out(tstzspan_result);
  printf("datespan_to_tstzspan(%s): %s\n", dspan1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* SpanSet *datespanset_to_tstzspanset(const SpanSet *ss); */
  tstzspanset_result = datespanset_to_tstzspanset(dspanset1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("datespanset_to_tstzspanset(%s): %s\n", dspanset1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* Set *float_to_set(double d); */
  fset_result = float_to_set(float8_in1);
  char_result = floatset_out(fset_result, 6);
  printf("float_to_set(%lf, 6): %s\n", float8_in1, char_result);
  free(fset_result); free(char_result);

  /* Span *float_to_span(double d); */
  fspan_result = float_to_span(float8_in1);
  char_result = floatspan_out(fspan_result, 6);
  printf("float_to_span(%lf, 6): %s\n", float8_in1, char_result);
  free(fspan_result); free(char_result);

  /* SpanSet *float_to_spanset(double d); */
  fspanset_result = float_to_spanset(float8_in1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("float_to_spanset(%lf, 6): %s\n", float8_in1, char_result);
  free(fspanset_result); free(char_result);

  /* Set *floatset_to_intset(const Set *s); */
  iset_result = floatset_to_intset(fset1);
  char_result = intset_out(iset_result);
  printf("floatset_to_intset(%s): %s\n", fset1_out, char_result);
  free(iset_result); free(char_result);

  /* Span *floatspan_to_intspan(const Span *s); */
  ispan_result = floatspan_to_intspan(fspan1);
  char_result = intspan_out(ispan_result);
  printf("floatspan_to_intspan(%s): %s\n", ispan1_out, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *floatspanset_to_intspanset(const SpanSet *ss); */
  ispanset_result = floatspanset_to_intspanset(fspanset1);
  char_result = intspanset_out(ispanset_result);
  printf("floatspanset_to_intspanset(%s): %s\n", fspanset1_out, char_result);
  free(ispanset_result); free(char_result);

  /* Set *int_to_set(int i); */
  iset_result = int_to_set(int32_in1);
  char_result = intset_out(iset_result);
  printf("int_to_set(%d): %s\n", int32_in1, char_result);
  free(iset_result); free(char_result);

  /* Span *int_to_span(int i); */
  ispan_result = int_to_span(int32_in1);
  char_result = intspan_out(ispan_result);
  printf("int_to_span(%d): %s\n", int32_in1, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *int_to_spanset(int i); */
  ispanset_result = int_to_spanset(int32_in1);
  char_result = intspanset_out(ispanset_result);
  printf("int_to_spanset(%d): %s\n", int32_in1, char_result);
  free(ispanset_result); free(char_result);

  /* Set *intset_to_floatset(const Set *s); */
  fset_result = intset_to_floatset(iset1);
  char_result = floatset_out(fset_result, 6);
  printf("intset_to_floatset(%s, 6): %s\n", iset1_out, char_result);
  free(fset_result); free(char_result);

  /* Span *intspan_to_floatspan(const Span *s); */
  fspan_result = intspan_to_floatspan(ispan1);
  char_result = floatspan_out(fspan_result, 6);
  printf("intspan_to_floatspan(%s, 6): %s\n", ispan1_out, char_result);
  free(fspan_result); free(char_result);

  /* SpanSet *intspanset_to_floatspanset(const SpanSet *ss); */
  fspanset_result = intspanset_to_floatspanset(ispanset1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("intspanset_to_floatspanset(%s): %s\n", ispanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* Span *set_to_span(const Set *s); */
  ispan_result = set_to_span(iset1);
  char_result = intspan_out(ispan_result);
  printf("set_to_span(%s): %s\n", iset1_out, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *set_to_spanset(const Set *s); */
  ispanset_result = set_to_spanset(iset1);
  char_result = intspanset_out(ispanset_result);
  printf("set_to_spanset(%s): %s\n", iset1_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *span_to_spanset(const Span *s); */
  ispanset_result = span_to_spanset(ispan1);
  char_result = intspanset_out(ispanset_result);
  printf("span_to_spanset(%s): %s\n", ispan1_out, char_result);
  free(ispanset_result); free(char_result);

  /* Set *text_to_set(const text *txt); */
  textset_result = text_to_set(text1);
  char_result = textset_out(textset_result);
  printf("text_to_set(%s): %s\n", text1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *timestamptz_to_set(TimestampTz t); */
  tstzset_result = timestamptz_to_set(tstz1);
  char_result = tstzset_out(tstzset_result);
  printf("timestamptz_to_set(%s): %s\n", tstz1_out, char_result);
  free(tstzset_result); free(char_result);

  /* Span *timestamptz_to_span(TimestampTz t); */
  tstzspan_result = timestamptz_to_span(tstz1);
  char_result = tstzspan_out(tstzspan_result);
  printf("timestamptz_to_span(%s): %s\n", tstz1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* SpanSet *timestamptz_to_spanset(TimestampTz t); */
  tstzspanset_result = timestamptz_to_spanset(tstz1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("timestamptz_to_spanset(%s): %s\n", tstz1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* Set *tstzset_to_dateset(const Set *s); */
  dset_result = tstzset_to_dateset(tstzset1);
  char_result = dateset_out(dset_result);
  printf("tstzset_to_dateset(%s): %s\n", tstzset1_out, char_result);
  free(dset_result); free(char_result);

  /* Span *tstzspan_to_datespan(const Span *s); */
  dspan_result = tstzspan_to_datespan(tstzspan1);
  char_result = datespan_out(dspan_result);
  printf("tstzspan_to_datespan(%s): %s\n", tstzspan1_out, char_result);
  free(dspan_result); free(char_result);

  /* SpanSet *tstzspanset_to_datespanset(const SpanSet *ss); */
  dspanset_result = tstzspanset_to_datespanset(tstzspanset1);
  char_result = datespanset_out(dspanset_result);
  printf("tstzspanset_to_datespanset(%s): %s\n", tstzspanset1_out, char_result);
  free(dspanset_result); free(char_result);

  /*****************************************************************************
   * Accessor functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* int64 bigintset_end_value(const Set *s); */
  int64_result = bigintset_end_value(bset1);
  printf("bigintset_end_value(%s): %ld\n", bset1_out, int64_result);

  /* int64 bigintset_start_value(const Set *s); */
  int64_result = bigintset_start_value(bset1);
  printf("bigintset_start_value(%s): %ld\n", bset1_out, int64_result);

  /* bool bigintset_value_n(const Set *s, int n, int64 *result); */
  bool_result = bigintset_value_n(bset1, 1, &int64_result);
  printf("bigintset_value_n(%s, 1, %ld): %c\n", bset1_out, int64_result, bool_result ? 't' : 'n');

  /* int64 *bigintset_values(const Set *s); */
  int64array_result = bigintset_values(bset1);
  printf("bigintset_values(%s): {", bset1_out);
  for (int i = 0; i < bset1->count; i++)
  {
    printf("%ld", int64array_result[i]);
    if (i < bset1->count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(int64array_result);

  /* int64 bigintspan_lower(const Span *s); */
  int64_result = bigintspan_lower(bspan1);
  printf("bigintspan_lower(%s): %ld\n", bspan1_out, int64_result);

  /* int64 bigintspan_upper(const Span *s); */
  int64_result = bigintspan_upper(bspan1);
  printf("bigintspan_upper(%s): %ld\n", bspan1_out, int64_result);

  /* int64 bigintspan_width(const Span *s); */
  int64_result = bigintspan_width(bspan1);
  printf("bigintspan_width(%s): %ld\n", bspan1_out, int64_result);

  /* int64 bigintspanset_lower(const SpanSet *ss); */
  int64_result = bigintspanset_lower(bspanset1);
  printf("bigintspanset_lower(%s): %ld\n", bspanset1_out, int64_result);

  /* int64 bigintspanset_upper(const SpanSet *ss); */
  int64_result = bigintspanset_upper(bspanset1);
  printf("bigintspanset_upper(%s): %ld\n", bspanset1_out, int64_result);

  /* int64 bigintspanset_width(spanset1, bool boundspan); */
  int64_result = bigintspanset_width(bspanset1, true);
  printf("bigintspanset_width(%s): %ld\n", bspanset1_out, int64_result);

  /* DateADT dateset_end_value(const Set *s); */
  date_result = dateset_end_value(dset1);
  char_result = date_out(date_result);
  printf("dateset_end_value(%s): %s\n", dset1_out, char_result);
  free(char_result);

  /* DateADT dateset_start_value(const Set *s); */
  date_result = dateset_start_value(dset1);
  char_result = date_out(date_result);
  printf("dateset_start_value(%s): %s\n", dset1_out, char_result);
  free(char_result);

  /* bool dateset_value_n(const Set *s, int n, DateADT *result); */
  bool_result = dateset_value_n(dset1, 1, &date_result);
  char_result = date_out(date_result);
  printf("dateset_value_n(%s, 1, %s): %c\n", dset1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* DateADT *dateset_values(const Set *s); */
  datearray_result = dateset_values(dset1);
  printf("bigintset_values(%s): {", dset1_out);
  for (int i = 0; i < dset1->count; i++)
  {
    char_result = date_out(datearray_result[i]);
    printf("%s", char_result);
    if (i < dset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(datearray_result);

  /* Interval *datespan_duration(const Span *s); */
  interv_result = datespan_duration(dspan1);
  char_result = interval_out(interv_result);
  printf("datespan_duration(%s): %s\n", dspan1_out, char_result);
  free(interv_result); free(char_result);

  /* DateADT datespan_lower(const Span *s); */
  date_result = datespan_lower(dspan1);
  char_result = date_out(date_result);
  printf("datespan_lower(%s): %s\n", dspan1_out, char_result);
  free(char_result);

  /* DateADT datespan_upper(const Span *s); */
  date_result = datespan_upper(dspan1);
  char_result = date_out(date_result);
  printf("datespan_upper(%s): %s\n", dspan1_out, char_result);
  free(char_result);

  /* bool datespanset_date_n(spanset1, int n, DateADT *result); */
  bool_result = datespanset_date_n(dspanset1, 1, &date_result);
  char_result = date_out(date_result);
  printf("datespanset_date_n(%s, 1, %s): %c\n", dspanset1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* Set *datespanset_dates(const SpanSet *ss); */
  dset_result = datespanset_dates(dspanset1);
  char_result = dateset_out(dset_result);
  printf("datespanset_dates(%s): %s\n", dset1_in, char_result);
  free(dset_result); free(char_result);

  /* Interval *datespanset_duration(spanset1, bool boundspan); */
  interv_result = datespanset_duration(dspanset1, true);
  char_result = interval_out(interv_result);
  printf("datespanset_duration(%s): %s\n", dspanset1_out, char_result);
  free(interv_result); free(char_result);

  /* DateADT datespanset_end_date(const SpanSet *ss); */
  date_result = datespanset_end_date(dspanset1);
  char_result = date_out(date_result);
  printf("datespanset_end_date(%s): %s\n", dspanset1_out, char_result);
  free(char_result);

  /* int datespanset_num_dates(const SpanSet *ss); */
  int32_result = datespanset_num_dates(dspanset1);
  printf("datespanset_num_dates(%s): %d\n", dspanset1_out, int32_result);

  /* DateADT datespanset_start_date(const SpanSet *ss); */
  date_result = datespanset_start_date(dspanset1);
  char_result = date_out(date_result);
  printf("datespanset_start_date(%s): %s\n", dspanset1_out, char_result);
  free(char_result);

  /* double floatset_end_value(const Set *s); */
  float8_result = floatset_end_value(fset1);
  printf("floatset_end_value(%s): %lf\n", fset1_out, float8_result);

  /* double floatset_start_value(const Set *s); */
  float8_result = floatset_start_value(fset1);
  printf("floatset_start_value(%s): %lf\n", fset1_out, float8_result);

  /* bool floatset_value_n(const Set *s, int n, double *result); */
  bool_result = floatset_value_n(fset1, 1, &float8_result);
  printf("floatset_value_n(%s, 1, %lf): %c\n", fset1_out, float8_result, bool_result ? 't' : 'n');

  /* double *floatset_values(const Set *s); */
  float8array_result = floatset_values(fset1);
  printf("floatset_values(%s): {", fset1_out);
  for (int i = 0; i < fset1->count; i++)
  {
    printf("%lf", float8array_result[i]);
    if (i < fset1->count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(float8array_result);

  /* double floatspan_lower(const Span *s); */
  float8_result = floatspan_lower(fspan1);
  printf("floatspan_lower(%s): %lf\n", fspan1_out, float8_result);

  /* double floatspan_upper(const Span *s); */
  float8_result = floatspan_upper(fspan1);
  printf("floatspan_upper(%s): %lf\n", fspan1_out, float8_result);

  /* double floatspan_width(const Span *s); */
  float8_result = floatspan_width(fspan1);
  printf("floatspan_width(%s): %lf\n", fspan1_out, float8_result);

  /* double floatspanset_lower(const SpanSet *ss); */
  float8_result = floatspanset_lower(fspanset1);
  printf("floatspanset_lower(%s): %lf\n", fspanset1_out, float8_result);

  /* double floatspanset_upper(const SpanSet *ss); */
  float8_result = floatspanset_upper(fspanset1);
  printf("floatspanset_upper(%s): %lf\n", fspanset1_out, float8_result);

  /* double floatspanset_width(spanset1, bool boundspan); */
  float8_result = floatspanset_width(fspanset1, true);
  printf("floatspanset_width(%s): %lf\n", fspanset1_out, float8_result);

  /* int intset_end_value(const Set *s); */
  int32_result = intset_end_value(iset1);
  printf("intset_end_value(%s): %d\n", iset1_out, int32_result);

  /* int intset_start_value(const Set *s); */
  int32_result = intset_start_value(iset1);
  printf("intset_start_value(%s): %d\n", iset1_out, int32_result);

  /* bool intset_value_n(const Set *s, int n, int *result); */
  bool_result = intset_value_n(iset1, 1, &int32_result);
  printf("intset_value_n(%s, 1, %d): %c\n", iset1_out, int32_result, bool_result ? 't' : 'n');

  /* int *intset_values(const Set *s); */
  int32array_result = intset_values(iset1);
  printf("intset_values(%s): {", iset1_out);
  for (int i = 0; i < iset1->count; i++)
  {
    printf("%d", int32array_result[i]);
    if (i < iset1->count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(int32array_result);

  /* int intspan_lower(const Span *s); */
  int32_result = intspan_lower(ispan1);
  printf("intspan_lower(%s): %d\n", ispan1_out, int32_result);

  /* int intspan_upper(const Span *s); */
  int32_result = intspan_upper(ispan1);
  printf("intspan_upper(%s): %d\n", ispan1_out, int32_result);

  /* int intspan_width(const Span *s); */
  int32_result = intspan_width(ispan1);
  printf("intspan_width(%s): %d\n", ispan1_out, int32_result);

  /* int intspanset_lower(const SpanSet *ss); */
  int32_result = intspanset_lower(ispanset1);
  printf("intspanset_lower(%s): %d\n", ispanset1_out, int32_result);

  /* int intspanset_upper(const SpanSet *ss); */
  int32_result = intspanset_upper(ispanset1);
  printf("intspanset_upper(%s): %d\n", ispanset1_out, int32_result);

  /* int intspanset_width(spanset1, bool boundspan); */
  int32_result = intspanset_width(ispanset1, true);
  printf("intspanset_width(%s): %d\n", ispanset1_out, int32_result);

  /* uint32 set_hash(const Set *s); */
  uint32_result = set_hash(iset1);
  printf("set_hash(%s): %u\n", iset1_out, uint32_result);

  /* uint64 set_hash_extended(const Set *s, uint64 seed); */
  uint64_result = set_hash_extended(iset1, int32_in1);
  printf("set_hash_extended(%s): %lu\n", iset1_out, uint64_result);

  /* int set_num_values(const Set *s); */
  int32_result = set_num_values(iset1);
  printf("set_num_values(%s): %d\n", iset1_out, int32_result);

  /* uint32 span_hash(const Span *s); */
  uint32_result = span_hash(ispan1);
  printf("span_hash(%s): %u\n", ispan1_out, uint32_result);

  /* uint64 span_hash_extended(const Span *s, uint64 seed); */
  uint64_result = span_hash_extended(ispan1, int32_in1);
  printf("span_hash_extended(%s): %lu\n", ispan1_out, uint64_result);

  /* bool span_lower_inc(const Span *s); */
  bool_result = span_lower_inc(ispan1);
  printf("span_lower_inc(%s): %c\n", ispan1_out, bool_result ? 't' : 'n');

  /* bool span_upper_inc(const Span *s); */
  bool_result = span_upper_inc(ispan1);
  printf("span_upper_inc(%s): %c\n", ispan1_out, bool_result ? 't' : 'n');

  /* Span *spanset_end_span(const SpanSet *ss); */
  ispan_result = spanset_end_span(ispanset1);
  char_result = intspan_out(ispan_result);
  printf("spanset_end_span(%s): %s\n", ispanset1_out, char_result);
  free(ispan_result); free(char_result);

  /* uint32 spanset_hash(const SpanSet *ss); */
  uint32_result = spanset_hash(ispanset1);
  printf("spanset_hash(%s): %u\n", ispanset1_out, uint32_result);

  /* uint64 spanset_hash_extended(spanset1, uint64 seed); */
  uint64_result = spanset_hash_extended(ispanset1, 1);
  printf("spanset_hash_extended(%s): %lu\n", ispanset1_out, uint64_result);

  /* bool spanset_lower_inc(const SpanSet *ss); */
  bool_result = spanset_lower_inc(ispanset1);
  printf("spanset_lower_inc(%s): %c\n", ispanset1_out, bool_result ? 't' : 'n');

  /* int spanset_num_spans(const SpanSet *ss); */
  int32_result = spanset_num_spans(ispanset1);
  printf("spanset_num_spans(%s): %d\n", ispanset1_out, int32_result);

  /* Span *spanset_span(const SpanSet *ss); */
  ispan_result = spanset_span(ispanset1);
  char_result = intspan_out(ispan_result);
  printf("spanset_span(%s): %s\n", ispanset1_out, char_result);
  free(ispan_result); free(char_result);

  /* Span *spanset_span_n(spanset1, int i); */
  ispan_result = spanset_span_n(ispanset1, 1);
  char_result = intspan_out(ispan_result);
  printf("spanset_span_n(%s): %s\n", ispanset1_out, char_result);
  free(ispan_result); free(char_result);

  /* Span **spanset_spanarr(const SpanSet *ss); */
  ispanarray_result = spanset_spanarr(ispanset1);
  printf("spanset_spanarr(%s): {", ispanset1_out);
  for (int i = 0; i < ispanset1->count; i++)
  {
    char_result = intspan_out(ispanarray_result[i]);
    printf("%s", char_result);
    if (i < ispanset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(ispanarray_result[i]);
    free(char_result);
  }
  free(ispanarray_result);

  /* Span *spanset_start_span(const SpanSet *ss); */
  ispan_result = spanset_start_span(ispanset1);
  char_result = intspan_out(ispan_result);
  printf("spanset_start_span(%s): %s\n", ispanset1_out, char_result);
  free(ispan_result); free(char_result);

  /* bool spanset_upper_inc(const SpanSet *ss); */
  bool_result = spanset_upper_inc(ispanset1);
  printf("spanset_upper_inc(%s): %c\n", ispanset1_out, bool_result ? 't' : 'n');

  /* text *textset_end_value(const Set *s); */
  text_result = textset_end_value(textset1);
  char_result = text_out(text_result);
  printf("textset_end_value(%s): %s\n", textset1_out, char_result);
  free(text_result); free(char_result);

  /* text *textset_start_value(const Set *s); */
  text_result = textset_start_value(textset1);
  char_result = text_out(text_result);
  printf("textset_start_value(%s): %s\n", textset1_out, char_result);
  free(text_result); free(char_result);

  /* bool textset_value_n(const Set *s, int n, text **result); */
  bool_result = textset_value_n(textset1, 1, &text_result);
  char_result = text_out(text_result);
  printf("textset_value_n(%s, 1, %s): %c\n", textset1_out, char_result, bool_result ? 't' : 'n');
  free(text_result); free(char_result);

  /* text **textset_values(const Set *s); */
  textarray_result = textset_values(textset1);
  printf("textset_values(%s): {", textset1_out);
  for (int i = 0; i < textset1->count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s", char_result);
    if (i < textset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(textarray_result[i]);
    free(char_result);
  }
  free(textarray_result);

  /* TimestampTz tstzset_end_value(const Set *s); */
  tstz_result = tstzset_end_value(tstzset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzset_end_value(%s): %s\n", tstzset1_out, char_result);
  free(char_result);

  /* TimestampTz tstzset_start_value(const Set *s); */
  tstz_result = tstzset_start_value(tstzset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzset_start_value(%s): %s\n", tstzset1_out, char_result);
  free(char_result);

  /* bool tstzset_value_n(const Set *s, int n, TimestampTz *result); */
  bool_result = tstzset_value_n(tstzset1, 1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("tstzset_value_n(%s, 1, %s): %c\n", tstzset1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* TimestampTz *tstzset_values(const Set *s); */
  tstzarray_result = tstzset_values(tstzset1);
  printf("tstzset_values(%s): {", tstzset1_out);
  for (int i = 0; i < tstzset1->count; i++)
  {
    char_result = timestamptz_out(tstzarray_result[i]);
    printf("%s", char_result);
    if (i < tstzset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzarray_result);

  /* Interval *tstzspan_duration(const Span *s); */
  interv_result = tstzspan_duration(tstzspan1);
  char_result = interval_out(interv_result);
  printf("tstzspan_duration(%s): %s\n", tstzspan1_out, char_result);
  free(interv_result); free(char_result);

  /* TimestampTz tstzspan_lower(const Span *s); */
  tstz_result = tstzspan_lower(tstzspan1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspan_lower(%s): %s\n", tstzspan1_out, char_result);
  free(char_result);

  /* TimestampTz tstzspan_upper(const Span *s); */
  tstz_result = tstzspan_upper(tstzspan1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspan_upper(%s): %s\n", tstzspan1_out, char_result);
  free(char_result);

  /* Interval *tstzspanset_duration(spanset1, bool boundspan); */
  interv_result = tstzspanset_duration(tstzspanset1, true);
  char_result = interval_out(interv_result);
  printf("tstzspanset_duration(%s): %s\n", tstzspanset1_out, char_result);
  free(interv_result); free(char_result);

  /* TimestampTz tstzspanset_end_timestamptz(const SpanSet *ss); */
  tstz_result = tstzspanset_end_timestamptz(tstzspanset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspanset_end_timestamptz(%s): %s\n", tstzspanset1_out, char_result);
  free(char_result);

  /* TimestampTz tstzspanset_lower(const SpanSet *ss); */
  tstz_result = tstzspanset_lower(tstzspanset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspanset_lower(%s): %s\n", tstzspanset1_out, char_result);
  free(char_result);

  /* int tstzspanset_num_timestamps(const SpanSet *ss); */
  int32_result = tstzspanset_num_timestamps(tstzspanset1);
  printf("tstzspanset_num_timestamps(%s): %d\n", tstzspanset1_out, int32_result);

  /* TimestampTz tstzspanset_start_timestamptz(const SpanSet *ss); */
  tstz_result = tstzspanset_start_timestamptz(tstzspanset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspanset_start_timestamptz(%s): %s\n", tstzspanset1_out, char_result);
  free(char_result);

  /* Set *tstzspanset_timestamps(const SpanSet *ss); */
  tstzset_result = tstzspanset_timestamps(tstzspanset1);
  char_result = tstzset_out(tstzset_result);
  printf("tstzspanset_timestamps(%s): %s\n", tstz1_out, char_result);
  free(tstzset_result); free(char_result);

  /* bool tstzspanset_timestamptz_n(const SpanSet *ss, int n, TimestampTz *result); */
  bool_result = tstzspanset_timestamptz_n(tstzspanset1, 1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspanset_timestamptz_n(%s, 1, %s): %c\n", tstzspanset1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* TimestampTz tstzspanset_upper(const SpanSet *ss); */
  tstz_result = tstzspanset_upper(tstzspanset1);
  char_result = timestamptz_out(tstz_result);
  printf("tstzspanset_upper(%s): %s\n", tstzspanset1_out, char_result);
  free(char_result);

  /*****************************************************************************
   * Transformation functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Set *bigintset_shift_scale(const Set *s, int64 shift, int64 width, bool hasshift, bool haswidth); */
  bset_result = bigintset_shift_scale(bset1, int64_in1, int64_in2, true, true);
  char_result = bigintset_out(bset_result);
  printf("bigintset_shift_scale(%s, %ld, %ld, true, true): %s\n", bset1_out, int64_in1, int64_in2, char_result);
  free(bset_result); free(char_result);

  /* Span *bigintspan_expand(const Span *s, int64 value); */
  bspan_result = bigintspan_expand(bspan1, int64_in1);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspan_expand(%s): %s\n", bspan1_out, char_result);
  free(bspan_result); free(char_result);

  /* Span *bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift, bool haswidth); */
  bspan_result = bigintspan_shift_scale(bspan1, int64_in1, int64_in2, true, true);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspan_shift_scale(%s, %ld, %ld, true, true): %s\n", bspan1_out, int64_in1, int64_in2, char_result);
  free(bspan_result); free(char_result);

  /* SpanSet *bigintspanset_shift_scale(spanset1, int64 shift, int64 width, bool hasshift, bool haswidth); */
  bspanset_result = bigintspanset_shift_scale(bspanset1, int64_in1, int64_in2, true, true);
  char_result = bigintspanset_out(bspanset_result);
  printf("bigintspanset_shift_scale(%s, %ld, %ld, true, true): %s\n", bspanset1_out, int64_in1, int64_in2, char_result);
  free(bspanset_result); free(char_result);

  /* Set *dateset_shift_scale(const Set *s, int shift, int width, bool hasshift, bool haswidth); */
  dset_result = dateset_shift_scale(dset1, date1, date2, true, true);
  char_result = dateset_out(dset_result);
  printf("dateset_shift_scale(%s, %s, %s, true, true): %s\n", dset1_out, date1_out, date2_out, char_result);
  free(dset_result); free(char_result);

  /* Span *datespan_shift_scale(const Span *s, int shift, int width, bool hasshift, bool haswidth); */
  dspan_result = datespan_shift_scale(dspan1, date1, date2, true, true);
  char_result = datespan_out(dspan_result);
  printf("datespan_shift_scale(%s, %s, %s, true, trues): %s\n", dspan1_out, date1_out, date2_out, char_result);
  free(dspan_result); free(char_result);

  /* SpanSet *datespanset_shift_scale(spanset1, int shift, int width, bool hasshift, bool haswidth); */
  dspanset_result = datespanset_shift_scale(dspanset1, int32_in1, int32_in2, true, true);
  char_result = datespanset_out(dspanset_result);
  printf("datespanset_shift_scale(%s, %d, %d, true, true): %s\n", dspanset1_out, int32_in1, int32_in2, char_result);
  free(dspanset_result); free(char_result);

  /* Set *floatset_ceil(const Set *s); */
  fset_result = floatset_ceil(fset1);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_ceil(%s): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Set *floatset_degrees(const Set *s, bool normalize); */
  fset_result = floatset_degrees(fset1, true);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_degrees(%s): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Set *floatset_floor(const Set *s); */
  fset_result = floatset_floor(fset1);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_floor(%s): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Set *floatset_radians(const Set *s); */
  fset_result = floatset_radians(fset1);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_radians(%s): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Set *floatset_shift_scale(const Set *s, double shift, double width, bool hasshift, bool haswidth); */
  fset_result = floatset_shift_scale(fset1, float8_in1, float8_in2, true, true);
  char_result = floatset_out(fset_result, 6);
  printf("floatset_shift_scale(%s): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Span *floatspan_ceil(const Span *s); */
  fspan_result = floatspan_ceil(fspan1);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_ceil(%s): %s\n", fspan1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspan_degrees(const Span *s, bool normalize); */
  fspan_result = floatspan_degrees(fspan1, true);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_degrees(%s): %s\n", fspan1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspan_floor(const Span *s); */
  fspan_result = floatspan_floor(fspan1);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_floor(%s): %s\n", fspan1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspan_radians(const Span *s); */
  fspan_result = floatspan_radians(fspan1);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_radians(%s): %s\n", fspan1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspan_round(const Span *s, int maxdd); */
  fspan_result = floatspan_round(fspan1, 6);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_round(%s): %s\n", fspan1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift, bool haswidth); */
  fspan_result = floatspan_shift_scale(fspan1, float8_in1, float8_in2, true, true);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_shift_scale(%s, %lf, %lf): %s\n", fspan1_out, float8_in1, float8_in2, char_result);
  free(fspan_result); free(char_result);

  /* SpanSet *floatspanset_ceil(const SpanSet *ss); */
  fspanset_result = floatspanset_ceil(fspanset1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_ceil(%s): %s\n", fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *floatspanset_floor(const SpanSet *ss); */
  fspanset_result = floatspanset_floor(fspanset1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_floor(%s): %s\n", fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *floatspanset_degrees(const SpanSet *ss, bool normalize); */
  fspanset_result = floatspanset_degrees(fspanset1, true);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_degrees(%s): %s\n", fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *floatspanset_radians(const SpanSet *ss); */
  fspanset_result = floatspanset_radians(fspanset1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_radians(%s): %s\n", fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *floatspanset_round(spanset1, int maxdd); */
  fspanset_result = floatspanset_round(fspanset1, 6);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_round(%s): %s\n", fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *floatspanset_shift_scale(const SpanSet *ss, double shift, double width, bool hasshift, bool haswidth); */
  fspanset_result = floatspanset_shift_scale(fspanset1, float8_in1, float8_in2, true, true);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("floatspanset_shift_scale(%s, %lf, %lf): %s\n", fspanset1_out, float8_in1, float8_in2, char_result);
  free(fspanset_result); free(char_result);

  /* Set *intset_shift_scale(const Set *s, int shift, int width, bool hasshift, bool haswidth); */
  iset_result = intset_shift_scale(iset1, int32_in1, int32_in2, true, true);
  char_result = intset_out(iset_result);
  printf("intset_shift_scale(%s, %d, %d): %s\n", iset1_out, int32_in1, int32_in2, char_result);
  free(iset_result); free(char_result);

  /* Span *intspan_expand(const Span *s, int value); */
  ispan_result = intspan_expand(ispan1, int32_in1);
  char_result = intspan_out(ispan_result);
  printf("intspan_expand(%s, %d): %s\n", ispan1_out, int32_in1, char_result);
  free(ispan_result); free(char_result);

  /* Span *intspan_shift_scale(const Span *s, int shift, int width, bool hasshift, bool haswidth); */
  ispan_result = intspan_shift_scale(ispan1, int32_in1, int32_in2, true, true);
  char_result = intspan_out(ispan_result);
  printf("intspan_shift_scale(%s, %d, %ds): %s\n", fspan1_out, int32_in1, int32_in2, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *intspanset_shift_scale(spanset1, int shift, int width, bool hasshift, bool haswidth); */
  ispanset_result = intspanset_shift_scale(ispanset1, int32_in1, int32_in2, true, true);
  char_result = intspanset_out(ispanset_result);
  printf("intspanset_shift_scale(%s, %d, %d): %s\n", ispanset1_out, int32_in1, int32_in2, char_result);
  free(ispanset_result); free(char_result);

  /* Span *numspan_expand(const Span *s, double value); */
  fspan_result = floatspan_expand(fspan1, float8_in1);
  char_result = floatspan_out(fspan_result, 6);
  printf("numspan_expand(%s, %lf): %s\n", fspan1_out, float8_in1, char_result);
  free(fspan_result); free(char_result);

  /* Set *set_round(const Set *s, int maxdd); */
  fset_result = set_round(fset1, 6);
  char_result = floatset_out(fset_result, 6);
  printf("set_round(%s, 6): %s\n", fset1_out, char_result);
  free(fset_result); free(char_result);

  /* Set *textcat_text_textset(const text *txt, const Set *s); */
  textset_result = textcat_text_textset(text1, textset1);
  char_result = textset_out(textset_result);
  printf("textcat_text_textset(%s, %s): %s\n", text1_out, textset1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *textcat_textset_text(const Set *s, const text *txt); */
  textset_result = textcat_textset_text(textset1, text1);
  char_result = textset_out(textset_result);
  printf("textcat_textset_text(%s, %s): %s\n", textset1_out, text1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *textset_initcap(const Set *s); */
  textset_result = textset_initcap(textset1);
  char_result = textset_out(textset_result);
  printf("textset_initcap(%s): %s\n", textset1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *textset_lower(const Set *s); */
  textset_result = textset_lower(textset1);
  char_result = textset_out(textset_result);
  printf("textset_lower(%s): %s\n", textset1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *textset_upper(const Set *s); */
  textset_result = textset_upper(textset1);
  char_result = textset_out(textset_result);
  printf("textset_upper(%s): %s\n", textset1_out, char_result);
  free(textset_result); free(char_result);

  /* TimestampTz timestamptz_tprecision(TimestampTz t, const Interval *duration, TimestampTz torigin); */
  tstz_result = timestamptz_tprecision(tstz1, interv1, tstz2);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_tprecision(%s, %s, %s): %s\n", tstz1_out, interv1_out, tstz2_out, char_result);
  free(char_result);

  /* Set *tstzset_shift_scale(const Set *s, const Interval *shift, const Interval *duration); */
  tstzset_result = tstzset_shift_scale(tstzset1, interv1, interv2);
  char_result = tstzset_out(tstzset_result);
  printf("tstzset_shift_scale(%s, %s, %s): %s\n", tstzset1_out, interv1_out, interv2_out, char_result);
  free(tstzset_result); free(char_result);

  /* Set *tstzset_tprecision(const Set *s, const Interval *duration, TimestampTz torigin); */
  tstzset_result = tstzset_tprecision(tstzset1, interv1, tstz2);
  char_result = tstzset_out(tstzset_result);
  printf("tstzset_tprecision(%s, %s, %s): %s\n", tstzset1_out, interv1_out, tstz2_out, char_result);
  free(tstzset_result); free(char_result);

  /* Span *tstzspan_expand(const Span *s, const Interval *interv); */
  tstzspan_result = tstzspan_expand(tstzspan1, interv1);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_expand(%s): %s\n", interv1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* Span *tstzspan_shift_scale(const Span *s, const Interval *shift, const Interval *duration); */
  tstzspan_result = tstzspan_shift_scale(tstzspan1, interv1, interv2);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_shift_scale(%s, %s, %s): %s\n", tstzspan1_out, interv1_out, interv2_out, char_result);
  free(tstzspan_result); free(char_result);

  /* Span *tstzspan_tprecision(const Span *s, const Interval *duration, TimestampTz torigin); */
  tstzspan_result = tstzspan_tprecision(tstzspan1, interv1, tstz2);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_tprecision(%s, %s, %s): %s\n", tstzspan1_out, interv1_out, tstz2_out, char_result);
  free(tstzspan_result); free(char_result);

  /* SpanSet *tstzspanset_shift_scale(spanset1, const Interval *shift, const Interval *duration); */
  tstzspanset_result = tstzspanset_shift_scale(tstzspanset1, interv1, interv2);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("tstzspanset_shift_scale(%s, %s, %s): %s\n", tstzspanset1_out, interv1_out, interv2_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* SpanSet *tstzspanset_tprecision(spanset1, const Interval *duration, TimestampTz torigin); */
  tstzspanset_result = tstzspanset_tprecision(tstzspanset1, interv1, tstz2);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("tstzspanset_tprecision(%s, %s, %s): %s\n", tstzspanset1_out, interv1_out, tstz2_out, char_result);
  free(tstzspanset_result); free(char_result);

  /*****************************************************************************
   * Comparison functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* int set_cmp(const Set *s1, const Set *s2); */
  int32_result = set_cmp(iset1, iset2);
  printf("set_cmp(%s, %s): %d\n", iset1_out, iset2_out, int32_result);

  /* bool set_eq(const Set *s1, const Set *s2); */
  bool_result = set_eq(iset1, iset2);
  printf("set_eq(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool set_ge(const Set *s1, const Set *s2); */
  bool_result = set_ge(iset1, iset2);
  printf("set_ge(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool set_gt(const Set *s1, const Set *s2); */
  bool_result = set_gt(iset1, iset2);
  printf("set_gt(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool set_le(const Set *s1, const Set *s2); */
  bool_result = set_le(iset1, iset2);
  printf("set_le(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool set_lt(const Set *s1, const Set *s2); */
  bool_result = set_lt(iset1, iset2);
  printf("set_lt(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool set_ne(const Set *s1, const Set *s2); */
  bool_result = set_ne(iset1, iset2);
  printf("set_ne(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* int span_cmp(const Span *s1, const Span *s2); */
  int32_result = span_cmp(ispan1, ispan2);
  printf("span_cmp(%s, %s): %d\n", ispan1_out, ispan2_out, int32_result);

  /* bool span_eq(const Span *s1, const Span *s2); */
  bool_result = span_eq(ispan1, ispan2);
  printf("span_eq(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool span_ge(const Span *s1, const Span *s2); */
  bool_result = span_ge(ispan1, ispan2);
  printf("span_ge(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool span_gt(const Span *s1, const Span *s2); */
  bool_result = span_gt(ispan1, ispan2);
  printf("span_gt(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool span_le(const Span *s1, const Span *s2); */
  bool_result = span_le(ispan1, ispan2);
  printf("span_le(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool span_lt(const Span *s1, const Span *s2); */
  bool_result = span_lt(ispan1, ispan2);
  printf("span_lt(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool span_ne(const Span *s1, const Span *s2); */
  bool_result = span_ne(ispan1, ispan2);
  printf("span_ne(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* int spanset_cmp(const SpanSet *ss1, const SpanSet *ss2); */
  int32_result = spanset_cmp(ispanset1, ispanset2);
  printf("spanset_cmp(%s, %s): %d\n", ispanset1_out, ispanset2_out, int32_result);

  /* bool spanset_eq(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_eq(ispanset1, ispanset2);
  printf("spanset_eq(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool spanset_ge(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_ge(ispanset1, ispanset2);
  printf("spanset_ge(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool spanset_gt(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_gt(ispanset1, ispanset2);
  printf("spanset_gt(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool spanset_le(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_le(ispanset1, ispanset2);
  printf("spanset_le(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool spanset_lt(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_lt(ispanset1, ispanset2);
  printf("spanset_lt(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool spanset_ne(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = spanset_ne(ispanset1, ispanset2);
  printf("spanset_ne(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Bounding box functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Split functions */

  /* Span *set_spans(const Set *s); */
  ispanvector_result = set_spans(iset1);
  printf("set_spans(%s): {", iset1_out);
  for (int i = 0; i < iset1->count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < iset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Span *set_split_each_n_spans(const Set *s, int elems_per_span, int *count); */
  ispanvector_result = set_split_each_n_spans(iset1, 1, &count);
  printf("set_split_each_n_spans(%s, 1, &count): {", iset1_out);
  for (int i = 0; i < iset1->count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < iset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Span *set_split_n_spans(const Set *s, int span_count, int *count); */
  ispanvector_result = set_split_n_spans(iset1, 1, &count);
  printf("set_split_n_spans(%s, 1, &count): {", iset1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Span *spanset_spans(const SpanSet *ss); */
  ispanvector_result = spanset_spans(ispanset1);
  printf("spanset_spans(%s, &count): {", ispanset1_out);
  for (int i = 0; i < ispanset1->count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < ispanset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Span *spanset_split_each_n_spans(spanset1, int elems_per_span, int *count); */
  ispanvector_result = spanset_split_each_n_spans(ispanset1, 1, &count);
  printf("spanset_split_each_n_spans(%s, 1, &count): {", ispanset1_out);
  for (int i = 0; i < ispanset1->count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < ispanset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Span *spanset_split_n_spans(spanset1, int span_count, int *count); */
  ispanvector_result = spanset_split_n_spans(ispanset1, 1, &count);
  printf("spanset_split_n_spans(%s, 1, &count): {", ispanset1_out);
  for (int i = 0; i < ispanset1->count; i++)
  {
    char_result = intspan_out(&(ispanvector_result[i]));
    printf("%s", char_result);
    if (i < ispanset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanvector_result);

  /* Topological functions */
  printf("****************************************************************\n");

  /* bool adjacent_span_bigint(const Span *s, int64 i); */
  bool_result = adjacent_span_bigint(bspan1, int64_in1);
  printf("adjacent_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool adjacent_span_date(const Span *s, DateADT d); */
  bool_result = adjacent_span_date(dspan1, date1);
  printf("adjacent_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool adjacent_span_float(const Span *s, double d); */
  bool_result = adjacent_span_float(fspan1, float8_in1);
  printf("adjacent_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool adjacent_span_int(const Span *s, int i); */
  bool_result = adjacent_span_int(ispan1, int32_in1);
  printf("adjacent_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool adjacent_span_span(const Span *s1, const Span *s2); */
  bool_result = adjacent_span_span(ispan1, ispan2);
  printf("adjacent_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool adjacent_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = adjacent_span_spanset(ispan1, ispanset1);
  printf("adjacent_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool adjacent_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = adjacent_span_timestamptz(tstzspan1, tstz1);
  printf("adjacent_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_bigint(spanset1, int64 i); */
  bool_result = adjacent_spanset_bigint(bspanset1, int64_in1);
  printf("adjacent_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_date(spanset1, DateADT d); */
  bool_result = adjacent_spanset_date(dspanset1, date1);
  printf("adjacent_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_float(spanset1, double d); */
  bool_result = adjacent_spanset_float(fspanset1, float8_in1);
  printf("adjacent_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_int(spanset1, int i); */
  bool_result = adjacent_spanset_int(ispanset1, int32_in1);
  printf("adjacent_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = adjacent_spanset_timestamptz(tstzspanset1, tstz1);
  printf("adjacent_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_span(spanset1, const Span *s); */
  bool_result = adjacent_spanset_span(ispanset1, ispan1);
  printf("adjacent_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = adjacent_spanset_spanset(ispanset1, ispanset2);
  printf("adjacent_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool contained_bigint_set(int64 i, const Set *s); */
  bool_result = contained_bigint_set(int64_in1, bset1);
  printf("contained_bigint_set(%ld, %s): %c\n", int64_in1, bset1_out, bool_result ? 't' : 'n');

  /* bool contained_bigint_span(int64 i, const Span *s); */
  bool_result = contained_bigint_span(int64_in1, bspan1);
  printf("contained_bigint_span(%ld, %s): %c\n", int64_in1, bspan1_out, bool_result ? 't' : 'n');

  /* bool contained_bigint_spanset(int64 i, const SpanSet *ss); */
  bool_result = contained_bigint_spanset(int64_in1, bspanset1);
  printf("contained_bigint_spanset(%ld, %s): %c\n", int64_in1, bspanset1_out, bool_result ? 't' : 'n');

  /* bool contained_date_set(DateADT d, const Set *s); */
  bool_result = contained_date_set(date1, dset1);
  printf("contained_date_set(%s, %s): %c\n", date1_out, dset1_out, bool_result ? 't' : 'n');

  /* bool contained_date_span(DateADT d, const Span *s); */
  bool_result = contained_date_span(date1, dspan1);
  printf("contained_date_span(%s, %s): %c\n", date1_out, dspan1_out, bool_result ? 't' : 'n');

  /* bool contained_date_spanset(DateADT d, const SpanSet *ss); */
  bool_result = contained_date_spanset(date1, dspanset1);
  printf("contained_date_spanset(%s, %s): %c\n", date1_out, dspanset1_out, bool_result ? 't' : 'n');

  /* bool contained_float_set(double d, const Set *s); */
  bool_result = contained_float_set(float8_in1, fset1);
  printf("contained_float_set(%lf, %s): %c\n", float8_in1, dset1_out, bool_result ? 't' : 'n');

  /* bool contained_float_span(double d, const Span *s); */
  bool_result = contained_float_span(float8_in1, fspan1);
  printf("contained_float_span(%lf, %s): %c\n", float8_in1, dspan1_out, bool_result ? 't' : 'n');

  /* bool contained_float_spanset(double d, const SpanSet *ss); */
  bool_result = contained_float_spanset(float8_in1, fspanset1);
  printf("contained_float_spanset(%lf, %s): %c\n", float8_in1, dspanset1_out, bool_result ? 't' : 'n');

  /* bool contained_int_set(int i, const Set *s); */
  bool_result = contained_int_set(int32_in1, iset1);
  printf("contained_int_set(%d, %s): %c\n", int32_in1, iset1_out, bool_result ? 't' : 'n');

  /* bool contained_int_span(int i, const Span *s); */
  bool_result = contained_int_span(int32_in1, ispan1);
  printf("contained_int_span(%d, %s): %c\n", int32_in1, ispan1_out, bool_result ? 't' : 'n');

  /* bool contained_int_spanset(int i, const SpanSet *ss); */
  bool_result = contained_int_spanset(int32_in1, ispanset1);
  printf("contained_int_spanset(%d, %s): %c\n", int32_in1, ispanset1_out, bool_result ? 't' : 'n');

  /* bool contained_set_set(const Set *s1, const Set *s2); */
  bool_result = contained_set_set(iset1, iset2);
  printf("contained_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool contained_span_span(const Span *s1, const Span *s2); */
  bool_result = contained_span_span(ispan1, ispan2);
  printf("contained_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool contained_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = contained_span_spanset(ispan1, ispanset1);
  printf("contained_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool contained_spanset_span(spanset1, const Span *s); */
  bool_result = contained_spanset_span(ispanset1, ispan1);
  printf("contained_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool contained_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = contained_spanset_spanset(ispanset1, ispanset2);
  printf("contained_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool contained_text_set(const text *txt, const Set *s); */
  bool_result = contained_text_set(text1, textset1);
  printf("contained_text_set(%s, %s): %c\n", text1_out, textset1_out, bool_result ? 't' : 'n');

  /* bool contained_timestamptz_set(TimestampTz t, const Set *s); */
  bool_result = contained_timestamptz_set(tstz1, tstzset1);
  printf("contained_timestamptz_set(%s, %s): %c\n", tstz1_out, tstzset1_out, bool_result ? 't' : 'n');

  /* bool contained_timestamptz_span(TimestampTz t, const Span *s); */
  bool_result = contained_timestamptz_span(tstz1, tstzspan1);
  printf("contained_timestamptz_span(%s, %s): %c\n", tstz1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool contained_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  bool_result = contained_timestamptz_spanset(tstz1, tstzspanset1);
  printf("contained_timestamptz_spanset(%s, %s): %c\n", tstz1_out, tstzspanset1_out, bool_result ? 't' : 'n');

  /* bool contains_set_bigint(const Set *s, int64 i); */
  bool_result = contains_set_bigint(bset1, int64_in1);
  printf("contains_set_bigint(%s, %ld): %c\n", bset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool contains_set_date(const Set *s, DateADT d); */
  bool_result = contains_set_date(dset1, date1);
  printf("contains_set_date(%s, %s): %c\n", dset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool contains_set_float(const Set *s, double d); */
  bool_result = contains_set_float(fset1, float8_in1);
  printf("contains_set_float(%s, %lf): %c\n", fset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool contains_set_int(const Set *s, int i); */
  bool_result = contains_set_int(iset1, int32_in1);
  printf("contains_set_int(%s, %d): %c\n", iset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool contains_set_set(const Set *s1, const Set *s2); */
  bool_result = contains_set_set(iset1, iset2);
  printf("contains_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool contains_set_text(const Set *s, text *t); */
  bool_result = contains_set_text(textset1, text1);
  printf("contains_set_text(%s, %s): %c\n", textset1_out, text1_out, bool_result ? 't' : 'n');

  /* bool contains_set_timestamptz(const Set *s, TimestampTz t); */
  bool_result = contains_set_timestamptz(tstzset1, tstz1);
  printf("contains_set_timestamptz(%s, %s): %c\n", tstzset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool contains_span_bigint(const Span *s, int64 i); */
  bool_result = contains_span_bigint(bspan1, int64_in1);
  printf("contains_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool contains_span_date(const Span *s, DateADT d); */
  bool_result = contains_span_date(dspan1, date1);
  printf("contains_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool contains_span_float(const Span *s, double d); */
  bool_result = contains_span_float(fspan1, float8_in1);
  printf("contains_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool contains_span_int(const Span *s, int i); */
  bool_result = contains_span_int(ispan1, int32_in1);
  printf("contains_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool contains_span_span(const Span *s1, const Span *s2); */
  bool_result = contains_span_span(ispan1, ispan2);
  printf("contains_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool contains_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = contains_span_spanset(ispan1, ispanset1);
  printf("contains_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool contains_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = contains_span_timestamptz(tstzspan1, tstz1);
  printf("contains_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool contains_spanset_bigint(spanset1, int64 i); */
  bool_result = contains_spanset_bigint(bspanset1, int64_in1);
  printf("contains_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool contains_spanset_date(spanset1, DateADT d); */
  bool_result = contains_spanset_date(dspanset1, date1);
  printf("contains_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool contains_spanset_float(spanset1, double d); */
  bool_result = contains_spanset_float(fspanset1, float8_in1);
  printf("contains_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool contains_spanset_int(spanset1, int i); */
  bool_result = contains_spanset_int(ispanset1, int32_in1);
  printf("contains_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool contains_spanset_span(spanset1, const Span *s); */
  bool_result = contains_spanset_span(ispanset1, ispan1);
  printf("contains_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = contains_spanset_spanset(ispanset1, ispanset2);
  printf("contains_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool contains_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = contains_spanset_timestamptz(tstzspanset1, tstz1);
  printf("contains_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overlaps_set_set(const Set *s1, const Set *s2); */
  bool_result = overlaps_set_set(iset1, iset2);
  printf("overlaps_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool overlaps_span_span(const Span *s1, const Span *s2); */
  bool_result = overlaps_span_span(ispan1, ispan2);
  printf("overlaps_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool overlaps_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = overlaps_span_spanset(ispan1, ispanset1);
  printf("overlaps_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool overlaps_spanset_span(spanset1, const Span *s); */
  bool_result = overlaps_spanset_span(ispanset1, ispan1);
  printf("overlaps_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = overlaps_spanset_spanset(ispanset1, ispanset2);
  printf("overlaps_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* Position functions for set and span types */
  printf("****************************************************************\n");

  /* bool after_date_set(DateADT d, const Set *s); */
  bool_result = after_date_set(date1, dset1);
  printf("after_date_set(%s, %s): %c\n", date1_out, dset1_out, bool_result ? 't' : 'n');

  /* bool after_date_span(DateADT d, const Span *s); */
  bool_result = after_date_span(date1, dspan1);
  printf("after_date_span(%s, %s): %c\n", date1_out, dspan1_out, bool_result ? 't' : 'n');

  /* bool after_date_spanset(DateADT d, const SpanSet *ss); */
  bool_result = after_date_spanset(date1, dspanset1);
  printf("after_date_spanset(%s, %s): %c\n", date1_out, dspanset1_out, bool_result ? 't' : 'n');

  /* bool after_set_date(const Set *s, DateADT d); */
  bool_result = after_set_date(dset1, date1);
  printf("after_set_date(%s, %s): %c\n", dset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool after_set_timestamptz(const Set *s, TimestampTz t); */
  bool_result = after_set_timestamptz(tstzset1, tstz1);
  printf("after_set_timestamptz(%s, %s): %c\n", tstzset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool after_span_date(const Span *s, DateADT d); */
  bool_result = after_span_date(dspan1, date1);
  printf("after_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool after_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = after_span_timestamptz(tstzspan1, tstz1);
  printf("after_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool after_spanset_date(spanset1, DateADT d); */
  bool_result = after_spanset_date(dspanset1, date1);
  printf("after_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool after_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = after_spanset_timestamptz(tstzspanset1, tstz1);
  printf("after_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool after_timestamptz_set(TimestampTz t, const Set *s); */
  bool_result = after_timestamptz_set(tstz1, tstzset1);
  printf("after_timestamptz_set(%s, %s): %c\n", tstz1_out, tstzset1_out, bool_result ? 't' : 'n');

  /* bool after_timestamptz_span(TimestampTz t, const Span *s); */
  bool_result = after_timestamptz_span(tstz1, tstzspan1);
  printf("after_timestamptz_span(%s, %s): %c\n", tstz1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool after_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  bool_result = after_timestamptz_spanset(tstz1, tstzspanset1);
  printf("after_timestamptz_spanset(%s, %s): %c\n", tstz1_out, tstzspanset1_out, bool_result ? 't' : 'n');

  /* bool before_date_set(DateADT d, const Set *s); */
  bool_result = before_date_set(date1, dset1);
  printf("before_date_set(%s, %s): %c\n", date1_out, dset1_out, bool_result ? 't' : 'n');

  /* bool before_date_span(DateADT d, const Span *s); */
  bool_result = before_date_span(date1, dspan1);
  printf("before_date_span(%s, %s): %c\n", date1_out, dspan1_out, bool_result ? 't' : 'n');

  /* bool before_date_spanset(DateADT d, const SpanSet *ss); */
  bool_result = before_date_spanset(date1, dspanset1);
  printf("before_date_spanset(%s, %s): %c\n", date1_out, dspanset1_out, bool_result ? 't' : 'n');

  /* bool before_set_date(const Set *s, DateADT d); */
  bool_result = before_set_date(dset1, date1);
  printf("before_set_date(%s, %s): %c\n", dset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool before_set_timestamptz(const Set *s, TimestampTz t); */
  bool_result = before_set_timestamptz(tstzset1, tstz1);
  printf("before_set_timestamptz(%s, %s): %c\n", tstzset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool before_span_date(const Span *s, DateADT d); */
  bool_result = before_span_date(dspan1, date1);
  printf("before_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool before_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = before_span_timestamptz(tstzspan1, tstz1);
  printf("before_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool before_spanset_date(spanset1, DateADT d); */
  bool_result = before_spanset_date(dspanset1, date1);
  printf("before_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool before_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = before_spanset_timestamptz(tstzspanset1, tstz1);
  printf("before_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool before_timestamptz_set(TimestampTz t, const Set *s); */
  bool_result = before_timestamptz_set(tstz1, tstzset1);
  printf("before_timestamptz_set(%s, %s): %c\n", tstz1_out, tstzset1_out, bool_result ? 't' : 'n');

  /* bool before_timestamptz_span(TimestampTz t, const Span *s); */
  bool_result = before_timestamptz_span(tstz1, tstzspan1);
  printf("before_timestamptz_span(%s, %s): %c\n", tstz1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool before_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  bool_result = before_timestamptz_spanset(tstz1, tstzspanset1);
  printf("before_timestamptz_spanset(%s, %s): %c\n", tstz1_out, tstzspanset1_out, bool_result ? 't' : 'n');

  /* bool left_bigint_set(int64 i, const Set *s); */
  bool_result = left_bigint_set(int64_in1, bset1);
  printf("left_bigint_set(%ld, %s): %c\n", int64_in1, bset1_out, bool_result ? 't' : 'n');

  /* bool left_bigint_span(int64 i, const Span *s); */
  bool_result = left_bigint_span(int64_in1, bspan1);
  printf("left_bigint_span(%ld, %s): %c\n", int64_in1, bspan1_out, bool_result ? 't' : 'n');

  /* bool left_bigint_spanset(int64 i, const SpanSet *ss); */
  bool_result = left_bigint_spanset(int64_in1, bspanset1);
  printf("left_bigint_spanset(%ld, %s): %c\n", int64_in1, bspanset1_out, bool_result ? 't' : 'n');

  /* bool left_float_set(double d, const Set *s); */
  bool_result = left_float_set(float8_in1, fset1);
  printf("left_float_set(%lf, %s): %c\n", float8_in1, fset1_out, bool_result ? 't' : 'n');

  /* bool left_float_span(double d, const Span *s); */
  bool_result = left_float_span(float8_in1, fspan1);
  printf("left_float_span(%lf, %s): %c\n", float8_in1, fspan1_out, bool_result ? 't' : 'n');

  /* bool left_float_spanset(double d, const SpanSet *ss); */
  bool_result = left_float_spanset(float8_in1, fspanset1);
  printf("left_float_spanset(%lf, %s): %c\n", float8_in1, fspanset1_out, bool_result ? 't' : 'n');

  /* bool left_int_set(int i, const Set *s); */
  bool_result = left_int_set(int32_in1, iset1);
  printf("left_int_set(%d, %s): %c\n", int32_in1, iset1_out, bool_result ? 't' : 'n');

  /* bool left_int_span(int i, const Span *s); */
  bool_result = left_int_span(int32_in1, ispan1);
  printf("left_int_span(%d, %s): %c\n", int32_in1, ispan1_out, bool_result ? 't' : 'n');

  /* bool left_int_spanset(int i, const SpanSet *ss); */
  bool_result = left_int_spanset(int32_in1, ispanset1);
  printf("left_int_spanset(%d, %s): %c\n", int32_in1, ispanset1_out, bool_result ? 't' : 'n');

  /* bool left_set_bigint(const Set *s, int64 i); */
  bool_result = left_set_bigint(bset1, int64_in1);
  printf("left_set_bigint(%s, %ld): %c\n", bset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool left_set_float(const Set *s, double d); */
  bool_result = left_set_float(fset1, float8_in1);
  printf("left_set_float(%s, %lf): %c\n", fset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool left_set_int(const Set *s, int i); */
  bool_result = left_set_int(iset1, int32_in1);
  printf("left_set_int(%s, %d): %c\n", iset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool left_set_set(const Set *s1, const Set *s2); */
  bool_result = left_set_set(iset1, iset2);
  printf("left_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool left_set_text(const Set *s, text *txt); */
  bool_result = left_set_text(textset1, text1);
  printf("left_set_text(%s, %s): %c\n", textset1_out, text1_out, bool_result ? 't' : 'n');

  /* bool left_span_bigint(const Span *s, int64 i); */
  bool_result = left_span_bigint(bspan1, int64_in1);
  printf("left_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool left_span_float(const Span *s, double d); */
  bool_result = left_span_float(fspan1, float8_in1);
  printf("left_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool left_span_int(const Span *s, int i); */
  bool_result = left_span_int(ispan1, int32_in1);
  printf("left_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool left_span_span(const Span *s1, const Span *s2); */
  bool_result = left_span_span(ispan1, ispan2);
  printf("left_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool left_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = left_span_spanset(ispan1, ispanset1);
  printf("left_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool left_spanset_bigint(spanset1, int64 i); */
  bool_result = left_spanset_bigint(bspanset1, int64_in1);
  printf("left_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool left_spanset_float(spanset1, double d); */
  bool_result = left_spanset_float(fspanset1, float8_in1);
  printf("left_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool left_spanset_int(spanset1, int i); */
  bool_result = left_spanset_int(ispanset1, int32_in1);
  printf("left_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool left_spanset_span(spanset1, const Span *s); */
  bool_result = left_spanset_span(ispanset1, ispan1);
  printf("left_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool left_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = left_spanset_spanset(ispanset1, ispanset2);
  printf("left_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool left_text_set(const text *txt, const Set *s); */
  bool_result = left_text_set(text1, textset1);
  printf("left_text_set(%s, %s): %c\n", text1_out, textset1_out, bool_result ? 't' : 'n');

  /* bool overafter_date_set(DateADT d, const Set *s); */
  bool_result = overafter_date_set(date1, dset1);
  printf("overafter_date_set(%s, %s): %c\n", date1_out, dset1_out, bool_result ? 't' : 'n');

  /* bool overafter_date_span(DateADT d, const Span *s); */
  bool_result = overafter_date_span(date1, dspan1);
  printf("overafter_date_span(%s, %s): %c\n", date1_out, dspan1_out, bool_result ? 't' : 'n');

  /* bool overafter_date_spanset(DateADT d, const SpanSet *ss); */
  bool_result = overafter_date_spanset(date1, dspanset1);
  printf("overafter_date_spanset(%s, %s): %c\n", date1_out, dspanset1_out, bool_result ? 't' : 'n');

  /* bool overafter_set_date(const Set *s, DateADT d); */
  bool_result = overafter_set_date(dset1, date1);
  printf("overafter_set_date(%s, %s): %c\n", dset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overafter_set_timestamptz(const Set *s, TimestampTz t); */
  bool_result = overafter_set_timestamptz(tstzset1, tstz1);
  printf("overafter_set_timestamptz(%s, %s): %c\n", tstzset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overafter_span_date(const Span *s, DateADT d); */
  bool_result = overafter_span_date(dspan1, date1);
  printf("overafter_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overafter_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = overafter_span_timestamptz(tstzspan1, tstz1);
  printf("overafter_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overafter_spanset_date(spanset1, DateADT d); */
  bool_result = overafter_spanset_date(dspanset1, date1);
  printf("overafter_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overafter_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = overafter_spanset_timestamptz(tstzspanset1, tstz1);
  printf("overafter_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overafter_timestamptz_set(TimestampTz t, const Set *s); */
  bool_result = overafter_timestamptz_set(tstz1, tstzset1);
  printf("overafter_timestamptz_set(%s, %s): %c\n", tstz1_out, tstzset1_out, bool_result ? 't' : 'n');

  /* bool overafter_timestamptz_span(TimestampTz t, const Span *s); */
  bool_result = overafter_timestamptz_span(tstz1, tstzspan1);
  printf("overafter_timestamptz_span(%s, %s): %c\n", tstz1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool overafter_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  bool_result = overafter_timestamptz_spanset(tstz1, tstzspanset1);
  printf("overafter_timestamptz_spanset(%s, %s): %c\n", tstz1_out, tstzspanset1_out, bool_result ? 't' : 'n');

  /* bool overbefore_date_set(DateADT d, const Set *s); */
  bool_result = overbefore_date_set(date1, dset1);
  printf("overbefore_date_set(%s, %s): %c\n", date1_out, dset1_out, bool_result ? 't' : 'n');

  /* bool overbefore_date_span(DateADT d, const Span *s); */
  bool_result = overbefore_date_span(date1, dspan1);
  printf("overbefore_date_span(%s, %s): %c\n", date1_out, dspan1_out, bool_result ? 't' : 'n');

  /* bool overbefore_date_spanset(DateADT d, const SpanSet *ss); */
  bool_result = overbefore_date_spanset(date1, dspanset1);
  printf("overbefore_date_spanset(%s, %s): %c\n", date1_out, dspanset1_out, bool_result ? 't' : 'n');

  /* bool overbefore_set_date(const Set *s, DateADT d); */
  bool_result = overbefore_set_date(dset1, date1);
  printf("overbefore_set_date(%s, %s): %c\n", dset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overbefore_set_timestamptz(const Set *s, TimestampTz t); */
  bool_result = overbefore_set_timestamptz(tstzset1, tstz1);
  printf("overbefore_set_timestamptz(%s, %s): %c\n", tstzset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overbefore_span_date(const Span *s, DateADT d); */
  bool_result = overbefore_span_date(dspan1, date1);
  printf("overbefore_span_date(%s, %s): %c\n", dspan1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overbefore_span_timestamptz(const Span *s, TimestampTz t); */
  bool_result = overbefore_span_timestamptz(tstzspan1, tstz1);
  printf("overbefore_span_timestamptz(%s, %s): %c\n", tstzspan1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overbefore_spanset_date(spanset1, DateADT d); */
  bool_result = overbefore_spanset_date(dspanset1, date1);
  printf("overbefore_spanset_date(%s, %s): %c\n", dspanset1_out, date1_out, bool_result ? 't' : 'n');

  /* bool overbefore_spanset_timestamptz(spanset1, TimestampTz t); */
  bool_result = overbefore_spanset_timestamptz(tstzspanset1, tstz1);
  printf("overbefore_spanset_timestamptz(%s, %s): %c\n", tstzspanset1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool overbefore_timestamptz_set(TimestampTz t, const Set *s); */
  bool_result = overbefore_timestamptz_set(tstz1, tstzset1);
  printf("overbefore_timestamptz_set(%s, %s): %c\n", tstz1_out, tstzset1_out, bool_result ? 't' : 'n');

  /* bool overbefore_timestamptz_span(TimestampTz t, const Span *s); */
  bool_result = overbefore_timestamptz_span(tstz1, tstzspan1);
  printf("overbefore_timestamptz_span(%s, %s): %c\n", tstz1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool overbefore_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  bool_result = overbefore_timestamptz_spanset(tstz1, tstzspanset1);
  printf("overbefore_timestamptz_spanset(%s, %s): %c\n", tstz1_out, tstzspanset1_out, bool_result ? 't' : 'n');

  /* bool overleft_bigint_set(int64 i, const Set *s); */
  bool_result = overleft_bigint_set(int64_in1, bset1);
  printf("overleft_bigint_set(%ld, %s): %c\n", int64_in1, bset1_out, bool_result ? 't' : 'n');

  /* bool overleft_bigint_span(int64 i, const Span *s); */
  bool_result = overleft_bigint_span(int64_in1, bspan1);
  printf("overleft_bigint_span(%ld, %s): %c\n", int64_in1, bspan1_out, bool_result ? 't' : 'n');

  /* bool overleft_bigint_spanset(int64 i, const SpanSet *ss); */
  bool_result = overleft_bigint_spanset(int64_in1, bspanset1);
  printf("overleft_bigint_spanset(%ld, %s): %c\n", int64_in1, bspanset1_out, bool_result ? 't' : 'n');

  /* bool overleft_float_set(double d, const Set *s); */
  bool_result = overleft_float_set(float8_in1, fset1);
  printf("overleft_float_set(%lf, %s): %c\n", float8_in1, fset1_out, bool_result ? 't' : 'n');

  /* bool overleft_float_span(double d, const Span *s); */
  bool_result = overleft_float_span(float8_in1, fspan1);
  printf("overleft_float_span(%lf, %s): %c\n", float8_in1, fspan1_out, bool_result ? 't' : 'n');

  /* bool overleft_float_spanset(double d, const SpanSet *ss); */
  bool_result = overleft_float_spanset(float8_in1, fspanset1);
  printf("overleft_float_spanset(%lf, %s): %c\n", float8_in1, fspanset1_out, bool_result ? 't' : 'n');

  /* bool overleft_int_set(int i, const Set *s); */
  bool_result = overleft_int_set(int32_in1, iset1);
  printf("overleft_int_set(%d, %s): %c\n", int32_in1, iset1_out, bool_result ? 't' : 'n');

  /* bool overleft_int_span(int i, const Span *s); */
  bool_result = overleft_int_span(int32_in1, ispan1);
  printf("overleft_int_span(%d, %s): %c\n", int32_in1, ispan1_out, bool_result ? 't' : 'n');

  /* bool overleft_int_spanset(int i, const SpanSet *ss); */
  bool_result = overleft_int_spanset(int32_in1, ispanset1);
  printf("overleft_int_spanset(%d, %s): %c\n", int32_in1, ispanset1_out, bool_result ? 't' : 'n');

  /* bool overleft_set_bigint(const Set *s, int64 i); */
  bool_result = overleft_set_bigint(bset1, int64_in1);
  printf("overleft_set_bigint(%s, %ld): %c\n", bset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overleft_set_float(const Set *s, double d); */
  bool_result = overleft_set_float(fset1, float8_in1);
  printf("overleft_set_float(%s, %lf): %c\n", fset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overleft_set_int(const Set *s, int i); */
  bool_result = overleft_set_int(iset1, int32_in1);
  printf("overleft_set_int(%s, %d): %c\n", iset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overleft_set_set(const Set *s1, const Set *s2); */
  bool_result = overleft_set_set(iset1, iset2);
  printf("overleft_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool overleft_set_text(const Set *s, text *txt); */
  bool_result = overleft_set_text(textset1, text1);
  printf("overleft_set_text(%s, %s): %c\n", textset1_out, text1_out, bool_result ? 't' : 'n');

  /* bool overleft_span_bigint(const Span *s, int64 i); */
  bool_result = overleft_span_bigint(bspan1, int64_in1);
  printf("overleft_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overleft_span_float(const Span *s, double d); */
  bool_result = overleft_span_float(fspan1, float8_in1);
  printf("overleft_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overleft_span_int(const Span *s, int i); */
  bool_result = overleft_span_int(ispan1, int32_in1);
  printf("overleft_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overleft_span_span(const Span *s1, const Span *s2); */
  bool_result = overleft_span_span(ispan1, ispan2);
  printf("overleft_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool overleft_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = overleft_span_spanset(ispan1, ispanset1);
  printf("overleft_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool overleft_spanset_bigint(spanset1, int64 i); */
  bool_result = overleft_spanset_bigint(bspanset1, int64_in1);
  printf("overleft_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overleft_spanset_float(spanset1, double d); */
  bool_result = overleft_spanset_float(fspanset1, float8_in1);
  printf("overleft_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overleft_spanset_int(spanset1, int i); */
  bool_result = overleft_spanset_int(ispanset1, int32_in1);
  printf("overleft_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overleft_spanset_span(spanset1, const Span *s); */
  bool_result = overleft_spanset_span(ispanset1, ispan1);
  printf("overleft_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool overleft_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = overleft_spanset_spanset(ispanset1, ispanset2);
  printf("overleft_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool overleft_text_set(const text *txt, const Set *s); */
  bool_result = overleft_text_set(text1, textset1);
  printf("overleft_text_set(%s, %s): %c\n", text1_out, textset1_out, bool_result ? 't' : 'n');

  /* bool overright_bigint_set(int64 i, const Set *s); */
  bool_result = overright_bigint_set(int64_in1, bset1);
  printf("overright_bigint_set(%ld, %s): %c\n", int64_in1, bset1_out, bool_result ? 't' : 'n');

  /* bool overright_bigint_span(int64 i, const Span *s); */
  bool_result = overright_bigint_span(int64_in1, bspan1);
  printf("overright_bigint_span(%ld, %s): %c\n", int64_in1, bspan1_out, bool_result ? 't' : 'n');

  /* bool overright_bigint_spanset(int64 i, const SpanSet *ss); */
  bool_result = overright_bigint_spanset(int64_in1, bspanset1);
  printf("overright_bigint_spanset(%ld, %s): %c\n", int64_in1, bspanset1_out, bool_result ? 't' : 'n');

  /* bool overright_float_set(double d, const Set *s); */
  bool_result = overright_float_set(float8_in1, fset1);
  printf("overright_float_set(%lf, %s): %c\n", float8_in1, fset1_out, bool_result ? 't' : 'n');

  /* bool overright_float_span(double d, const Span *s); */
  bool_result = overright_float_span(float8_in1, fspan1);
  printf("overright_float_span(%lf, %s): %c\n",float8_in1, fspan1_out, bool_result ? 't' : 'n');

  /* bool overright_float_spanset(double d, const SpanSet *ss); */
  bool_result = overright_float_spanset(float8_in1, fspanset1);
  printf("overright_float_spanset(%lf, %s): %c\n", float8_in1, fspanset1_out, bool_result ? 't' : 'n');

  /* bool overright_int_set(int i, const Set *s); */
  bool_result = overright_int_set(int32_in1, iset1);
  printf("overright_int_set(%d, %s): %c\n", int32_in1, iset1_out, bool_result ? 't' : 'n');

  /* bool overright_int_span(int i, const Span *s); */
  bool_result = overright_int_span(int32_in1, ispan1);
  printf("overright_int_span(%d, %s): %c\n", int32_in1, ispan1_out, bool_result ? 't' : 'n');

  /* bool overright_int_spanset(int i, const SpanSet *ss); */
  bool_result = overright_int_spanset(int32_in1, ispanset1);
  printf("overright_int_spanset(%d, %s): %c\n", int32_in1, ispanset1_out, bool_result ? 't' : 'n');

  /* bool overright_set_bigint(const Set *s, int64 i); */
  bool_result = overright_set_bigint(bset1, int64_in1);
  printf("overright_set_bigint(%s, %ld): %c\n", bset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overright_set_float(const Set *s, double d); */
  bool_result = overright_set_float(fset1, float8_in1);
  printf("overright_set_float(%s, %lf): %c\n", fset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overright_set_int(const Set *s, int i); */
  bool_result = overright_set_int(iset1, int32_in1);
  printf("overright_set_int(%s, %d): %c\n", iset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overright_set_set(const Set *s1, const Set *s2); */
  bool_result = overright_set_set(iset1, iset2);
  printf("overright_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool overright_set_text(const Set *s, text *txt); */
  bool_result = overright_set_text(textset1, text1);
  printf("overright_set_text(%s, %s): %c\n", textset1_out, text1_out, bool_result ? 't' : 'n');

  /* bool overright_span_bigint(const Span *s, int64 i); */
  bool_result = overright_span_bigint(bspan1, int64_in1);
  printf("overright_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overright_span_float(const Span *s, double d); */
  bool_result = overright_span_float(fspan1, float8_in1);
  printf("overright_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overright_span_int(const Span *s, int i); */
  bool_result = overright_span_int(ispan1, int32_in1);
  printf("overright_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overright_span_span(const Span *s1, const Span *s2); */
  bool_result = overright_span_span(ispan1, ispan2);
  printf("overright_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool overright_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = overright_span_spanset(ispan1, ispanset1);
  printf("overright_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool overright_spanset_bigint(spanset1, int64 i); */
  bool_result = overright_spanset_bigint(bspanset1, int64_in1);
  printf("overright_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool overright_spanset_float(spanset1, double d); */
  bool_result = overright_spanset_float(fspanset1, float8_in1);
  printf("overright_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool overright_spanset_int(spanset1, int i); */
  bool_result = overright_spanset_int(ispanset1, int32_in1);
  printf("overright_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool overright_spanset_span(spanset1, const Span *s); */
  bool_result = overright_spanset_span(ispanset1, ispan1);
  printf("overright_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool overright_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = overright_spanset_spanset(ispanset1, ispanset2);
  printf("overright_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool overright_text_set(const text *txt, const Set *s); */
  bool_result = overright_text_set(text1, textset1);
  printf("overright_text_set(%s, %s): %c\n", text1_out, textset1_out, bool_result ? 't' : 'n');

  /* bool right_bigint_set(int64 i, const Set *s); */
  bool_result = right_bigint_set(int64_in1, bset1);
  printf("right_bigint_set(%ld, %s): %c\n", int64_in1, bset1_out, bool_result ? 't' : 'n');

  /* bool right_bigint_span(int64 i, const Span *s); */
  bool_result = right_bigint_span(int64_in1, bspan1);
  printf("right_bigint_span(%ld, %s): %c\n", int64_in1, bspan1_out, bool_result ? 't' : 'n');

  /* bool right_bigint_spanset(int64 i, const SpanSet *ss); */
  bool_result = right_bigint_spanset(int64_in1, bspanset1);
  printf("right_bigint_spanset(%ld, %s): %c\n", int64_in1, bspanset1_out, bool_result ? 't' : 'n');

  /* bool right_float_set(double d, const Set *s); */
  bool_result = right_float_set(float8_in1, fset1);
  printf("right_float_set(%lf, %s): %c\n", float8_in1, fset1_out, bool_result ? 't' : 'n');

  /* bool right_float_span(double d, const Span *s); */
  bool_result = right_float_span(float8_in1, fspan1);
  printf("right_float_span(%lf, %s): %c\n", float8_in1, fspan1_out, bool_result ? 't' : 'n');

  /* bool right_float_spanset(double d, const SpanSet *ss); */
  bool_result = right_float_spanset(float8_in1, fspanset1);
  printf("right_float_spanset(%lf, %s): %c\n", float8_in1, fspanset1_out, bool_result ? 't' : 'n');

  /* bool right_int_set(int i, const Set *s); */
  bool_result = right_int_set(int32_in1, iset1);
  printf("right_int_set(%d, %s): %c\n", int32_in1, iset1_out, bool_result ? 't' : 'n');

  /* bool right_int_span(int i, const Span *s); */
  bool_result = right_int_span(int32_in1, ispan1);
  printf("right_int_span(%d, %s): %c\n", int32_in1, ispan1_out, bool_result ? 't' : 'n');

  /* bool right_int_spanset(int i, const SpanSet *ss); */
  bool_result = right_int_spanset(int32_in1, ispanset1);
  printf("right_int_spanset(%d, %s): %c\n", int32_in1, ispanset1_out, bool_result ? 't' : 'n');

  /* bool right_set_bigint(const Set *s, int64 i); */
  bool_result = right_set_bigint(bset1, int64_in1);
  printf("right_set_bigint(%s, %ld): %c\n", bset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool right_set_float(const Set *s, double d); */
  bool_result = right_set_float(fset1, float8_in1);
  printf("right_set_float(%s, %lf): %c\n", fset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool right_set_int(const Set *s, int i); */
  bool_result = right_set_int(iset1, int32_in1);
  printf("right_set_int(%s, %d): %c\n", iset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool right_set_set(const Set *s1, const Set *s2); */
  bool_result = right_set_set(iset1, iset2);
  printf("right_set_set(%s, %s): %c\n", iset1_out, iset2_out, bool_result ? 't' : 'n');

  /* bool right_set_text(const Set *s, text *txt); */
  bool_result = right_set_text(textset1, text1);
  printf("right_set_text(%s, %s): %c\n", textset1_out, text1_out, bool_result ? 't' : 'n');

  /* bool right_span_bigint(const Span *s, int64 i); */
  bool_result = right_span_bigint(bspan1, int64_in1);
  printf("right_span_bigint(%s, %ld): %c\n", bspan1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool right_span_float(const Span *s, double d); */
  bool_result = right_span_float(fspan1, float8_in1);
  printf("right_span_float(%s, %lf): %c\n", fspan1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool right_span_int(const Span *s, int i); */
  bool_result = right_span_int(ispan1, int32_in1);
  printf("right_span_int(%s, %d): %c\n", ispan1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool right_span_span(const Span *s1, const Span *s2); */
  bool_result = right_span_span(ispan1, ispan2);
  printf("right_span_span(%s, %s): %c\n", ispan1_out, ispan2_out, bool_result ? 't' : 'n');

  /* bool right_span_spanset(const Span *s, const SpanSet *ss); */
  bool_result = right_span_spanset(ispan1, ispanset1);
  printf("right_span_spanset(%s, %s): %c\n", ispan1_out, ispanset1_out, bool_result ? 't' : 'n');

  /* bool right_spanset_bigint(spanset1, int64 i); */
  bool_result = right_spanset_bigint(bspanset1, int64_in1);
  printf("right_spanset_bigint(%s, %ld): %c\n", bspanset1_out, int64_in1, bool_result ? 't' : 'n');

  /* bool right_spanset_float(spanset1, double d); */
  bool_result = right_spanset_float(fspanset1, float8_in1);
  printf("right_spanset_float(%s, %lf): %c\n", fspanset1_out, float8_in1, bool_result ? 't' : 'n');

  /* bool right_spanset_int(spanset1, int i); */
  bool_result = right_spanset_int(ispanset1, int32_in1);
  printf("right_spanset_int(%s, %d): %c\n", ispanset1_out, int32_in1, bool_result ? 't' : 'n');

  /* bool right_spanset_span(spanset1, const Span *s); */
  bool_result = right_spanset_span(ispanset1, ispan1);
  printf("right_spanset_span(%s, %s): %c\n", ispanset1_out, ispan1_out, bool_result ? 't' : 'n');

  /* bool right_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  bool_result = right_spanset_spanset(ispanset1, ispanset2);
  printf("right_spanset_spanset(%s, %s): %c\n", ispanset1_out, ispanset2_out, bool_result ? 't' : 'n');

  /* bool right_text_set(const text *txt, const Set *s); */
  bool_result = right_text_set(text1, textset1);
  printf("right_text_set(%s, %s): %c\n", text1_out, textset1_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Set functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Set *intersection_bigint_set(int64 i, const Set *s); */
  bset_result = intersection_bigint_set(int64_in1, bset1);
  char_result = bset_result ? bigintset_out(bset_result) : text_out(text_null);
  printf("intersection_bigint_set(%ld, %s): %s\n", int64_in1, bset1_out, char_result);  if (bset_result)
    free(bset_result);
  free(char_result);

  /* Set *intersection_date_set(DateADT d, const Set *s); */
  dset_result = intersection_date_set(date1, dset1);
  char_result = dset_result ? dateset_out(dset_result) : text_out(text_null);
  printf("intersection_date_set(%s, %s): %s\n", date1_out, dset1_out, char_result);
  if (dset_result)
    free(dset_result);
  free(char_result);

  /* Set *intersection_float_set(double d, const Set *s); */
  fset_result = intersection_float_set(float8_in1, fset1);
  char_result = fset_result ? floatset_out(fset_result, 6) : text_out(text_null);
  printf("intersection_float_set(%lf1, %s): %s\n", float8_in1, fset1_out, char_result);
  if (fset_result)
    free(fset_result);
  free(char_result);

  /* Set *intersection_int_set(int i, const Set *s); */
  iset_result = intersection_int_set(int32_in1, iset1);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("intersection_int_set(%d, %s): %s\n", int32_in1, iset1_out, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* Set *intersection_set_bigint(const Set *s, int64 i); */
  bset_result = intersection_set_bigint(bset1, int64_in1);
  char_result = bset_result ? bigintset_out(bset_result) : text_out(text_null);
  printf("intersection_set_bigint(%s, %ld): %s\n", bset1_out, int64_in1, char_result);
  if (bset_result)
    free(bset_result);
  free(char_result);

  /* Set *intersection_set_date(const Set *s, DateADT d); */
  dset_result = intersection_set_date(dset1, date1);
  char_result = dset_result ? dateset_out(dset_result) : text_out(text_null);
  printf("intersection_set_date(%s, %s): %s\n", dset1_out, date1_out, char_result);
  if (dset_result)
    free(dset_result);
  free(char_result);

  /* Set *intersection_set_float(const Set *s, double d); */
  fset_result = intersection_set_float(fset1, float8_in1);
  char_result = fset_result ? floatset_out(fset_result, 6) : text_out(text_null);
  printf("intersection_set_float(%s, %lf): %s\n", fset1_out, float8_in1, char_result);
  if (fset_result)
    free(fset_result);
  free(char_result);

  /* Set *intersection_set_int(const Set *s, int i); */
  iset_result = intersection_set_int(iset1, int32_in1);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("intersection_set_int(%s, %d): %s\n", iset1_out, int32_in1, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* Set *intersection_set_set(const Set *s1, const Set *s2); */
  iset_result = intersection_set_set(iset1, iset2);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("intersection_set_set(%s, %s): %s\n", iset1_out, iset2_out, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* Set *intersection_set_text(const Set *s, const text *txt); */
  textset_result = intersection_set_text(textset1, text1);
  char_result = textset_result ? textset_out(textset_result) : text_out(text_null);
  printf("intersection_set_text(%s, %s): %s\n", textset1_out, text1_out, char_result);
  if (textset_result)
    free(textset_result);
  free(char_result);

  /* Set *intersection_set_timestamptz(const Set *s, TimestampTz t); */
  tstzset_result = intersection_set_timestamptz(tstzset1, tstz1);
  char_result = tstzset_result ? tstzset_out(tstzset_result) : text_out(text_null);
  printf("intersection_set_timestamptz(%s, %s): %s\n", tstzset1_out, tstz1_out, char_result);
  if (tstzset_result)
    free(tstzset_result);
  free(char_result);

  /* Span *intersection_span_bigint(const Span *s, int64 i); */
  bspan_result = intersection_span_bigint(bspan1, int64_in1);
  char_result = bspan_result ? bigintspan_out(bspan_result) : text_out(text_null);
  printf("intersection_span_bigint(%s, %ld): %s\n", bspan1_out, int64_in1, char_result);
  if (bspan_result)
    free(bspan_result);
  free(char_result);

  /* Span *intersection_span_date(const Span *s, DateADT d); */
  dspan_result = intersection_span_date(dspan1, date1);
  char_result = dspan_result ? datespan_out(dspan_result) : text_out(text_null);
  printf("intersection_span_date(%s, %s): %s\n", dspan1_out, date1_out, char_result);
  if (dspan_result)
    free(dspan_result);
  free(char_result);

  /* Span *intersection_span_float(const Span *s, double d); */
  fspan_result = intersection_span_float(fspan1, float8_in1);
  char_result = fspan_result ? floatspan_out(fspan_result, 6) : text_out(text_null);
  printf("intersection_span_float(%s, %lf): %s\n", fspan1_out, float8_in1, char_result);
  if (fspan_result)
    free(fspan_result);
  free(char_result);

  /* Span *intersection_span_int(const Span *s, int i); */
  ispan_result = intersection_span_int(ispan1, int32_in1);
  char_result = ispan_result ? intspan_out(ispan_result) : text_out(text_null);
  printf("intersection_span_int(%s, %d): %s\n", ispan1_out, int32_in1, char_result);
  if (ispan_result)
    free(ispan_result);
  free(char_result);

  /* Span *intersection_span_span(const Span *s1, const Span *s2); */
  ispan_result = intersection_span_span(ispan1, ispan2);
  char_result = ispan_result ? intspan_out(ispan_result) : text_out(text_null);
  printf("intersection_span_span(%s, %s): %s\n", ispan1_out, ispan2_out, char_result);
  if (ispan_result)
    free(ispan_result);
  free(char_result);

  /* SpanSet *intersection_span_spanset(const Span *s, const SpanSet *ss); */
  ispanset_result = intersection_span_spanset(ispan1, ispanset1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("intersection_span_spanset(%s, %s): %s\n", ispan1_out, ispanset1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* Span *intersection_span_timestamptz(const Span *s, TimestampTz t); */
  tstzspan_result = intersection_span_timestamptz(tstzspan1, tstz1);
  char_result = tstzspan_result ? tstzspan_out(tstzspan_result) : text_out(text_null);
  printf("intersection_span_timestamptz(%s, %s): %s\n", tstzspan1_out, tstz1_out, char_result);
  if (tstzspan_result)
    free(tstzspan_result);
  free(char_result);

  /* SpanSet *intersection_spanset_bigint(spanset1, int64 i); */
  bspanset_result = intersection_spanset_bigint(bspanset1, int64_in1);
  char_result = bspanset_result ? bigintspanset_out(bspanset_result) : text_out(text_null);
  printf("intersection_spanset_bigint(%s, %ld): %s\n", bspanset1_out, int64_in1, char_result);
  if (bspanset_result)
    free(bspanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_date(spanset1, DateADT d); */
  dspanset_result = intersection_spanset_date(dspanset1, date1);
  char_result = dspanset_result ? datespanset_out(dspanset_result) : text_out(text_null);
  printf("intersection_spanset_date(%s, %s): %s\n", dspanset1_out, date1_out, char_result);
  if (dspanset_result)
    free(dspanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_float(spanset1, double d); */
  fspanset_result = intersection_spanset_float(fspanset1, float8_in1);
  char_result = fspanset_result ? floatspanset_out(fspanset_result, 6) : text_out(text_null);
  printf("intersection_spanset_float(%s, %lf): %s\n", fspanset1_out, float8_in1, char_result);
  if (fspanset_result)
    free(fspanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_int(spanset1, int i); */
  ispanset_result = intersection_spanset_int(ispanset1, int32_in1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("intersection_spanset_int(%s, %d): %s\n", ispanset1_out, int32_in1, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_span(spanset1, const Span *s); */
  ispanset_result = intersection_spanset_span(ispanset1, ispan1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("intersection_spanset_span(%s, %s): %s\n", ispanset1_out, ispan1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  ispanset_result = intersection_spanset_spanset(ispanset1, ispanset2);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("intersection_spanset_spanset(%s, %s): %s\n", ispanset1_out, ispanset2_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *intersection_spanset_timestamptz(spanset1, TimestampTz t); */
  tstzspanset_result = intersection_spanset_timestamptz(tstzspanset1, tstz1);
  char_result = tstzspanset_result ? tstzspanset_out(tstzspanset_result) : text_out(text_null);
  printf("intersection_spanset_timestamptz(%s, %s): %s\n", tstzspanset1_out, tstz1_out, char_result);
  if (tstzspanset_result)
    free(tstzspanset_result);
  free(char_result);

  /* Set *intersection_text_set(const text *txt, const Set *s); */
  textset_result = intersection_text_set(text1, textset1);
  char_result = textset_result ? textset_out(textset_result) : text_out(text_null);
  printf("intersection_text_set(%s, %s): %s\n", text1_out, textset1_out, char_result);
  if (textset_result)
    free(textset_result);
  free(char_result);

  /* Set *intersection_timestamptz_set(TimestampTz t, const Set *s); */
  tstzset_result = intersection_timestamptz_set(tstz1, tstzset1);
  char_result = tstzset_result ? tstzset_out(tstzset_result) : text_out(text_null);
  printf("set_out(%s, %s): %s\n", tstz1_out, tstzset1_out, char_result);
  if (tstzset_result)
    free(tstzset_result);
  free(char_result);

  /* Set *minus_bigint_set(int64 i, const Set *s); */
  bset_result = minus_bigint_set(int64_in1, bset1);
  char_result = bset_result ? bigintset_out(bset_result) : text_out(text_null);
  printf("minus_bigint_set(%ld, %s): %s\n", int64_in1, bset1_out, char_result);
  if (bset_result)
    free(bset_result);
  free(char_result);

  /* SpanSet *minus_bigint_span(int64 i, const Span *s); */
  bspanset_result = minus_bigint_span(int64_in1, bspan1);
  char_result = bspanset_result ? bigintspanset_out(bspanset_result) : text_out(text_null);
  printf("minus_bigint_span(%ld, %s): %s\n", int64_in1, bspan1_out, char_result);
  if (bspanset_result)
    free(bspanset_result);
  free(char_result);

  /* SpanSet *minus_bigint_spanset(int64 i, const SpanSet *ss); */
  bspanset_result = minus_bigint_spanset(int64_in1, bspanset1);
  char_result = bspanset_result ? bigintspanset_out(bspanset_result) : text_out(text_null);
  printf("minus_bigint_spanset(%ld, %s): %s\n", int64_in1, bspanset1_out, char_result);
  if (bspanset_result)
    free(bspanset_result);
  free(char_result);

  /* Set *minus_date_set(DateADT d, const Set *s); */
  dset_result = minus_date_set(date1, dset1);
  char_result = dset_result ? dateset_out(dset_result) : text_out(text_null);
  printf("minus_date_set(%s, %s): %s\n", date1_out, dset1_out, char_result);
  if (dset_result)
    free(dset_result);
  free(char_result);

  /* SpanSet *minus_date_span(DateADT d, const Span *s); */
  dspanset_result = minus_date_span(date1, dspan1);
  char_result = dspanset_result ? datespanset_out(dspanset_result) : text_out(text_null);
  printf("minus_date_span(%s, %s): %s\n", date1_out, dspan1_out, char_result);
  if (dspanset_result)
    free(dspanset_result);
  free(char_result);

  /* SpanSet *minus_date_spanset(DateADT d, const SpanSet *ss); */
  dspanset_result = minus_date_spanset(date1, dspanset1);
  char_result = dspanset_result ? datespanset_out(dspanset_result) : text_out(text_null);
  printf("minus_date_spanset(%s, %s): %s\n", date1_out, dspanset1_out, char_result);
  if (dspanset_result)
    free(dspanset_result);
  free(char_result);

  /* Set *minus_float_set(double d, const Set *s); */
  fset_result = minus_float_set(float8_in1, fset1);
  char_result = fset_result ? floatset_out(fset_result, 6) : text_out(text_null);
  printf("minus_float_set(%lf, %s): %s\n", float8_in1, fset1_out, char_result);
  if (fset_result)
    free(fset_result);
  free(char_result);

  /* SpanSet *minus_float_span(double d, const Span *s); */
  fspanset_result = minus_float_span(float8_in1, fspan1);
  char_result = fspanset_result ? floatspanset_out(fspanset_result, 6) : text_out(text_null);
  printf("minus_float_span(%lf, %s): %s\n", float8_in1, fspan1_out, char_result);
  if (fspanset_result)
    free(fspanset_result);
  free(char_result);

  /* SpanSet *minus_float_spanset(double d, const SpanSet *ss); */
  fspanset_result = minus_float_spanset(float8_in1, fspanset1);
  char_result = fspanset_result ? floatspanset_out(fspanset_result, 6) : text_out(text_null);
  printf("minus_float_spanset(%lf, %s): %s\n", float8_in1, fspanset1_out, char_result);
  if (fspanset_result)
    free(fspanset_result);
  free(char_result);

  /* Set *minus_int_set(int i, const Set *s); */
  iset_result = minus_int_set(int32_in1, iset1);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("minus_int_set(%d, %s): %s\n", int32_in1, iset1_out, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* SpanSet *minus_int_span(int i, const Span *s); */
  ispanset_result = minus_int_span(int32_in1, ispan1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_int_span(%d, %s): %s\n", int32_in1, ispan1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_int_spanset(int i, const SpanSet *ss); */
  ispanset_result = minus_int_spanset(int32_in1, ispanset1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_int_spanset(%d, %s): %s\n", int32_in1, ispanset1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* Set *minus_set_bigint(const Set *s, int64 i); */
  bset_result = minus_set_bigint(bset1, int64_in1);
  char_result = bset_result ? bigintset_out(bset_result) : text_out(text_null);
  printf("minus_set_bigint(%s, %ld): %s\n", bset1_out, int64_in1, char_result);
  if (bset_result)
    free(bset_result);
  free(char_result);

  /* Set *minus_set_date(const Set *s, DateADT d); */
  dset_result = minus_set_date(dset1, date1);
  char_result = dset_result ? dateset_out(dset_result) : text_out(text_null);
  printf("minus_set_date(%s, %s): %s\n", dset1_out, date1_out, char_result);
  if (dset_result)
    free(dset_result);
  free(char_result);

  /* Set *minus_set_float(const Set *s, double d); */
  fset_result = minus_set_float(fset1, float8_in1);
  char_result = fset_result ? floatset_out(fset_result, 6) : text_out(text_null);
  printf("minus_set_float(%s, %lf): %s\n", fset1_out, float8_in1, char_result);
  if (fset_result)
    free(fset_result);
  free(char_result);

  /* Set *minus_set_int(const Set *s, int i); */
  iset_result = minus_set_int(iset1, int32_in1);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("minus_set_int(%s, %d): %s\n", iset1_out, int32_in1, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* Set *minus_set_set(const Set *s1, const Set *s2); */
  iset_result = minus_set_set(iset1, iset2);
  char_result = iset_result ? intset_out(iset_result) : text_out(text_null);
  printf("minus_set_set(%s, %s): %s\n", iset1_out, iset2_out, char_result);
  if (iset_result)
    free(iset_result);
  free(char_result);

  /* Set *minus_set_text(const Set *s, const text *txt); */
  textset_result = minus_set_text(textset1, text1);
  char_result = textset_result ? textset_out(textset_result) : text_out(text_null);
  printf("minus_set_text(%s, %s): %s\n", textset1_out, text1_out, char_result);
  if (textset_result)
    free(textset_result);
  free(char_result);

  /* Set *minus_set_timestamptz(const Set *s, TimestampTz t); */
  tstzset_result = minus_set_timestamptz(tstzset1, tstz1);
  char_result = tstzset_result ? tstzset_out(tstzset_result) : text_out(text_null);
  printf("minus_set_timestamptz(%s, %s): %s\n", tstzset1_out, tstz1_out, char_result);
  if (tstzset_result)
    free(tstzset_result);
  free(char_result);

  /* SpanSet *minus_span_bigint(const Span *s, int64 i); */
  bspanset_result = minus_span_bigint(bspan1, int64_in1);
  char_result = bspanset_result ? bigintspanset_out(bspanset_result) : text_out(text_null);
  printf("minus_span_bigint(%s, %ld): %s\n", bspan1_out, int64_in1, char_result);
  if (bspanset_result)
    free(bspanset_result);
  free(char_result);

  /* SpanSet *minus_span_date(const Span *s, DateADT d); */
  dspanset_result = minus_span_date(dspan1, date1);
  char_result = dspanset_result ? datespanset_out(dspanset_result) : text_out(text_null);
  printf("minus_span_date(%s, %s): %s\n", dspan1_out, date1_out, char_result);
  if (dspanset_result)
    free(dspanset_result);
  free(char_result);

  /* SpanSet *minus_span_float(const Span *s, double d); */
  fspanset_result = minus_span_float(fspan1, float8_in1);
  char_result = fspanset_result ? floatspanset_out(fspanset_result, 6) : text_out(text_null);
  printf("minus_span_float(%s, %lf): %s\n", fspan1_out, float8_in1, char_result);
  if (fspanset_result)
    free(fspanset_result);
  free(char_result);

  /* SpanSet *minus_span_int(const Span *s, int i); */
  ispanset_result = minus_span_int(ispan1, int32_in1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_span_int(%s, %d): %s\n", ispan1_out, int32_in1, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_span_span(const Span *s1, const Span *s2); */
  ispanset_result = minus_span_span(ispan1, ispan2);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_span_span(%s, %s): %s\n", ispan1_out, ispan2_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_span_spanset(const Span *s, const SpanSet *ss); */
  ispanset_result = minus_span_spanset(ispan1, ispanset1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_span_spanset(%s, %s): %s\n", ispan1_out, ispanset1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_span_timestamptz(const Span *s, TimestampTz t); */
  tstzspanset_result = minus_span_timestamptz(tstzspan1, tstz1);
  char_result = tstzspanset_result ? tstzspanset_out(tstzspanset_result) : text_out(text_null);
  printf("minus_span_timestamptz(%s, %s): %s\n", tstzspan1_out, tstz1_out, char_result);
  if (tstzspanset_result)
    free(tstzspanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_bigint(spanset1, int64 i); */
  bspanset_result = minus_spanset_bigint(bspanset1, int64_in1);
  char_result = bspanset_result ? bigintspanset_out(bspanset_result) : text_out(text_null);
  printf("minus_spanset_bigint(%s, %ld): %s\n", bspanset1_out, int64_in1, char_result);
  if (bspanset_result)
    free(bspanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_date(spanset1, DateADT d); */
  dspanset_result = minus_spanset_date(dspanset1, date1);
  char_result = dspanset_result ? datespanset_out(dspanset_result) : text_out(text_null);
  printf("minus_spanset_date(%s, %s): %s\n", dspanset1_out, date1_out, char_result);
  if (dspanset_result)
    free(dspanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_float(spanset1, double d); */
  fspanset_result = minus_spanset_float(fspanset1, float8_in1);
  char_result = dspanset_result ? floatspanset_out(fspanset_result, 6) : text_out(text_null);
  printf("spanset_out(%s, %lf): %s\n", fspanset1_out, float8_in1, char_result);
  if (fspanset_result)
    free(fspanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_int(spanset1, int i); */
  ispanset_result = minus_spanset_int(ispanset1, int32_in1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_spanset_int(%s, %d): %s\n", ispanset1_out, int32_in1, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_span(spanset1, const Span *s); */
  ispanset_result = minus_spanset_span(ispanset1, ispan1);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_spanset_span(%s, %s): %s\n", ispanset1_out, ispan1_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  ispanset_result = minus_spanset_spanset(ispanset1, ispanset2);
  char_result = ispanset_result ? intspanset_out(ispanset_result) : text_out(text_null);
  printf("minus_spanset_spanset(%s, %s): %s\n", ispanset1_out, ispanset2_out, char_result);
  if (ispanset_result)
    free(ispanset_result);
  free(char_result);

  /* SpanSet *minus_spanset_timestamptz(spanset1, TimestampTz t); */
  tstzspanset_result = minus_spanset_timestamptz(tstzspanset1, tstz1);
  char_result = tstzspanset_result ? tstzspanset_out(tstzspanset_result) : text_out(text_null);
  printf("minus_spanset_timestamptz(%s, %s): %s\n", tstzspanset1_out, tstz1_out, char_result);
  if (tstzspanset_result)
    free(tstzspanset_result);
  free(char_result);

  /* Set *minus_text_set(const text *txt, const Set *s); */
  textset_result = minus_text_set(text1, textset1);
  char_result = textset_result ? textset_out(textset_result) : text_out(text_null);
  printf("minus_text_set(%s, %s): %s\n", text1_out, textset1_out, char_result);
  if (textset_result)
    free(textset_result);
  free(char_result);

  /* Set *minus_timestamptz_set(TimestampTz t, const Set *s); */
  tstzset_result = minus_timestamptz_set(tstz1, tstzset1);
  char_result = tstzset_result ? tstzset_out(tstzset_result) : text_out(text_null);
  printf("minus_timestamptz_set(%s, %s): %s\n", tstz1_out, tstzset1_out, char_result);
  if (tstzset_result)
    free(tstzset_result);
  free(char_result);

  /* SpanSet *minus_timestamptz_span(TimestampTz t, const Span *s); */
  tstzspanset_result = minus_timestamptz_span(tstz1, tstzspan1);
  char_result = tstzspanset_result ? tstzspanset_out(tstzspanset_result) : text_out(text_null);
  printf("minus_timestamptz_span(%s, %s): %s\n", tstz1_out, tstzspan1_out, char_result);
  if (tstzspanset_result)
    free(tstzspanset_result);
  free(char_result);

  /* SpanSet *minus_timestamptz_spanset(TimestampTz t, const SpanSet *ss); */
  tstzspanset_result = minus_timestamptz_spanset(tstz1, tstzspanset1);
  char_result = tstzspanset_result ? tstzspanset_out(tstzspanset_result) : text_out(text_null);
  printf("minus_timestamptz_spanset(%s, %s): %s\n", tstz1_out, tstzspanset1_out, char_result);
  if (tstzspanset_result)
    free(tstzspanset_result);
  free(char_result);

  /* Set *union_bigint_set(int64 i, const Set *s); */
  bset_result = union_bigint_set(int64_in1, bset1);
  char_result = bigintset_out(bset_result);
  printf("union_bigint_set(%ld, %s): %s\n", int64_in1, bset1_out, char_result);
  free(bset_result); free(char_result);

  /* SpanSet *union_bigint_spanspan1, int64 i); */
  bspanset_result = union_bigint_span(bspan1, int64_in1);
  char_result = bigintspanset_out(bspanset_result);
  printf("union_bigint_span(%s, %ld): %s\n", bspan1_out, int64_in1, char_result);
  free(bspanset_result); free(char_result);

  /* SpanSet *union_bigint_spanset(int64 i, SpanSet *ss); */
  bspanset_result = union_bigint_spanset(int64_in1, bspanset1);
  char_result = bigintspanset_out(bspanset_result);
  printf("union_bigint_spanset(%ld, %s): %s\n", int64_in1, bspanset1_out, char_result);
  free(bspanset_result); free(char_result);

  /* Set *union_date_set(DateADT d, const Set *s); */
  dset_result = union_date_set(date1, dset1);
  char_result = dateset_out(dset_result);
  printf("union_date_set(%s, %s): %s\n", date1_out, dset1_out, char_result);
  free(dset_result); free(char_result);

  /* SpanSet *union_date_spanspan1, DateADT d); */
  dspanset_result = union_date_span(dspan1, date1);
  char_result = datespanset_out(dspanset_result);
  printf("union_date_span(%s, %s): %s\n", dspan1_out, date1_out, char_result);
  free(dspanset_result); free(char_result);

  /* SpanSet *union_date_spanset(DateADT d, SpanSet *ss); */
  dspanset_result = union_date_spanset(date1, dspanset1);
  char_result = datespanset_out(dspanset_result);
  printf("union_date_spanset(%s, %s): %s\n", date1_out, dspanset1_out, char_result);
  free(dspanset_result); free(char_result);

  /* Set *union_float_set(double d, const Set *s); */
  fset_result = union_float_set(float8_in1, fset1);
  char_result = floatset_out(fset_result, 6);
  printf("union_float_set(%lf, %s): %s\n", float8_in1, fset1_out, char_result);
  free(fset_result); free(char_result);

  /* SpanSet *union_float_span(const Span *s, double d); */
  fspanset_result = union_float_span(fspan1, float8_in1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("union_float_span(%s, %lf): %s\n", fspan1_out, float8_in1, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *union_float_spanset(double d, SpanSet *ss); */
  fspanset_result = union_float_spanset(float8_in1, fspanset1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("union_float_spanset(%lf, %s): %s\n", float8_in1, fspanset1_out, char_result);
  free(fspanset_result); free(char_result);

  /* Set *union_int_set(int i, const Set *s); */
  iset_result = union_int_set(int32_in1, iset1);
  char_result = intset_out(iset_result);
  printf("union_int_set(%d, %s): %s\n", int32_in1, iset1_out, char_result);
  free(iset_result); free(char_result);

  /* SpanSet *union_int_span(int i, const Span *s); */
  ispanset_result = union_int_span(int32_in1, ispan1);
  char_result = intspanset_out(ispanset_result);
  printf("union_int_span(%d, %s): %s\n", int32_in1, ispan1_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_int_spanset(int i, SpanSet *ss); */
  ispanset_result = union_int_spanset(int32_in1, ispanset1);
  char_result = intspanset_out(ispanset_result);
  printf("union_int_spanset(%d, %s): %s\n", int32_in1, ispanset1_out, char_result);
  free(ispanset_result); free(char_result);

  /* Set *union_set_bigint(const Set *s, int64 i); */
  bset_result = union_set_bigint(bset1, int64_in1);
  char_result = bigintset_out(bset_result);
  printf("union_set_bigint(%s, %ld): %s\n", bset1_out, int64_in1, char_result);
  free(bset_result); free(char_result);

  /* Set *union_set_date(const Set *s, DateADT d); */
  dset_result = union_set_date(dset1, date1);
  char_result = dateset_out(dset_result);
  printf("union_set_date(%s, %s): %s\n", dset1_out, date1_out, char_result);
  free(dset_result); free(char_result);

  /* Set *union_set_float(const Set *s, double d); */
  fset_result = union_set_float(fset1, float8_in1);
  char_result = floatset_out(fset_result, 6);
  printf("union_set_float(%s, %lf): %s\n", fset1_out, float8_in1, char_result);
  free(fset_result); free(char_result);

  /* Set *union_set_int(const Set *s, int i); */
  iset_result = union_set_int(iset1, int32_in1);
  char_result = intset_out(iset_result);
  printf("union_set_int(%s, %d): %s\n", iset1_out, int32_in1, char_result);
  free(iset_result); free(char_result);

  /* Set *union_set_set(const Set *s1, const Set *s2); */
  iset_result = union_set_set(iset1, iset2);
  char_result = intset_out(iset_result);
  printf("union_set_set(%s, %s): %s\n", iset1_out, iset2_out, char_result);
  free(iset_result); free(char_result);

  /* Set *union_set_text(const Set *s, const text *txt); */
  textset_result = union_set_text(textset1, text1);
  char_result = textset_out(textset_result);
  printf("union_set_text(%s, %s): %s\n", textset1_out, text1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *union_set_timestamptz(const Set *s, TimestampTz t); */
  tstzset_result = union_set_timestamptz(tstzset1, tstz1);
  char_result = tstzset_out(tstzset_result);
  printf("union_set_timestamptz(%s, %s): %s\n", tstzset1_out, tstz1_out, char_result);
  free(tstzset_result); free(char_result);

  /* SpanSet *union_span_bigint(const Span *s, int64 i); */
  bspanset_result = union_span_bigint(bspan1, int64_in1);
  char_result = bigintspanset_out(bspanset_result);
  printf("union_span_bigint(%s, %ld): %s\n", bspan1_out, int64_in1, char_result);
  free(bspanset_result); free(char_result);

  /* SpanSet *union_span_date(const Span *s, DateADT d); */
  dspanset_result = union_span_date(dspan1, date1);
  char_result = datespanset_out(dspanset_result);
  printf("union_span_date(%s, %s): %s\n", dspan1_out, date1_out, char_result);
  free(dspanset_result); free(char_result);

  /* SpanSet *union_span_float(const Span *s, double d); */
  fspanset_result = union_span_float(fspan1, float8_in1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("union_span_float(%s, %lf): %s\n", fspan1_out, float8_in1, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *union_span_int(const Span *s, int i); */
  ispanset_result = union_span_int(ispan1, int32_in1);
  char_result = intspanset_out(ispanset_result);
  printf("union_span_int(%s, %d): %s\n", ispan1_out, int32_in1, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_span_span(const Span *s1, const Span *s2); */
  ispanset_result = union_span_span(ispan1, ispan2);
  char_result = intspanset_out(ispanset_result);
  printf("union_span_span(%s, %s): %s\n", ispan1_out, ispan2_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_span_spanset(const Span *s, const SpanSet *ss); */
  ispanset_result = union_span_spanset(ispan1, ispanset1);
  char_result = intspanset_out(ispanset_result);
  printf("union_span_spanset(%s, %s): %s\n", ispan1_out, ispanset1_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_span_timestamptz(const Span *s, TimestampTz t); */
  tstzspanset_result = union_span_timestamptz(tstzspan1, tstz1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("union_span_timestamptz(%s, %s): %s\n", tstzspan1_out, tstz1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* SpanSet *union_spanset_bigint(spanset1, int64 i); */
  bspanset_result = union_spanset_bigint(bspanset1, int64_in1);
  char_result = bigintspanset_out(bspanset_result);
  printf("union_spanset_bigint(%s, %ld): %s\n", bspanset1_out, int64_in1, char_result);
  free(bspanset_result); free(char_result);

  /* SpanSet *union_spanset_date(spanset1, DateADT d); */
  dspanset_result = union_spanset_date(dspanset1, date1);
  char_result = datespanset_out(dspanset_result);
  printf("union_spanset_date(%s, %s): %s\n", dspanset1_out, date1_out, char_result);
  free(dspanset_result); free(char_result);

  /* SpanSet *union_spanset_float(spanset1, double d); */
  fspanset_result = union_spanset_float(fspanset1, float8_in1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("union_spanset_float(%s, %lf): %s\n", fspanset1_out, float8_in1, char_result);
  free(fspanset_result); free(char_result);

  /* SpanSet *union_spanset_int(spanset1, int i); */
  ispanset_result = union_spanset_int(ispanset1, int32_in1);
  char_result = intspanset_out(ispanset_result);
  printf("union_spanset_int(%s, %d): %s\n", ispanset1_out, int32_in1, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_spanset_span(spanset1, const Span *s); */
  ispanset_result = union_spanset_span(ispanset1, ispan1);
  char_result = intspanset_out(ispanset_result);
  printf("union_spanset_span(%s, %s): %s\n", ispanset1_out, ispan1_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2); */
  ispanset_result = union_spanset_spanset(ispanset1, ispanset2);
  char_result = intspanset_out(ispanset_result);
  printf("union_spanset_spanset(%s, %s): %s\n", ispanset1_out, ispanset2_out, char_result);
  free(ispanset_result); free(char_result);

  /* SpanSet *union_spanset_timestamptz(spanset1, TimestampTz t); */
  tstzspanset_result = union_spanset_timestamptz(tstzspanset1, tstz1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("union_spanset_timestamptz(%s, %s): %s\n", tstzspanset1_out, tstz1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* Set *union_text_set(const text *txt, const Set *s); */
  textset_result = union_text_set(text1, textset1);
  char_result = textset_out(textset_result);
  printf("union_text_set(%s, %s): %s\n", text1_out, textset1_out, char_result);
  free(textset_result); free(char_result);

  /* Set *union_timestamptz_set(TimestampTz t, const Set *s); */
  tstzset_result = union_timestamptz_set(tstz1, tstzset1);
  char_result = tstzset_out(tstzset_result);
  printf("union_timestamptz_set(%s, %s): %s\n", tstz1_out, tstzset1_out, char_result);
  free(tstzset_result); free(char_result);

  /* SpanSet *union_timestamptz_span(TimestampTz t, const Span *s); */
  tstzspanset_result = union_timestamptz_span(tstz1, tstzspan1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("union_timestamptz_span(%s, %s): %s\n", tstz1_out, tstzspan1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* SpanSet *union_timestamptz_spanset(TimestampTz t, SpanSet *ss); */
  tstzspanset_result = union_timestamptz_spanset(tstz1, tstzspanset1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("union_timestamptz_spanset(%s, %s): %s\n", tstz1_out, tstzspanset1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /*****************************************************************************
   * Distance functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* int64 distance_bigintset_bigintset(const Set *s1, const Set *s2); */
  int64_result = distance_bigintset_bigintset(bset1, bset2);
  printf("distance_bigintset_bigintset(%s, %s): %ld\n", bset1_out, bset2_out, int64_result);

  /* int64 distance_bigintspan_bigintspan(const Span *s1, const Span *s2); */
  int64_result = distance_bigintspan_bigintspan(bspan1, bspan2);
  printf("distance_bigintspan_bigintspan(%s, %s): %ld\n", bspan1_out, bspan2_out, int64_result);

  /* int64 distance_bigintspanset_bigintspan(spanset1, const Span *s); */
  int64_result = distance_bigintspanset_bigintspan(bspanset1, bspan1);
  printf("distance_bigintspanset_bigintspan(%s, %s): %ld\n", bspanset1_out, bspan1_out, int64_result);

  /* int64 distance_bigintspanset_bigintspanset(const SpanSet *ss1, const SpanSet *ss2); */
  int64_result = distance_bigintspanset_bigintspanset(bspanset1, bspanset2);
  printf("distance_bigintspanset_bigintspanset(%s, %s): %ld\n", bspanset1_out, bspanset2_out, int64_result);

  /* int distance_dateset_dateset(const Set *s1, const Set *s2); */
  int32_result = distance_dateset_dateset(dset1, dset2);
  printf("distance_dateset_dateset(%s, %s): %d\n", dset1_out, dset2_out, int32_result);

  /* int distance_datespan_datespan(const Span *s1, const Span *s2); */
  int32_result = distance_datespan_datespan(dspan1, dspan2);
  printf("distance_datespan_datespan(%s, %s): %d\n", dspan1_out, dspan2_out, int32_result);

  /* int distance_datespanset_datespan(spanset1, const Span *s); */
  int32_result = distance_datespanset_datespan(dspanset1, dspan1);
  printf("distance_datespanset_datespan(%s, %s): %d\n", dspanset1_out, dspan1_out, int32_result);

  /* int distance_datespanset_datespanset(const SpanSet *ss1, const SpanSet *ss2); */
  int32_result = distance_datespanset_datespanset(dspanset1, dspanset2);
  printf("distance_datespanset_datespanset(%s, %s): %d\n", dspanset1_out, dspanset2_out, int32_result);

  /* double distance_floatset_floatset(const Set *s1, const Set *s2); */
  float8_result = distance_floatset_floatset(fset1, fset2);
  printf("distance_floatset_floatset(%s, %s): %lf\n", fset1_out, fset2_out, float8_result);

  /* double distance_floatspan_floatspan(const Span *s1, const Span *s2); */
  float8_result = distance_floatspan_floatspan(fspan1, fspan2);
  printf("distance_floatspan_floatspan(%s, %s): %lf\n", fspan1_out, fspan2_out, float8_result);

  /* double distance_floatspanset_floatspan(spanset1, const Span *s); */
  float8_result = distance_floatspanset_floatspan(fspanset1, fspan1);
  printf("distance_floatspanset_floatspan(%s, %s): %lf\n", fspanset1_out, fspan1_out, float8_result);

  /* double distance_floatspanset_floatspanset(const SpanSet *ss1, const SpanSet *ss2); */
  float8_result = distance_floatspanset_floatspanset(fspanset1, fspanset2);
  printf("distance_floatspanset_floatspanset(%s, %s): %lf\n", fspanset1_out, fspanset2_out, float8_result);

  /* int distance_intset_intset(const Set *s1, const Set *s2); */
  int32_result = distance_intset_intset(iset1, iset2);
  printf("distance_intset_intset(%s, %s): %d\n", iset1_out, iset2_out, int32_result);

  /* int distance_intspan_intspan(const Span *s1, const Span *s2); */
  int32_result = distance_intspan_intspan(ispan1, ispan2);
  printf("distance_intspan_intspan(%s, %s): %d\n", ispan1_out, ispan2_out, int32_result);

  /* int distance_intspanset_intspan(spanset1, const Span *s); */
  int32_result = distance_intspanset_intspan(ispanset1, ispan1);
  printf("distance_intspanset_intspan(%s, %s): %d\n", ispanset1_out, ispan1_out, int32_result);

  /* int distance_intspanset_intspanset(const SpanSet *ss1, const SpanSet *ss2); */
  int32_result = distance_intspanset_intspanset(ispanset1, ispanset2);
  printf("distance_intspanset_intspanset(%s, %s): %d\n", ispanset1_out, ispanset2_out, int32_result);

  /* int64 distance_set_bigint(const Set *s, int64 i); */
  int64_result = distance_set_bigint(bset1, int64_in1);
  printf("distance_set_bigint(%s, %ld): %ld\n", bset1_out, int64_in1, int64_result);

  /* int distance_set_date(const Set *s, DateADT d); */
  int32_result = distance_set_date(dset1, date1);
  printf("distance_set_date(%s, %s): %d\n", dset1_out, date1_out, int32_result);

  /* double distance_set_float(const Set *s, double d); */
  float8_result = distance_set_float(fset1, float8_in1);
  printf("distance_set_float(%s, %lf): %lf\n", fset1_out, float8_in1, float8_result);

  /* int distance_set_int(const Set *s, int i); */
  int32_result = distance_set_int(iset1, int32_in1);
  printf("distance_set_int(%s, %d): %d\n", iset1_out, int32_in1, int32_result);

  /* double distance_set_timestamptz(const Set *s, TimestampTz t); */
  float8_result = distance_set_timestamptz(tstzset1, tstz1);
  printf("distance_set_timestamptz(%s, %s): %lf\n", tstzset1_out, tstz1_out, float8_result);

  /* int64 distance_span_bigint(const Span *s, int64 i); */
  int64_result = distance_span_bigint(bspan1, int64_in1);
  printf("distance_span_bigint(%s, %ld): %ld\n", bspan1_out, int64_in1, int64_result);

  /* int distance_span_date(const Span *s, DateADT d); */
  int32_result = distance_span_date(dspan1, date1);
  printf("distance_span_date(%s, %s): %d\n", dspan1_out, date1_out, int32_result);

  /* double distance_span_float(const Span *s, double d); */
  float8_result = distance_span_float(fspan1, float8_in1);
  printf("distance_span_float(%s, %lf): %lf\n", fspan1_out, float8_in1, float8_result);

  /* int distance_span_int(const Span *s, int i); */
  int32_result = distance_span_int(ispan1, int32_in1);
  printf("distance_span_int(%s, %d): %d\n", ispan1_out, int32_in1, int32_result);

  /* double distance_span_timestamptz(const Span *s, TimestampTz t); */
  float8_result = distance_span_timestamptz(tstzspan1, tstz1);
  printf("distance_span_timestamptz(%s, %s): %lf\n", tstzspan1_out, tstz1_out, float8_result);

  /* int64 distance_spanset_bigint(spanset1, int64 i); */
  int64_result = distance_spanset_bigint(bspanset1, int64_in1);
  printf("distance_spanset_bigint(%s, %ld): %ld\n", bspanset1_out, int64_in1, int64_result);

  /* int distance_spanset_date(spanset1, DateADT d); */
  int32_result = distance_spanset_date(dspanset1, date1);
  printf("distance_spanset_date(%s, %s): %d\n", dspanset1_out, date1_out, int32_result);

  /* double distance_spanset_float(spanset1, double d); */
  float8_result = distance_spanset_float(fspanset1, float8_in1);
  printf("distance_spanset_float(%s, %lf): %lf\n", fspanset1_out, float8_in1, float8_result);

  /* int distance_spanset_int(spanset1, int i); */
  int32_result = distance_spanset_int(ispanset1, int32_in1);
  printf("distance_spanset_int(%s, %d): %d\n", ispanset1_out, int32_in1, int32_result);

  /* double distance_spanset_timestamptz(spanset1, TimestampTz t); */
  float8_result = distance_spanset_timestamptz(tstzspanset1, tstz1);
  printf("distance_spanset_timestamptz(%s, %s): %lf\n", tstzspanset1_out, tstz1_out, float8_result);

  /* double distance_tstzset_tstzset(const Set *s1, const Set *s2); */
  float8_result = distance_tstzset_tstzset(tstzset1, tstzset2);
  printf("distance_tstzset_tstzset(%s, %s): %lf\n", tstzset1_out, tstzset2_out, float8_result);

  /* double distance_tstzspan_tstzspan(const Span *s1, const Span *s2); */
  float8_result = distance_tstzspan_tstzspan(tstzspan1, tstzspan2);
  printf("distance_tstzspan_tstzspan(%s, %s): %lf\n", tstzspan1_out, tstzspan2_out, float8_result);

  /* double distance_tstzspanset_tstzspan(spanset1, const Span *s); */
  float8_result = distance_tstzspanset_tstzspan(tstzspanset1, tstzspan1);
  printf("distance_tstzspanset_tstzspan(%s, %s): %lf\n", tstzspanset1_out, tstzspan1_out, float8_result);

  /* double distance_tstzspanset_tstzspanset(const SpanSet *ss1, const SpanSet *ss2); */
  float8_result = distance_tstzspanset_tstzspanset(tstzspanset1, tstzspanset2);
  printf("distance_tstzspanset_tstzspanset(%s, %s): %lf\n", tstzspanset1_out, tstzspanset2_out, float8_result);

  /*****************************************************************************
   * Aggregate functions for set and span types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Span *bigint_extent_transfn(Span *state, int64 i); */
  ispan_result = bigint_extent_transfn(NULL, int64_in1);
  ispan_result = bigint_extent_transfn(ispan_result, int64_in2);
  char_result = bigintspan_out(ispan_result);
  printf("bigint_extent aggregate: %ld, %ld -> %s\n", int64_in1, int64_in2, char_result);
  free(ispan_result); free(char_result);

  /* Set *bigint_union_transfn(Set *state, int64 i); */
  bset_result = bigint_union_transfn(NULL, int64_in1);
  bset_result = bigint_union_transfn(bset_result, int64_in2);
  char_result = bigintset_out(bset_result);
  printf("bigint_union aggregate: %ld, %ld -> %s\n", int64_in1, int64_in2, char_result);
  free(bset_result); free(char_result);

  /* Span *date_extent_transfn(Span *state, DateADT d); */
  dspan_result = date_extent_transfn(NULL, date1);
  dspan_result = date_extent_transfn(dspan_result, date2);
  char_result = datespan_out(dspan_result);
  printf("date_extent aggregate: %s, %s -> %s\n", date1_out, date2_out, char_result);
  free(dspan_result); free(char_result);

  /* Set *date_union_transfn(Set *state, DateADT d); */
  dset_result = date_union_transfn(NULL, date1);
  dset_result = date_union_transfn(dset_result, date2);
  char_result = dateset_out(dset_result);
  printf("date_union aggregate: %s, %s -> %s\n", date1_out, date2_out, char_result);
  free(dset_result); free(char_result);

  /* Span *float_extent_transfn(Span *state, double d); */
  fspan_result = float_extent_transfn(NULL, float8_in1);
  fspan_result = float_extent_transfn(fspan_result, float8_in2);
  char_result = floatspan_out(fspan_result, 4);
  printf("float_extent aggregate: %lf, %lf -> %s\n", float8_in1, float8_in2, char_result);
  free(fspan_result); free(char_result);

  /* Set *float_union_transfn(Set *state, double d); */
  fset_result = float_union_transfn(NULL, float8_in1);
  fset_result = float_union_transfn(fset_result, float8_in2);
  char_result = floatset_out(fset_result, 6);
  printf("float_union aggregate: %lf, %lf -> %s\n", float8_in1, float8_in2, char_result);
  free(fset_result); free(char_result);

  /* Span *int_extent_transfn(Span *state, int i); */
  ispan_result = int_extent_transfn(NULL, int32_in1);
  ispan_result = int_extent_transfn(ispan_result, int32_in2);
  char_result = intspan_out(ispan_result);
  printf("int_extent aggregate: %d, %d -> %s\n", int32_in1, int32_in2, char_result);
  free(ispan_result); free(char_result);

  /* Span *int_extent_transfn(Span *state, int i); */
  ispan_result = int_extent_transfn(NULL, int32_in1);
  ispan_result = int_extent_transfn(ispan_result, int32_in2);
  char_result = intspan_out(ispan_result);
  printf("int_extent aggregate: %d, %d -> %s\n", int32_in1, int32_in2, char_result);
  free(ispan_result); free(char_result);

  /* Set *int_union_transfn(Set *state, int32 i); */
  iset_result = int_union_transfn(NULL, int32_in1);
  iset_result = int_union_transfn(iset_result, int32_in2);
  iset_result = set_union_finalfn(iset_result);
  char_result = intset_out(iset_result);
  printf("set_union aggregate: %d, %d -> %s\n", int32_in1, int32_in2, char_result);
  free(iset_result); free(char_result);

  /* Span *set_extent_transfn(Span *state, const Set *s); */
  ispan_result = set_extent_transfn(NULL, iset1);
  ispan_result = set_extent_transfn(ispan_result, iset2);
  char_result = intspan_out(ispan_result);
  printf("int_extent aggregate: %s, %s -> %s\n", iset1_out, iset2_out, char_result);
  free(ispan_result); free(char_result);

  /* Set *set_union_transfn(Set *state, Set *s); */
  iset_result = set_union_transfn(NULL, iset1);
  iset_result = set_union_transfn(iset_result, iset2);
  iset_result = set_union_finalfn(iset_result);
  char_result = intset_out(iset_result);
  printf("set_union aggregate: %s, %s -> %s\n", iset1_out, iset2_out, char_result);
  free(iset_result); free(char_result);

  /* Span *span_extent_transfn(Span *state, const Span *s); */
  ispan_result = span_extent_transfn(NULL, ispan1);
  ispan_result = span_extent_transfn(ispan_result, ispan2);
  char_result = intspan_out(ispan_result);
  printf("span_extent aggregate: %s, %s -> %s\n", ispan1_out, ispan2_out, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *span_union_transfn(SpanSet *state, const Span *s); */
  ispanset_result = span_union_transfn(NULL, ispan1);
  ispanset_result = span_union_transfn(ispanset_result, ispan2);
  ispanset_result = spanset_union_finalfn(ispanset_result);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_union aggregate: %s, %s -> %s\n", ispan1_out, ispan2_out, char_result);
  free(ispanset_result); free(char_result);

  /* Span *spanset_extent_transfn(Span *state, const SpanSet *ss); */
  ispan_result = spanset_extent_transfn(NULL, ispanset1);
  ispan_result = spanset_extent_transfn(ispan_result, ispanset2);
  char_result = intspan_out(ispan_result);
  printf("spanset_extent aggregate: %s, %s -> %s\n", ispanset1_out, ispanset2_out, char_result);
  free(ispan_result); free(char_result);

  /* SpanSet *spanset_union_transfn(SpanSet *state, const SpanSet *ss); */
  ispanset_result = spanset_union_transfn(NULL, ispanset1);
  ispanset_result = spanset_union_transfn(ispanset_result, ispanset2);
  ispanset_result = spanset_union_finalfn(ispanset_result);
  char_result = intspanset_out(ispanset_result);
  printf("spanset_union aggregate: %s, %s -> %s\n", ispanset1_out, ispanset2_out, char_result);
  free(ispanset_result); free(char_result);

  /* Set *text_union_transfn(Set *state, const text *txt); */
  textset_result = text_union_transfn(NULL, text1);
  textset_result = text_union_transfn(textset_result, text2);
  char_result = textset_out(textset_result);
  printf("text_union aggregate: %s, %s -> %s\n", text1_out, text2_out, char_result);
  free(textset_result); free(char_result);

  /* Span *timestamptz_extent_transfn(Span *state, TimestampTz t); */
  tstzspan_result = timestamptz_extent_transfn(NULL, tstz1);
  tstzspan_result = timestamptz_extent_transfn(tstzspan_result, tstz2);
  char_result = tstzspan_out(tstzspan_result);
  printf("timestamptz_extent aggregate: %s, %s -> %s\n", tstz1_out, tstz2_out, char_result);
  free(tstzspan_result); free(char_result);

  /* Set *timestamptz_union_transfn(Set *state, TimestampTz t); */
  tstzset_result = timestamptz_union_transfn(NULL, tstz1);
  tstzset_result = timestamptz_union_transfn(tstzset_result, tstz2);
  char_result = tstzset_out(tstzset_result);
  printf("timestamptz_union aggregate: %s, %s -> %s\n", tstz1_out, tstz2_out, char_result);
  free(tstzset_result); free(char_result);

  /*****************************************************************************
   * Bin functions for span and spanset types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* int64 bigint_get_bin(int64 value, int64 vsize, int64 vorigin); */
  int64_result = bigint_get_bin(int64_in1, int64_in2, int64_in2);
  printf("bigint_get_bin(%ld, %ld, %ld): %d\n", int64_in1, int64_in2, int64_in2, int32_result);

  /* Span *bigintspan_bins(const Span *s, int64 vsize, int64 vorigin, int *count); */
  bspan_result = bigintspan_bins(bspan1, int64_in1, int64_in2, &count);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspan_bins(%s, %ld, %ld): %s\n", bspan1_out, int64_in1, int64_in2, char_result);
  free(bspan_result); free(char_result);

  /* Span *bigintspanset_bins(spanset1, int64 vsize, int64 vorigin, int *count); */
  bspan_result = bigintspanset_bins(bspanset1, int64_in1, int64_in2, &count);
  char_result = bigintspan_out(bspan_result);
  printf("bigintspanset_bins(%s, %ld, %ld): %s\n", bspanset1_out, int64_in1, int64_in2, char_result);
  free(bspan_result); free(char_result);

  /* DateADT date_get_bin(DateADT d, const Interval *duration, DateADT torigin); */
  date_result = date_get_bin(date1, interv1, date2);
  char_result = date_out(date_result);
  printf("date_get_bin(%s, %s, %s): %s\n", date1_out, interv1_out, date2_out, char_result);
  free(char_result);

  /* Span *datespan_bins(const Span *s, const Interval *duration, DateADT torigin, int *count); */
  dspanarray_result = datespan_bins(dspan1, interv1, date1, &count);
  printf("datespan_bins(%s, %s, %s, %d): {", dspan1_out, interv1_out, date1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = datespan_out(&(dspanarray_result)[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(dspanarray_result);

  /* Span *datespanset_bins(spanset1, const Interval *duration, DateADT torigin, int *count); */
  dspanarray_result = datespanset_bins(dspanset1, interv1, date1, &count);
  printf("datespanset_bins(%s, %s, %s, %d): {", dspanset1_out, interv1_out, date1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = datespan_out(&(dspanarray_result)[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(dspanarray_result);

  /* double float_get_bin(double value, double vsize, double vorigin); */
  float8_result = float_get_bin(float8_in1, float8_in1, float8_in2);
  printf("float_get_bin(%lf, %lf, %lf): %lf\n", float8_in1, float8_in1, float8_in2, float8_result);

  /* Span *floatspan_bins(const Span *s, double vsize, double vorigin, int *count); */
  fspan_result = floatspan_bins(fspan1, float8_in1, float8_in2, &count);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspan_bins(%s, %lf, %lf): %s\n", fspan1_out, float8_in1, float8_in2, char_result);
  free(fspan_result); free(char_result);

  /* Span *floatspanset_bins(spanset1, double vsize, double vorigin, int *count); */
  fspan_result = floatspanset_bins(fspanset1, float8_in1, float8_in2, &count);
  char_result = floatspan_out(fspan_result, 6);
  printf("floatspanset_bins(%s, %lf, %lf): %s\n", fspanset1_out, float8_in1, float8_in2, char_result);
  free(fspan_result); free(char_result);

  /* int int_get_bin(int value, int vsize, int vorigin); */
  int32_result = int_get_bin(int32_in1, int32_in2, int32_in2);
  printf("int_get_bin(%d, %d, %d): %d\n", int32_in1, int32_in2, int32_in2, int32_result);

  /* Span *intspan_bins(const Span *s, int vsize, int vorigin, int *count); */
  ispan_result = intspan_bins(ispan1, int32_in1, int32_in2, &count);
  char_result = intspan_out(ispan_result);
  printf("intspan_bins(%s, %d, %d): %s\n", ispan1_out, int32_in1, int32_in2, char_result);
  free(ispan_result); free(char_result);

  /* Span *intspanset_bins(spanset1, int vsize, int vorigin, int *count); */
  ispan_result = intspanset_bins(ispanset1, int32_in1, int32_in2, &count);
  char_result = intspan_out(ispan_result);
  printf("intspanset_bins(%s, %d, %d): %s\n", ispanset1_out, int32_in1, int32_in2, char_result);
  free(ispan_result); free(char_result);

  /* TimestampTz timestamptz_get_bin(TimestampTz t, const Interval *duration, TimestampTz torigin); */
  tstz_result = timestamptz_get_bin(tstz1, interv1, tstz2);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_get_bin(%s, %s, %s): %s\n", tstz1_out, interv1_out, tstz2_out, char_result);
  free(char_result);

  /* Span *tstzspan_bins(const Span *s, const Interval *duration, TimestampTz origin, int *count); */
  tstzspan_result = tstzspan_bins(tstzspan1, interv1, tstz2, &count);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspan_bins(%s, %s, %s): %s\n", tstzspan1_out, interv1_out, tstz2_out, char_result);
  free(tstzspan_result); free(char_result);

  /* Span *tstzspanset_bins(spanset1, const Interval *duration, TimestampTz torigin, int *count); */
  tstzspan_result = tstzspanset_bins(tstzspanset1, interv1, tstz2, &count);
  char_result = tstzspan_out(tstzspan_result);
  printf("tstzspanset_bins(%s, %s, %s): %s\n", tstzspanset1_out, interv1_out, tstz2_out, char_result);
  free(tstzspan_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */

  free(b1_out);
  free(text1); free(text1_out);
  free(text2); free(text2_out);
  free(text_null);
  free(tstz1_out); free(tstz2_out);
  free(date1_out); free(date2_out);
  free(interv1); free(interv1_out);
  free(interv2); free(interv2_out);
  free(iset1); free(iset1_out); free(iset1_hexwkb); free(iset1_wkb);
  free(ispan1); free(ispan1_out); free(ispan1_hexwkb); free(ispan1_wkb);
  free(ispanset1); free(ispanset1_out); free(ispanset1_hexwkb); free(ispanset1_wkb);
  free(iset2); free(iset2_out);
  free(ispan2); free(ispan2_out);
  free(ispanset2); free(ispanset2_out);
  free(bset1); free(bset1_out);
  free(bspan1); free(bspan1_out);
  free(bspanset1); free(bspanset1_out);
  free(bset2); free(bset2_out);
  free(bspan2); free(bspan2_out);
  free(bspanset2); free(bspanset2_out);
  free(fset1); free(fset1_out);
  free(fspan1); free(fspan1_out);
  free(fspanset1); free(fspanset1_out);
  free(fset2); free(fset2_out);
  free(fspan2); free(fspan2_out);
  free(fspanset2); free(fspanset2_out);
  free(textset1); free(textset1_out);
  free(dset1); free(dset1_out);
  free(dspan1); free(dspan1_out);
  free(dspanset1); free(dspanset1_out);
  free(dset2); free(dset2_out);
  free(dspan2); free(dspan2_out);
  free(dspanset2); free(dspanset2_out);
  free(tstzset1); free(tstzset1_out);
  free(tstzspan1); free(tstzspan1_out);
  free(tstzspanset1); free(tstzspanset1_out);
  free(tstzset2); free(tstzset2_out);
  free(tstzspan2); free(tstzspan2_out);
  free(tstzspanset2); free(tstzspanset2_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
