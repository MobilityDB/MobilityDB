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
 * gcc -Wall -g -I/usr/local/include -o temporal_test temporal_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_float.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /***************************************************************************
   * Create values to test the functions of the API
   ***************************************************************************/

  bool bool1 = bool_in("true");
  char *bool1_out = bool_out(bool1);

  int32 int32_in1 = 1;
  int32 int32_in2 = 3;

  double float8_in1 = 1;
  double float8_in2 = 3;

  size_t size, size_hexwkb, tbox_size_wkb, tfloat_size_wkb;

  char *text1_in = "abcdef";
  text *text1 = text_in(text1_in);
  char *text1_out = text_out(text1);

  text *text_null = text_in("NULL");

  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstz2_in = "2001-01-02";
  TimestampTz tstz2 = timestamptz_in(tstz2_in, -1);
  char *tstz2_out = timestamptz_out(tstz2);

  char *interv1_in = "3 hours";
  Interval *interv1 = interval_in(interv1_in, -1);
  char *interv1_out = interval_out(interv1);
  char *interv2_in = "3 mins";
  Interval *interv2 = interval_in(interv2_in, -1);
  char *interv2_out = interval_out(interv2);
  char *interv3_in = "3 days";
  Interval *interv3 = interval_in(interv3_in, -1);
  char *interv3_out = interval_out(interv3);

  char *fset1_in = "{1,3,5}";
  Set *fset1 = floatset_in(fset1_in);
  char *fset1_out = floatset_out(fset1, 6);
  char *fspan1_in = "[1,3]";
  Span *fspan1 = floatspan_in(fspan1_in);
  char *fspan1_out = floatspan_out(fspan1, 6);
  char *fspanset1_in = "{[1,3],[4,6]}";
  SpanSet *fspanset1 = floatspanset_in(fspanset1_in);
  char *fspanset1_out = floatspanset_out(fspanset1, 6);

  char *tstzset1_in = "{2001-01-01, 2001-01-03, 2001-01-05}";
  Set *tstzset1 = tstzset_in(tstzset1_in);
  char *tstzset1_out = tstzset_out(tstzset1);
  char *tstzset2_in = "{2001-01-02, 2001-01-03, 2001-01-04, 2001-01-06}";
  Set *tstzset2 = tstzset_in(tstzset2_in);
  char *tstzset2_out = tstzset_out(tstzset2);

  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);
  char *tstzspan2_in = "[2001-01-02, 2001-01-04]";
  Span *tstzspan2 = tstzspan_in(tstzspan2_in);
  char *tstzspan2_out = tstzspan_out(tstzspan2);

  char *tstzspanset1_in = "{[2001-01-01, 2001-01-03],[2001-01-04, 2001-01-06]}";
  SpanSet *tstzspanset1 = tstzspanset_in(tstzspanset1_in);
  char *tstzspanset1_out = tstzspanset_out(tstzspanset1);
  char *tstzspanset2_in = "{[2001-01-02, 2001-01-04],[2001-01-05, 2001-01-07]}";
  SpanSet *tstzspanset2 = tstzspanset_in(tstzspanset2_in);
  char *tstzspanset2_out = tstzspanset_out(tstzspanset2);

  char *tbox1_in = "TBOXFLOAT XT([1, 3],[2001-01-01, 2001-01-03])";
  TBox *tbox1 = tbox_in(tbox1_in);
  char *tbox1_out = tbox_out(tbox1, 6);
  char *tbox1_hexwkb = tbox_as_hexwkb(tbox1, 1, &size_hexwkb);
  uint8_t *tbox1_wkb = tbox_as_wkb(tbox1, 1, &tbox_size_wkb);
  char *tbox2_in = "TBOXFLOAT XT([2, 4],[2001-01-02, 2001-01-04])";
  TBox *tbox2 = tbox_in(tbox2_in);
  char *tbox2_out = tbox_out(tbox2, 6);

  TBox *tintbox1 = tbox_in("TBOXINT XT([1, 3],[2001-01-01, 2001-01-03])");
  char *tintbox1_out = tbox_out(tbox1, 6);
  TBox *tintbox2 = tbox_in("TBOXINT XT([2, 4],[2001-01-02, 2001-01-04])");
  char *tintbox2_out = tbox_out(tbox2, 6);

  char *tbool1_in = "{[t@2001-01-01, t@2001-01-03],[f@2001-01-04, f@2001-01-06]}";
  Temporal *tbool1 = tbool_in(tbool1_in);
  char *tbool1_out = tbool_out(tbool1);
  char *tbool2_in = "{[t@2001-01-02, t@2001-01-03],[f@2001-01-04, f@2001-01-06]}";
  Temporal *tbool2 = tbool_in(tbool2_in);
  char *tbool2_out = tbool_out(tbool2);

  char *tint1_in = "{[1@2001-01-01, 3@2001-01-03],[4@2001-01-04, 6@2001-01-06]}";
  Temporal *tint1 = tint_in(tint1_in);
  char *tint1_out = tint_out(tint1);
  char *tint1_mfjson = temporal_as_mfjson(tint1, true, 1, 6, NULL);
  char *tint2_in = "{[2@2001-01-01, 4@2001-01-04],[5@2001-01-05, 7@2001-01-07]}";
  Temporal *tint2 = tint_in(tint2_in);
  char *tint2_out = tint_out(tint2);

  char *tfloat1_in = "{[1@2001-01-01, 3@2001-01-03],[4@2001-01-04, 6@2001-01-06]}";
  Temporal *tfloat1 = tfloat_in(tfloat1_in);
  char *tfloat1_out = tfloat_out(tfloat1, 6);
  char *tfloat1_hexwkb = temporal_as_hexwkb(tfloat1, 1, &size_hexwkb);
  uint8_t *tfloat1_wkb = temporal_as_wkb(tfloat1, 1, &tfloat_size_wkb);
  char *tfloat1_mfjson = temporal_as_mfjson(tfloat1, true, 1, 6, NULL);
  char *tfloatinst1_in = "1@2001-01-01";
  TInstant *tfloatinst1 = (TInstant *) tfloat_in(tfloatinst1_in);
  char *tfloatinst1_out = tfloat_out((Temporal *) tfloatinst1, 6);
  char *tfloatseq1_in = "[1@2001-01-01, 3@2001-01-03]";
  TSequence *tfloatseq1 = (TSequence *) tfloat_in(tfloatseq1_in);
  char *tfloatseq1_out = tfloat_out((Temporal *) tfloatseq1, 6);

  char *tfloat2_in = "{[2@2001-01-02, 4@2001-01-03],[5@2001-01-04, 7@2001-01-06]}";
  Temporal *tfloat2 = tfloat_in(tfloat2_in);
  char *tfloat2_out = tfloat_out(tfloat2, 6);
  char *tfloatinst2_in = "2@2001-01-02";
  TInstant *tfloatinst2 = (TInstant *) tfloat_in(tfloatinst2_in);
  char *tfloatinst2_out = tfloat_out((Temporal *) tfloatinst2, 6);
  char *tfloatseq2_in = "[2@2001-01-04, 4@2001-01-06]";
  TSequence *tfloatseq2 = (TSequence *) tfloat_in(tfloatseq2_in);
  char *tfloatseq2_out = tfloat_out((Temporal *) tfloatseq2, 6);

  Temporal *tfloatarray[2];
  TInstant *tfloatinstarray[2];
  TSequence *tfloatseqarray[2];

  char *ttext1_in = "{[A@2001-01-01, B@2001-01-03],[C@2001-01-04, D@2001-01-06]}";
  Temporal *ttext1 = ttext_in(ttext1_in);
  char *ttext1_out = ttext_out(ttext1);
  char *ttext1_mfjson = temporal_as_mfjson(ttext1, true, 1, 6, NULL);
  char *ttext2_in = "{[E@2001-01-02, F@2001-01-04],[G@2001-01-05, H@2001-01-07]}";
  Temporal *ttext2 = ttext_in(ttext2_in);
  char *ttext2_out = ttext_out(ttext2);

  /* For aggregates */
  SkipList *sklist;

  /* For modification operators that update the input values */
  Temporal *tfloat_start;
  TInstant *tfloat_inst;
  TSequence *tfloat_seq;

  /***************************************************************************
   * Create output variables for the functions of the API
   ***************************************************************************/

  bool bool_result;
  bool bool1_result;
  int32 int32_result;
  uint32 uint32_result;
  double float8_result;
  char *char_result;
  char *char1_result;
  char *char2_result;
  text *text_result;
  TimestampTz tstz_result;
  Interval *interv_result;
  int count;

  Span *ispan_result;
  Span *fspan_result;
  SpanSet *fspanset_result;
  Span *tstzspan_result;
  SpanSet *tstzspanset_result;
  TBox *tbox_result;

  Temporal *tbool_result;
  Temporal *tint_result;
  Temporal *tfloat_result;
  TInstant *tfloatinst_result;
  TSequence *tfloatseq_result;
  TSequenceSet *tfloatseqset_result;
  Temporal *ttext_result;

  uint8_t *binchar_result;

  bool *boolarray_result;
  int32 *int32array_result;
  double *float8array_result;
  text **textarray_result;
  TimestampTz *tstzarray_result;

  Span *ispanarray_result;
  Span *fspanarray_result;
  Span *tstzspanarray_result;
  TBox *tboxarray_result;
  Temporal **tintarray_result;
  Temporal **tfloatarray_result;

  Match *matches;

  /***************************************************************************
   * Execute and print the result of the functions of the API
   ***************************************************************************/

  printf("****************************************************************\n");
  printf("* Temporal types *\n");
  printf("****************************************************************\n");

  /*****************************************************************************
   * Input and output functions for temporal boxes
   *****************************************************************************/
  printf("****************************************************************\n");

  /* char *tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size); */
  char_result = tbox_as_hexwkb(tbox1, 1, &size);
  printf("tbox_as_hexwkb(%s, 1, %ld): %s\n", tbox1_out, size, char_result);
  free(char_result);

  /* uint8_t *tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out); */
  binchar_result = tbox_as_wkb(tbox1, 1, &size);
  printf("tbox_as_wkb(%s, 1, %ld): ", tbox1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* TBox *tbox_from_hexwkb(const char *hexwkb); */
  tbox_result = tbox_from_hexwkb(tbox1_hexwkb);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_from_hexwkb(%s, %ld): %s\n", tbox1_hexwkb, size_hexwkb, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_from_wkb(const uint8_t *wkb, size_t size); */
  tbox_result = tbox_from_wkb(tbox1_wkb, tbox_size_wkb);
  char_result = tbox_out(tbox_result, 6);
  printf("temporal_from_wkb(");
  fwrite(tbox1_wkb, tbox_size_wkb, 1, stdout);
  printf(", %ld): %s\n", size, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_in(const char *str); */
  tbox_result = tbox_in(tbox1_in);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_in(%s): %s\n", tbox1_in, char_result);
  free(tbox_result); free(char_result);

  /* char *tbox_out(const TBox *box, int maxdd); */
  char_result = tbox_out(tbox1, 6);
  printf("tbox_out(%s, 6): %s\n", tbox1_out, char_result);
  free(char_result);

  /*****************************************************************************
   * Constructor functions for box types
   *****************************************************************************/

  /* TBox *float_timestamptz_to_tbox(double d, TimestampTz t); */
  tbox_result = float_timestamptz_to_tbox(float8_in1, tstz1);
  char_result = tbox_out(tbox_result, 6);
  printf("float_timestamptz_to_tbox(%lf, %s): %s\n", float8_in1, tstz1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *float_tstzspan_to_tbox(double d, const Span *s); */
  tbox_result = float_tstzspan_to_tbox(float8_in1, tstzspan1);
  char_result = tbox_out(tbox_result, 6);
  printf("float_tstzspan_to_tbox(%lf, %s): %s\n", float8_in1, tstzspan1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *int_timestamptz_to_tbox(int i, TimestampTz t); */
  tbox_result = int_timestamptz_to_tbox(int32_in1, tstz1);
  char_result = tbox_out(tbox_result, 6);
  printf("int_timestamptz_to_tbox(%d, %s): %s\n", int32_in1, tstz1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *int_tstzspan_to_tbox(int i, const Span *s); */
  tbox_result = int_tstzspan_to_tbox(int32_in1, tstzspan1);
  char_result = tbox_out(tbox_result, 6);
  printf("int_tstzspan_to_tbox(%d, %s): %s\n", int32_in1, tstzspan1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *numspan_tstzspan_to_tbox(const Span *span, const Span *s); */
  tbox_result = numspan_tstzspan_to_tbox(fspan1, tstzspan1);
  char_result = tbox_out(tbox_result, 6);
  printf("numspan_tstzspan_to_tbox(%s, %s): %s\n", fspan1_out, tstzspan1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *numspan_timestamptz_to_tbox(const Span *span, TimestampTz t); */
  tbox_result = numspan_timestamptz_to_tbox(fspan1, tstz1);
  char_result = tbox_out(tbox_result, 6);
  printf("numspan_timestamptz_to_tbox(%s, %s): %s\n", fspan1_out, tstz1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_copy(const TBox *box); */
  tbox_result = tbox_copy(tbox1);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_copy(%s): %s\n", tbox1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_make(const Span *s, const Span *p); */
  tbox_result = tbox_make(fspan1, tstzspan1);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_make(%s, %s): %s\n", fspan1_out, tstzspan1_out, char_result);
  free(tbox_result); free(char_result);

  /*****************************************************************************
   * Conversion functions for box types
   *****************************************************************************/

  /* TBox *float_to_tbox(double d); */
  tbox_result = float_to_tbox(float8_in1);
  char_result = tbox_out(tbox_result, 6);
  printf("float_to_tbox(%lf): %s\n", float8_in1, char_result);
  free(tbox_result); free(char_result);

  /* TBox *int_to_tbox(int i); */
  tbox_result = int_to_tbox(int32_in1);
  char_result = tbox_out(tbox_result, 6);
  printf("int_to_tbox(%d): %s\n", int32_in1, char_result);
  free(tbox_result); free(char_result);

  /* TBox *set_to_tbox(const Set *s); */
  tbox_result = set_to_tbox(fset1);
  char_result = tbox_out(tbox_result, 6);
  printf("set_to_tbox(%s): %s\n", fset1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *span_to_tbox(const Span *s); */
  tbox_result = span_to_tbox(fspan1);
  char_result = tbox_out(tbox_result, 6);
  printf("span_to_tbox(%s): %s\n", fspan1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *spanset_to_tbox(const SpanSet *ss); */
  tbox_result = spanset_to_tbox(fspanset1);
  char_result = tbox_out(tbox_result, 6);
  printf("spanset_to_tbox(%s): %s\n", fspanset1_out, char_result);
  free(tbox_result); free(char_result);

  /* Span *tbox_to_intspan(const TBox *box); */
  ispan_result = tbox_to_intspan(tbox1);
  char_result = intspan_out(ispan_result);
  printf("tbox_to_intspan(%s): %s\n", tbox1_out, char_result);
  free(ispan_result); free(char_result);

  /* Span *tbox_to_floatspan(const TBox *box); */
  fspan_result = tbox_to_floatspan(tbox1);
  char_result = floatspan_out(fspan_result, 6);
  printf("tbox_to_floatspan(%s): %s\n", tbox1_out, char_result);
  free(fspan_result); free(char_result);

  /* Span *tbox_to_tstzspan(const TBox *box); */
  tstzspan_result = tbox_to_tstzspan(tbox1);
  char_result = tstzspan_out(tstzspan_result);
  printf("tbox_to_tstzspan(%s): %s\n", tbox1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* TBox *timestamptz_to_tbox(TimestampTz t); */
  tbox_result = timestamptz_to_tbox(tstz1);
  char_result = tbox_out(tbox_result, 6);
  printf("timestamptz_to_tbox(%s): %s\n", tstz1_out, char_result);
  free(tbox_result); free(char_result);

  /*****************************************************************************
   * Accessor functions for box types
   *****************************************************************************/

  /* uint32 tbox_hash(const TBox *box); */
  uint32_result = tbox_hash(tbox1);
  printf("tbox_hash(%s): %ud\n", tbox1_out, uint32_result);

  /* bool tbox_hast(const TBox *box); */
  bool_result = tbox_hast(tbox1);
  printf("tbox_hast(%s): %c\n", tbox1_out, bool_result ? 't' : 'n');

  /* bool tbox_hasx(const TBox *box); */
  bool_result = tbox_hasx(tbox1);
  printf("tbox_hasx(%s): %c\n", tbox1_out, bool_result ? 't' : 'n');

  /* bool tbox_tmax(const TBox *box, TimestampTz *result); */
  bool_result = tbox_tmax(tbox1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("tbox_tmax(%s, %s): %c\n", tbox1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* bool tbox_tmax_inc(const TBox *box, bool *result); */
  bool_result = tbox_tmax_inc(tbox1, &bool1_result);
  printf("tbox_tmax_inc(%s, %c): %c\n", tbox1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool tbox_tmin(const TBox *box, TimestampTz *result); */
  bool_result = tbox_tmin(tbox1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("tbox_tmin(%s, %s): %c\n", tbox1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* bool tbox_tmin_inc(const TBox *box, bool *result); */
  bool_result = tbox_tmin_inc(tbox1, &bool1_result);
  printf("tbox_tmin_inc(%s, %c): %c\n", tbox1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool tbox_xmax(const TBox *box, double *result); */
  bool_result = tbox_xmax(tbox1, &float8_result);
  printf("tbox_xmax(%s, %lf): %c\n", tbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool tbox_xmax_inc(const TBox *box, bool *result); */
  bool_result = tbox_xmax_inc(tbox1, &bool1_result);
  printf("tbox_xmax_inc(%s, %c): %c\n", tbox1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool tbox_xmin(const TBox *box, double *result); */
  bool_result = tbox_xmin(tbox1, &float8_result);
  printf("tbox_xmin(%s, %lf): %c\n", tbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool tbox_xmin_inc(const TBox *box, bool *result); */
  bool_result = tbox_xmin_inc(tbox1, &bool1_result);
  printf("tbox_xmin_inc(%s, %c): %c\n", tbox1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool tboxfloat_xmax(const TBox *box, double *result); */
  bool_result = tboxfloat_xmax(tbox1, &float8_result);
  printf("tboxfloat_xmax(%s, %lf): %c\n", tbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool tboxfloat_xmin(const TBox *box, double *result); */
  bool_result = tboxfloat_xmin(tbox1, &float8_result);
  printf("tboxfloat_xmin(%s, %lf): %c\n", tbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool tboxint_xmax(const TBox *box, int *result); */
  bool_result = tboxint_xmax(tintbox1, &int32_result);
  printf("tboxint_xmax(%s, %d): %c\n", tintbox1_out, int32_result, bool_result ? 't' : 'n');

  /* bool tboxint_xmin(const TBox *box, int *result); */
  bool_result = tboxint_xmin(tintbox1, &int32_result);
  printf("tboxint_xmin(%s, %d): %c\n", tintbox1_out, int32_result, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Transformation functions for box types
   *****************************************************************************/

  /* TBox *tfloatbox_expand(const TBox *box, double d); */
  tbox_result = tfloatbox_expand(tbox1, float8_in1);
  char_result = tbox_out(tbox_result, 6);
  printf("tfloatbox_expand(%s, %lf): %s\n", tbox1_out, float8_in1, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tintbox_expand(const TBox *box, int i); */
  tbox_result = tintbox_expand(tintbox1, int32_in1);
  char_result = tbox_out(tbox_result, 6);
  printf("tintbox_expand(%s, %d): %s\n", tintbox1_out, int32_in1, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_expand_time(const TBox *box, const Interval *interv); */
  tbox_result = tbox_expand_time(tbox1, interv1);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_expand_time(%s, %s): %s\n", tbox1_out, interv1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_round(const TBox *box, int maxdd); */
  tbox_result = tbox_round(tbox1, 6);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_round(%s, 6): %s\n", tbox1_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tfloatbox_shift_scale(const TBox *box, double shift, double width, bool hasshift, bool haswidth); */
  tbox_result = tfloatbox_shift_scale(tbox1, float8_in1, float8_in2, true, true);
  char_result = tbox_out(tbox_result, 6);
  printf("tfloatbox_shift_scale(%s, %lf, %lf, true, true): %s\n", tbox1_out, float8_in1, float8_in2, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tintbox_shift_scale(const TBox *box, int shift, int width, bool hasshift, bool haswidth); */
  tbox_result = tintbox_shift_scale(tintbox1, int32_in1, int32_in2, true, true);
  char_result = tbox_out(tbox_result, 6);
  printf("tintbox_shift_scale(%s, %d, %d, true, true): %s\n", tintbox1_out, int32_in1, int32_in2, char_result);
  free(tbox_result); free(char_result);

  /* TBox *tbox_shift_scale_time(const TBox *box, const Interval *shift, const Interval *duration); */
  tbox_result = tbox_shift_scale_time(tbox1, interv1, interv2);
  char_result = tbox_out(tbox_result, 6);
  printf("tbox_shift_scale_time(%s, %s, %s): %s\n", tbox1_out, interv1_out, interv2_out, char_result);
  free(tbox_result); free(char_result);

  /*****************************************************************************
   * Set functions for box types
   *****************************************************************************/

  /* TBox *union_tbox_tbox(const TBox *box1, const TBox *box2, bool strict); */
  tbox_result = union_tbox_tbox(tbox1, tbox2, true);
  char_result = tbox_out(tbox_result, 6);
  printf("union_tbox_tbox(%s, %s, true): %s\n", tbox1_out, tbox2_out, char_result);
  free(tbox_result); free(char_result);

  /* TBox *intersection_tbox_tbox(const TBox *box1, const TBox *box2); */
  tbox_result = intersection_tbox_tbox(tbox1, tbox2);
  char_result = tbox_out(tbox_result, 6);
  printf("intersection_tbox_tbox(%s, %s): %s\n", tbox1_out, tbox2_out, char_result);
  free(tbox_result); free(char_result);

  /*****************************************************************************
   * Bounding box functions for box types
   *****************************************************************************/

  /* Topological functions for box types */

  /* bool adjacent_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = adjacent_tbox_tbox(tbox1, tbox2);
  printf("adjacent_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool contained_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = contained_tbox_tbox(tbox1, tbox2);
  printf("contained_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool contains_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = contains_tbox_tbox(tbox1, tbox2);
  printf("contains_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool overlaps_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = overlaps_tbox_tbox(tbox1, tbox2);
  printf("overlaps_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool same_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = same_tbox_tbox(tbox1, tbox2);
  printf("same_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /*****************************************************************************/

  /* Position functions for box types */

  /* bool after_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = after_tbox_tbox(tbox1, tbox2);
  printf("after_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool before_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = before_tbox_tbox(tbox1, tbox2);
  printf("before_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool left_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = left_tbox_tbox(tbox1, tbox2);
  printf("left_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool overafter_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = overafter_tbox_tbox(tbox1, tbox2);
  printf("overafter_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool overbefore_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = overbefore_tbox_tbox(tbox1, tbox2);
  printf("overbefore_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool overleft_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = overleft_tbox_tbox(tbox1, tbox2);
  printf("overleft_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool overright_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = overright_tbox_tbox(tbox1, tbox2);
  printf("overright_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool right_tbox_tbox(const TBox *box1, const TBox *box2); */
  bool_result = right_tbox_tbox(tbox1, tbox2);
  printf("right_tbox_tbox(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Comparison functions for box types
   *****************************************************************************/

  /* int tbox_cmp(const TBox *box1, const TBox *box2); */
  int32_result = tbox_cmp(tbox1, tbox2);
  printf("tbox_cmp(%s, %s): %d\n", tbox1_out, tbox2_out, int32_result);

  /* bool tbox_eq(const TBox *box1, const TBox *box2); */
  bool_result = tbox_eq(tbox1, tbox2);
  printf("tbox_eq(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool tbox_ge(const TBox *box1, const TBox *box2); */
  bool_result = tbox_ge(tbox1, tbox2);
  printf("tbox_ge(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool tbox_gt(const TBox *box1, const TBox *box2); */
  bool_result = tbox_gt(tbox1, tbox2);
  printf("tbox_gt(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool tbox_le(const TBox *box1, const TBox *box2); */
  bool_result = tbox_le(tbox1, tbox2);
  printf("tbox_le(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool tbox_lt(const TBox *box1, const TBox *box2); */
  bool_result = tbox_lt(tbox1, tbox2);
  printf("tbox_lt(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /* bool tbox_ne(const TBox *box1, const TBox *box2); */
  bool_result = tbox_ne(tbox1, tbox2);
  printf("tbox_ne(%s, %s): %c\n", tbox1_out, tbox2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Input and output functions for temporal types
   *****************************************************************************/

  /* Temporal *tbool_from_mfjson(const char *str); */
  char *mfjson = temporal_as_mfjson(tbool1, true, 1, 6, NULL);
  tbool_result = tbool_from_mfjson(mfjson);
  char_result = tbool_out(tbool_result);
  printf("tbool_from_mfjson(%s): %s\n", tbool1_in, char_result);
  free(mfjson); free(tbool_result); free(char_result);

  /* Temporal *tbool_in(const char *str); */
  tbool_result = tbool_in(tbool1_in);
  char_result = tbool_out(tbool_result);
  printf("tbool_in(%s): %s\n", tbool1_in, char_result);
  free(tbool_result); free(char_result);

  /* char *tbool_out(const Temporal *temp); */
  char_result = tbool_out(tbool1);
  printf("tbool_out(%s): %s\n", tbool1_out, char_result);
  free(char_result);

  /* char *temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out); */
  char_result = temporal_as_hexwkb(tfloat1, 1, &size);
  printf("temporal_as_hexwkb(%s, 1, &size): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* char *temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags, int precision, const char *srs); */
  char_result = temporal_as_mfjson(tfloat1, true, 1, 1, "");
  printf("temporal_as_mfjson(%s, true, 1, 1, \"\"): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* uint8_t *temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out); */
  binchar_result = temporal_as_wkb(tfloat1, 1, &size);
  printf("tbox_as_wkb(%s, 1, %ld): ", tfloat1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* Temporal *temporal_from_hexwkb(const char *hexwkb); */
  tfloat_result = temporal_from_hexwkb(tfloat1_hexwkb);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_from_hexwkb(%s): %s\n", tfloat1_hexwkb, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_from_wkb(const uint8_t *wkb, size_t size); */
  tfloat_result = temporal_from_wkb(tfloat1_wkb, tfloat_size_wkb);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_from_wkb(");
  fwrite(tfloat1_wkb, tfloat_size_wkb, 1, stdout);
  printf(", %ld): %s\n", size, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_from_mfjson(const char *str); */
  tfloat_result = tfloat_from_mfjson(tfloat1_mfjson);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_from_mfjson(%s): %s\n", tfloat1_mfjson, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_in(const char *str); */
  tfloat_result = tfloat_in(tfloat1_in);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_in(%s): %s\n", tfloat1_in, char_result);
  free(tfloat_result); free(char_result);

  /* char *tfloat_out(const Temporal *temp, int maxdd); */
  char_result = tfloat_out(tfloat1, 6);
  printf("tfloat_out(%s, 6): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* Temporal *tint_from_mfjson(const char *str); */
  tint_result = tint_from_mfjson(tint1_mfjson);
  char_result = tint_out(tint_result);
  printf("tint_from_mfjson(%s): %s\n", tfloat1_mfjson, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tint_in(const char *str); */
  tint_result = tint_in(tint1_in);
  char_result = tint_out(tint_result);
  printf("tint_in(%s): %s\n", tint1_in, char_result);
  free(tint_result); free(char_result);

  /* char *tint_out(const Temporal *temp); */
  char_result = tint_out(tint1);
  printf("tint_out(%s): %s\n", tint1_out, char_result);
  free(char_result);

  /* Temporal *ttext_from_mfjson(const char *str); */
  ttext_result = ttext_from_mfjson(ttext1_mfjson);
  char_result = ttext_out(ttext_result);
  printf("ttext_from_mfjson(%s): %s\n", ttext1_mfjson, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *ttext_in(const char *str); */
  ttext_result = ttext_in(ttext1_in);
  char_result = ttext_out(ttext_result);
  printf("ttext_in(%s): %s\n", ttext1_in, char_result);
  free(ttext_result); free(char_result);

  /* char *ttext_out(const Temporal *temp); */
  char_result = ttext_out(ttext1);
  printf("ttext_out(%s): %s\n", ttext1_out, char_result);
  free(char_result);

  /*****************************************************************************
   * Constructor functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tbool_from_base_temp(bool b, const Temporal *temp); */
  tbool_result = tbool_from_base_temp(bool1, tbool1);
  char_result = tbool_out(tbool_result);
  printf("tbool_from_base_temp(%s, %s): %s\n", bool1_out, tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* TInstant *tboolinst_make(bool b, TimestampTz t); */
  tbool_result = (Temporal *) tboolinst_make(bool1, tstz1);
  char_result = tbool_out(tbool_result);
  printf("tboolinst_make(%s, %s): %s\n", bool1_out, tstz1_out, char_result);
  free(tbool_result); free(char_result);

  /* TSequence *tboolseq_from_base_tstzset(bool b, const Set *s); */
  tbool_result = (Temporal *) tboolseq_from_base_tstzset(bool1, tstzset1);
  char_result = tbool_out(tbool_result);
  printf("tboolseq_from_base_tstzset(%s, %s): %s\n", bool1_out, tstz1_out, char_result);
  free(tbool_result); free(char_result);

  /* TSequence *tboolseq_from_base_tstzspan(bool b, const Span *s); */
  tbool_result = (Temporal *) tboolseq_from_base_tstzspan(bool1, tstzspan1);
  char_result = tbool_out(tbool_result);
  printf("tboolseq_from_base_tstzspan(%s, %s): %s\n", bool1_out, tstzspan1_out, char_result);
  free(tbool_result); free(char_result);

  /* TSequenceSet *tboolseqset_from_base_tstzspanset(bool b, const SpanSet *ss); */
  tbool_result = (Temporal *) tboolseqset_from_base_tstzspanset(bool1, tstzspanset1);
  char_result = tbool_out(tbool_result);
  printf("tboolseqset_from_base_tstzspanset(%s, %s): %s\n", bool1_out, tstzspanset1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *temporal_copy(const Temporal *temp); */
  tfloat_result = temporal_copy(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_copy(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_from_base_temp(double d, const Temporal *temp); */
  tfloat_result = tfloat_from_base_temp(float8_in1, tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_from_base_temp(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TInstant *tfloatinst_make(double d, TimestampTz t); */
  tfloat_result = (Temporal *) tfloatinst_make(float8_in1, tstz1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloatinst_make(%lf, %s): %s\n", float8_in1, tstz1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequence *tfloatseq_from_base_tstzset(double d, const Set *s); */
  tfloat_result = (Temporal *) tfloatseq_from_base_tstzset(float8_in1, tstzset1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloatseq_from_base_tstzset(%lf, %s): %s\n", float8_in1, tstz1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequence *tfloatseq_from_base_tstzspan(double d, const Span *s, interpType interp); */
  tfloat_result = (Temporal *) tfloatseq_from_base_tstzspan(float8_in1, tstzspan1, LINEAR);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloatseq_from_base_tstzspan(%lf, %s, LINEAR): %s\n", float8_in1, tstzspan1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequenceSet *tfloatseqset_from_base_tstzspanset(double d, const SpanSet *ss, interpType interp); */
  tfloat_result = (Temporal *) tfloatseqset_from_base_tstzspanset(float8_in1, tstzspanset1, LINEAR);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloatseqset_from_base_tstzspanset(%lf, %s, LINEAR): %s\n", float8_in1, tstzspanset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tint_from_base_temp(int i, const Temporal *temp); */
  tint_result = tint_from_base_temp(int32_in1, tint1);
  char_result = tint_out(tint_result);
  printf("tint_from_base_temp(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tint_result); free(char_result);

  /* TInstant *tintinst_make(int i, TimestampTz t); */
  tint_result = (Temporal *) tintinst_make(int32_in1, tstz1);
  char_result = tint_out(tint_result);
  printf("tintinst_make(%d, %s): %s\n", int32_in1, tstz1_out, char_result);
  free(tint_result); free(char_result);

  /* TSequence *tintseq_from_base_tstzset(int i, const Set *s); */
  tint_result = (Temporal *) tintseq_from_base_tstzset(int32_in1, tstzset1);
  char_result = tint_out(tint_result);
  printf("tintinst_make(%d, %s): %s\n", int32_in1, tstzset1_out, char_result);
  free(tint_result); free(char_result);

  /* TSequence *tintseq_from_base_tstzspan(int i, const Span *s); */
  tint_result = (Temporal *) tintseq_from_base_tstzspan(int32_in1, tstzspan1);
  char_result = tint_out(tint_result);
  printf("tintseq_from_base_tstzspan(%d, %s): %s\n", int32_in1, tstzspan1_out, char_result);
  free(tint_result); free(char_result);

  /* TSequenceSet *tintseqset_from_base_tstzspanset(int i, const SpanSet *ss); */
  tint_result = (Temporal *) tintseqset_from_base_tstzspanset(int32_in1, tstzspanset1);
  char_result = tint_out(tint_result);
  printf("tintseqset_from_base_tstzspanset(%d, %s): %s\n", int32_in1, tstzspanset1_out, char_result);
  free(tint_result); free(char_result);

  /* TSequence *tsequence_make(TInstant **instants, int count, bool lower_inc, bool upper_inc, interpType interp, bool normalize); */
  tfloatinstarray[0] = tfloatinst1;
  tfloatinstarray[1] = tfloatinst2;
  tfloat_result = (Temporal *) tsequence_make(tfloatinstarray, 1, true, true, LINEAR, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tsequence_make({%s, %s}, 1, true, true, LINEAR, true): %s\n", tfloatseq1_out, tfloatseq2_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequenceSet *tsequenceset_make(TSequence **sequences, int count, bool normalize); */
  tfloatseqarray[0] = tfloatseq1;
  tfloatseqarray[1] = tfloatseq2;
  tfloat_result = (Temporal *) tsequenceset_make(tfloatseqarray, 2, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tsequenceset_make({%s, %s}, 2, true): %s\n", tfloatseq1_out, tfloatseq2_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequenceSet *tsequenceset_make_gaps(TInstant **instants, int count, interpType interp, const Interval *maxt, double maxdist); */
  tfloatinstarray[0] = tfloatinst1;
  tfloatinstarray[1] = tfloatinst2;
  tfloat_result =  (Temporal *) tsequenceset_make_gaps(tfloatinstarray, 2, LINEAR, interv1, float8_in2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tsequenceset_make_gaps({%s, %s}, 2, LINEAR, %s, %lf): %s\n", tfloatinst1_out, tfloatinst2_out, interv1_out, float8_in2, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *ttext_from_base_temp(const text *txt, const Temporal *temp); */
  ttext_result = ttext_from_base_temp(text1, ttext1);
  char_result = ttext_out(ttext_result);
  printf("ttext_from_base_temp(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(ttext_result); free(char_result);

  /* TInstant *ttextinst_make(const text *txt, TimestampTz t); */
  ttext_result = (Temporal *) ttextinst_make(text1, tstz1);
  char_result = ttext_out(ttext_result);
  printf("ttextinst_make(%s, %s): %s\n", text1_out, tstz1_out, char_result);
  free(ttext_result); free(char_result);

  /* TSequence *ttextseq_from_base_tstzset(const text *txt, const Set *s); */
  ttext_result = (Temporal *) ttextseq_from_base_tstzset(text1, tstzset1);
  char_result = ttext_out(ttext_result);
  printf("ttextinst_make(%s, %s): %s\n", text1_out, tstz1_out, char_result);
  free(ttext_result); free(char_result);

  /* TSequence *ttextseq_from_base_tstzspan(const text *txt, const Span *s); */
  ttext_result = (Temporal *) ttextseq_from_base_tstzspan(text1, tstzspan1);
  char_result = ttext_out(ttext_result);
  printf("ttextseq_from_base_tstzspan(%s, %s): %s\n", text1_out, tstzspan1_out, char_result);
  free(ttext_result); free(char_result);

  /* TSequenceSet *ttextseqset_from_base_tstzspanset(const text *txt, const SpanSet *ss); */
  ttext_result = (Temporal *) ttextseqset_from_base_tstzspanset(text1, tstzspanset1);
  char_result = ttext_out(ttext_result);
  printf("ttextseqset_from_base_tstzspanset(%s, %s): %s\n", text1_out, tstzspanset1_out, char_result);
  free(ttext_result); free(char_result);

  /*****************************************************************************
   * Conversion functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tbool_to_tint(const Temporal *temp); */
  tint_result = tbool_to_tint(tbool1);
  char_result = tint_out(tint_result);
  printf("tbool_to_tint(%s): %s\n", tbool1_out, char_result);
  free(tint_result); free(char_result);

  /* Span *temporal_to_tstzspan(const Temporal *temp); */
  tstzspan_result = temporal_to_tstzspan(tfloat1);
  char_result = tstzspan_out(tstzspan_result);
  printf("temporal_to_tstzspan(%s): %s\n", tfloat1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* Temporal *tfloat_to_tint(const Temporal *temp); */
  Temporal *tfloat_step = tfloat_in("interp=Step;[1@2001-01-01, 2@2001-01-02]");
  tint_result = tfloat_to_tint(tfloat_step);
  char_result = tint_out(tint_result);
  printf("tfloat_to_tint(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_step); free(tint_result); free(char_result);

  /* Temporal *tint_to_tfloat(const Temporal *temp); */
  tfloat_result = tint_to_tfloat(tint1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tint_to_tfloat(%s): %s\n", tint1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Span *tnumber_to_span(const Temporal *temp); */
  fspan_result = tnumber_to_span(tfloat1);
  char_result = floatspan_out(fspan_result, 6);
  printf("tnumber_to_span(%s): %s\n", tfloat1_out, char_result);
  free(fspan_result); free(char_result);

  /* TBox *tnumber_to_tbox (const Temporal *temp); */
  tbox_result = tnumber_to_tbox (tfloat1);
  char_result = tbox_out(tbox_result, 6);
  printf("tnumber_to_tbox(%s): %s\n", tfloat1_out, char_result);
  free(tbox_result); free(char_result);

  /*****************************************************************************
   * Accessor functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* bool tbool_end_value(const Temporal *temp); */
  bool_result = tbool_end_value(tbool1);
  printf("tbool_end_value(%s): %c\n", tbool1_out, bool_result ? 't' : 'n');

  /* bool tbool_start_value(const Temporal *temp); */
  bool_result = tbool_start_value(tbool1);
  printf("tbool_start_value(%s): %c\n", tbool1_out, bool_result ? 't' : 'n');

  /* bool tbool_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, bool *value); */
  bool_result = tbool_value_at_timestamptz(tbool1, tstz1, true, &bool1_result);
  printf("tbool_value_at_timestamptz(%s, %s, true, %c): %c\n", tbool1_out, tstz1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool tbool_value_n(const Temporal *temp, int n, bool *result); */
  bool_result = tbool_value_n(tbool1, 1, &bool_result);
  printf("tbool_value_n(%s, 1, %c): %c\n", tbool1_out, bool1_result ? 't' : 'n', bool_result ? 't' : 'n');

  /* bool *tbool_values(const Temporal *temp, int *count); */
  boolarray_result = tbool_values(tbool1, &count);
  printf("tbool_values(%s): {", tbool1_out);
  for (int i = 0; i < count; i++)
  {
    printf("%c", boolarray_result[i] ? 't' : 'n');
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(boolarray_result);

  /* Interval *temporal_duration(const Temporal *temp, bool boundspan); */
  interv_result = temporal_duration(tfloat1, true);
  char_result = interval_out(interv_result);
  printf("tnumber_to_span(%s, true): %s\n", tfloat1_out, char_result);
  free(interv_result); free(char_result);

  /* TInstant *temporal_end_instant(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_end_instant(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_end_instant(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequence *temporal_end_sequence(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_end_sequence(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_end_sequence(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TimestampTz temporal_end_timestamptz(const Temporal *temp); */
  tstz_result = temporal_end_timestamptz(tfloat1);
  char_result = timestamptz_out(tstz_result);
  printf("temporal_end_timestamptz(%s): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* uint32 temporal_hash(const Temporal *temp); */
  uint32_result = temporal_hash(tfloat1);
  printf("temporal_hash(%s): %ud\n", tfloat1_out, uint32_result);

  /* TInstant *temporal_instant_n(const Temporal *temp, int n); */
  tfloat_result = (Temporal *) temporal_instant_n(tfloat1, 1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_instant_n(%s, 1): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TInstant **temporal_instants(const Temporal *temp, int *count); */
  tfloatarray_result = (Temporal **) temporal_instants(tfloat1, &count);
  printf("temporal_instants(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
  }
  free(tfloatarray_result);

  /* const char *temporal_interp(const Temporal *temp); */
  char_result = strdup(temporal_interp(tfloat1));
  printf("temporal_interp(%s): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* bool temporal_lower_inc(const Temporal *temp); */
  bool_result = temporal_lower_inc(tfloat1);
  printf("temporal_lower_inc(%s): %c\n", tfloat1_out, bool_result ? 't' : 'n');

  /* TInstant *temporal_max_instant(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_max_instant(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_max_instant(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TInstant *temporal_min_instant(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_min_instant(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_min_instant(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* int temporal_num_instants(const Temporal *temp); */
  int32_result = temporal_num_instants(tfloat1);
  printf("temporal_num_instants(%s): %d\n", tfloat1_out, int32_result);

  /* int temporal_num_sequences(const Temporal *temp); */
  int32_result = temporal_num_sequences(tfloat1);
  printf("temporal_num_instants(%s): %d\n", tfloat1_out, int32_result);

  /* int temporal_num_timestamps(const Temporal *temp); */
  int32_result = temporal_num_timestamps(tfloat1);
  printf("temporal_num_timestamps(%s): %d\n", tfloat1_out, int32_result);

  /* TSequence **temporal_segments(const Temporal *temp, int *count); */
  tfloatarray_result = (Temporal **) temporal_segments(tfloat1, &count);
  printf("temporal_segments(%s, %d): {", tfloat1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
  }
  free(tfloatarray_result);

  /* TSequence *temporal_sequence_n(const Temporal *temp, int i); */
  tfloat_result = (Temporal *) temporal_sequence_n(tfloat1, 1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_sequence_n(%s, 1): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequence **temporal_sequences(const Temporal *temp, int *count); */
  tfloatarray_result = (Temporal **) temporal_sequences(tfloat1, &count);
  printf("temporal_sequences(%s, %d): {", tfloat1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
  }
  free(tfloatarray_result);

  /* TInstant *temporal_start_instant(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_start_instant(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_start_instant(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TSequence *temporal_start_sequence(const Temporal *temp); */
  tfloat_result = (Temporal *) temporal_start_sequence(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_start_sequence(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TimestampTz temporal_start_timestamptz(const Temporal *temp); */
  tstz_result = temporal_start_timestamptz(tfloat1);
  char_result = timestamptz_out(tstz_result);
  printf("temporal_start_timestamptz(%s): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* TSequenceSet *temporal_stops(const Temporal *temp, double maxdist, const Interval *minduration); */
  tfloat_result = (Temporal *) temporal_stops(tfloat1, float8_in1, interv1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_stops(%s, %lf, %s): %s\n", tfloat1_out, float8_in1, interv1_out, char_result);
  free(tfloat_result); free(char_result);

  /* const char *temporal_subtype(const Temporal *temp); */
  char_result = strdup(temporal_subtype(tfloat1));
  printf("temporal_subtype(%s): %s\n", tfloat1_out, char_result);
  free(char_result);

  /* SpanSet *temporal_time(const Temporal *temp); */
  tstzspanset_result = temporal_time(tfloat1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("temporal_time(%s): %s\n", tfloat1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* TimestampTz *temporal_timestamps(const Temporal *temp, int *count); */
  tstzarray_result = temporal_timestamps(tfloat1, &count);
  printf("temporal_timestamps(%s, %d): {", tstzset1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = timestamptz_out(tstzarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzarray_result);

  /* bool temporal_timestamptz_n(const Temporal *temp, int n, TimestampTz *result); */
  bool_result = temporal_timestamptz_n(tfloat1, 1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("temporal_timestamptz_n(%s, 1, %s): %c\n", tfloat1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* bool temporal_upper_inc(const Temporal *temp); */
  bool_result = temporal_upper_inc(tfloat1);
  printf("temporal_upper_inc(%s): %c\n", tfloat1_out, bool_result ? 't' : 'n');

  /* double tfloat_end_value(const Temporal *temp); */
  float8_result = tfloat_end_value(tfloat1);
  printf("tfloat_end_value(%s): %lf\n", tfloat1_out, float8_result);

  /* double tfloat_min_value(const Temporal *temp); */
  float8_result = tfloat_min_value(tfloat1);
  printf("tfloat_min_value(%s): %lf\n", tfloat1_out, float8_result);

  /* double tfloat_max_value(const Temporal *temp); */
  float8_result = tfloat_max_value(tfloat1);
  printf("tfloat_max_value(%s): %lf\n", tfloat1_out, float8_result);

  /* double tfloat_start_value(const Temporal *temp); */
  float8_result = tfloat_start_value(tfloat1);
  printf("tfloat_start_value(%s): %lf\n", tfloat1_out, float8_result);

  /* bool tfloat_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, double *value); */
  bool_result = tfloat_value_at_timestamptz(tfloat1, tstz1, true, &float8_result);
  printf("tfloat_value_at_timestamptz(%s, %s, true, %lf): %c\n", tfloat1_out, tstz1_out, float8_result, bool_result ? 't' : 'n');

  /* bool tfloat_value_n(const Temporal *temp, int n, double *result); */
  bool_result = tfloat_value_n(tfloat1, 1, &float8_result);
  printf("tfloat_value_n(%s, %lf): %c\n", tfloat1_out, float8_result, bool_result ? 't' : 'n');

  /* double *tfloat_values(const Temporal *temp, int *count); */
  float8array_result = tfloat_values(tfloat1, &count);
  printf("tfloat_values(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    printf("%lf", float8array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(float8array_result);

  /* int tint_end_value(const Temporal *temp); */
  int32_result = tint_end_value(tint1);
  printf("tint_end_value(%s): %d\n", tint1_out, int32_result);

  /* int tint_max_value(const Temporal *temp); */
  int32_result = tint_max_value(tint1);
  printf("tint_max_value(%s): %d\n", tint1_out, int32_result);

  /* int tint_min_value(const Temporal *temp); */
  int32_result = tint_min_value(tint1);
  printf("tint_min_value(%s): %d\n", tint1_out, int32_result);

  /* int tint_start_value(const Temporal *temp); */
  int32_result = tint_start_value(tint1);
  printf("tint_start_value(%s): %d\n", tint1_out, int32_result);

  /* bool tint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, int *value); */
  bool_result = tint_value_at_timestamptz(tint1, tstz1, true, &int32_result);
  printf("tint_value_at_timestamptz(%s, %s, true, &int32_result): %c\n", tint1_out, tstz1_out, bool_result ? 't' : 'n');

  /* bool tint_value_n(const Temporal *temp, int n, int *result); */
  bool_result = tint_value_n(tint1, 1, &int32_result);
  printf("tint_value_n(%s, 1, %d): %c\n", tint1_out, int32_result, bool_result ? 't' : 'n');

  /* int *tint_values(const Temporal *temp, int *count); */
  int32array_result = tint_values(tint1, &count);
  printf("tint_values(%s): {", tint1_out);
  for (int i = 0; i < count; i++)
  {
    printf("%d", int32array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(int32array_result);

  /* double tnumber_avg_value(const Temporal *temp); */
  float8_result = tnumber_avg_value(tfloat1);
  printf("tnumber_avg_value(%s): %lf\n", tfloat1_out, float8_result);

  /* double tnumber_integral(const Temporal *temp); */
  float8_result = tnumber_integral(tfloat1);
  printf("tnumber_integral(%s): %lf\n", tfloat1_out, float8_result);

  /* double tnumber_twavg(const Temporal *temp); */
  float8_result = tnumber_twavg(tfloat1);
  printf("tnumber_twavg(%s): %lf\n", tfloat1_out, float8_result);

  /* SpanSet *tnumber_valuespans(const Temporal *temp); */
  fspanset_result = tnumber_valuespans(tfloat1);
  char_result = floatspanset_out(fspanset_result, 6);
  printf("tnumber_valuespans(%s): %s\n", tfloat1_out, char_result);
  free(fspanset_result); free(char_result);

  /* text *ttext_end_value(const Temporal *temp); */
  text_result = ttext_end_value(ttext1);
  char_result = text_out(text_result);
  printf("ttext_end_value(%s): %s\n", ttext1_out, char_result);
  free(text_result); free(char_result);

  /* text *ttext_max_value(const Temporal *temp); */
  text_result = ttext_max_value(ttext1);
  char_result = text_out(text_result);
  printf("ttext_max_value(%s): %s\n", ttext1_out, char_result);
  free(text_result); free(char_result);

  /* text *ttext_min_value(const Temporal *temp); */
  text_result = ttext_min_value(ttext1);
  char_result = text_out(text_result);
  printf("ttext_min_value(%s): %s\n", ttext1_out, char_result);
  free(text_result); free(char_result);

  /* text *ttext_start_value(const Temporal *temp); */
  text_result = ttext_start_value(ttext1);
  char_result = text_out(text_result);
  printf("ttext_start_value(%s): %s\n", ttext1_out, char_result);
  free(text_result); free(char_result);

  /* bool ttext_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, text **value); */
  bool_result = ttext_value_at_timestamptz(ttext1, tstz1, true, &text_result);
  char_result = text_out(text_result);
  printf("ttext_value_at_timestamptz(%s, %s, true, %s): %c\n", ttext1_out, tstz1_out, char_result, bool_result ? 't' : 'n');
  free(text_result); free(char_result);

  /* bool ttext_value_n(const Temporal *temp, int n, text **result); */
  bool_result = ttext_value_n(ttext1, 1, &text_result);
  char_result = text_out(text_result);
  printf("ttext_value_n(%s, 1, %s): %c\n", ttext1_out, char_result, bool_result ? 't' : 'n');
  free(text_result); free(char_result);

  /* text **ttext_values(const Temporal *temp, int *count); */
  textarray_result = ttext_values(ttext1, &count);
  printf("ttext_values(%s): {", tint1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(textarray_result[i]);
    free(char_result);
  }
  free(textarray_result);

  /*****************************************************************************
   * Transformation functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* double float_degrees(double value, bool normalize); */
  float8_result = float_degrees(float8_in1, true);
  printf("float_degrees(%lf, true): %lf\n", float8_in1, float8_result);

  /* Temporal **temparr_round(Temporal **temp, int count, int maxdd); */
  tfloatarray[0] = tfloat1;
  tfloatarray[1] = tfloat2;
  tfloatarray_result = temparr_round(tfloatarray, 2, 6);
  printf("temparr_round({%s, %s}, 2, 6): {", tfloatinst1_out, tfloatinst2_out);
  for (int i = 0; i < 2; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    printf("%s", char_result);
    if (i < 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
  }
  free(tfloatarray_result);

  /* Temporal *temporal_round(const Temporal *temp, int maxdd); */
  tfloat_result = temporal_round(tfloat1, 6);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_round(%s, 6): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_scale_time(const Temporal *temp, const Interval *duration); */
  tfloat_result = temporal_scale_time(tfloat1, interv1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_scale_time(%s, %s): %s\n", tfloat1_out, interv1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_set_interp(const Temporal *temp, interpType interp); */
  tfloat_result = temporal_set_interp(tfloat1, LINEAR);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_set_interp(%s, LINEAR): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_shift_scale_time(const Temporal *temp, const Interval *shift, const Interval *duration); */
  tfloat_result = temporal_shift_scale_time(tfloat1, interv1, interv2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_shift_scale_time(%s, %s, %s): %s\n", tfloat1_out, interv1_out, interv2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_shift_time(const Temporal *temp, const Interval *shift); */
  tfloat_result = temporal_shift_time(tfloat1, interv1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_shift_time(%s, %s): %s\n", tfloat1_out, interv1_out, char_result);
  free(tfloat_result); free(char_result);

  /* TInstant *temporal_to_tinstant(const Temporal *temp); */
  tfloatinst_result = temporal_to_tinstant((Temporal *) tfloatinst1);
  char_result = tfloat_out((Temporal *) tfloatinst_result, 6);
  printf("temporal_to_tinstant(%s): %s\n", tfloat1_out, char_result);
  free(tfloatinst_result); free(char_result);

  /* TSequence *temporal_to_tsequence(const Temporal *temp, interpType interp); */
  tfloatseq_result = temporal_to_tsequence((Temporal *) tfloatinst1, LINEAR);
  char_result = tfloat_out((Temporal *) tfloatseq_result, 6);
  printf("temporal_to_tsequence(%s, LINEAR): %s\n", tfloat1_out, char_result);
  free(tfloatseq_result); free(char_result);

  /* TSequenceSet *temporal_to_tsequenceset(const Temporal *temp, interpType interp); */
  tfloatseqset_result = temporal_to_tsequenceset(tfloat1, LINEAR);
  char_result = tfloat_out((Temporal *) tfloatseqset_result, 6);
  printf("temporal_to_tsequenceset(%s, LINEAR): %s\n", tfloat1_out, char_result);
  free(tfloatseqset_result); free(char_result);

  /* Temporal *tfloat_ceil(const Temporal *temp); */
  tfloat_result = tfloat_ceil(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_ceil(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_degrees(const Temporal *temp, bool normalize); */
  tfloat_result = tfloat_degrees(tfloat1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_degrees(%s, true): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_floor(const Temporal *temp); */
  tfloat_result = tfloat_floor(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_floor(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_radians(const Temporal *temp); */
  tfloat_result = tfloat_radians(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_radians(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_scale_value(const Temporal *temp, double width); */
  tfloat_result = tfloat_scale_value(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_scale_value(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_shift_scale_value(const Temporal *temp, double shift, double width); */
  tfloat_result = tfloat_shift_scale_value(tfloat1, float8_in1, float8_in2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_shift_scale_value(%s, %lf, %lf): %s\n", tfloat1_out, float8_in1, float8_in2, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_shift_value(const Temporal *temp, double shift); */
  tfloat_result = tfloat_shift_value(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_shift_value(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tint_scale_value(const Temporal *temp, int width); */
  tint_result = tint_scale_value(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("tint_scale_value(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tint_shift_scale_value(const Temporal *temp, int shift, int width); */
  tint_result = tint_shift_scale_value(tint1, int32_in1, int32_in2);
  char_result = tint_out(tint_result);
  printf("tint_shift_scale_value(%s, %d, %d): %s\n", tint1_out, int32_in1, int32_in2, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tint_shift_value(const Temporal *temp, int shift); */
  tint_result = tint_shift_value(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("tint_shift_value(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /*****************************************************************************
   * Modification functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *temporal_append_tinstant(Temporal *temp, const TInstant *inst, interpType interp, double maxdist, const Interval *maxt, bool expand); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_inst = (TInstant *) tfloat_in("4@20001-01-04");
  tfloat_result = temporal_append_tinstant(tfloat_start, tfloat_inst, LINEAR, float8_in1, interv1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_append_tinstant(%s, %s): %s\n", tfloat1_out, tfloatinst1_out, char_result);
  free(tfloat_start); free(tfloat_inst); free(tfloat_result); free(char_result);

  /* Temporal *temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_seq = (TSequence *) tfloat_in("[4@20001-01-04, 5@20001-01-05]");
  tfloat_result = temporal_append_tsequence(tfloat_start, tfloat_seq, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_append_tsequence(%s, %s): %s\n", tfloat1_out, tfloatseq1_out, char_result);
  free(tfloat_start); free(tfloat_seq); free(tfloat_result); free(char_result);

  /* Temporal *temporal_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_delete_timestamptz(tfloat_start, tstz1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_delete_timestamptz(%s, %s, true): %s\n", tfloat1_out, tstz1_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /* Temporal *temporal_delete_tstzset(const Temporal *temp, const Set *s, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_delete_tstzset(tfloat_start, tstzset1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_delete_tstzset(%s, %s): %s\n", tfloat1_out, tstzset1_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /* Temporal *temporal_delete_tstzspan(const Temporal *temp, const Span *s, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_delete_tstzspan(tfloat_start, tstzspan1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_delete_tstzspan(%s, %s, true): %s\n", tfloat1_out, tstzspan1_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /* Temporal *temporal_delete_tstzspanset(const Temporal *temp, const SpanSet *ss, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_delete_tstzspanset(tfloat_start, tstzspanset1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_delete_tstzspanset(%s, %s, true): %s\n", tfloat1_out, tstzspanset1_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /* Temporal *temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_insert(tfloat_start, tfloat2, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_insert(%s, %s, true): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /* Temporal *temporal_merge(const Temporal *temp1, const Temporal *temp2); */
  Temporal *tfloat_merge =  tfloat_in("7@2001-01-07");
  tfloat_result = temporal_merge(tfloat1, tfloat_merge);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_merge(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_merge); free(tfloat_result); free(char_result);

  /* Temporal *temporal_merge_array(Temporal **temparr, int count); */
  tfloatarray[0] = (Temporal *) tfloatseq1;
  tfloatarray[1] = (Temporal *) tfloatseq2;
  tfloat_result = temporal_merge_array(tfloatarray, 2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_merge_array({%s, %s}, 2): %s\n", tfloatinst1_out, tfloatinst2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect); */
  tfloat_start = tfloat_in("[1@20001-01-01, 3@20001-01-03]");
  tfloat_result = temporal_update(tfloat_start, tfloat2, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_update(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_start); free(tfloat_result); free(char_result);

  /*****************************************************************************
   * Restriction functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tbool_at_value(const Temporal *temp, bool b); */
  tbool_result = tbool_at_value(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("tbool_at_value(%s, %s): %s\n", tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tbool_minus_value(const Temporal *temp, bool b); */
  tbool_result = tbool_minus_value(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("tbool_minus_value(%s, %s): %s\n", tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *temporal_at_max(const Temporal *temp); */
  tfloat_result = temporal_at_max(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_max(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_min(const Temporal *temp); */
  tfloat_result = temporal_at_min(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_min(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_timestamptz(const Temporal *temp, TimestampTz t); */
  tfloat_result = temporal_at_timestamptz(tfloat1, tstz1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_timestamptz(%s, %s): %s\n", tfloat1_out, tstz1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_tstzset(const Temporal *temp, const Set *s); */
  tfloat_result = temporal_at_tstzset(tfloat1, tstzset1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_tstzset(%s, %s): %s\n", tfloat1_out, tstzset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_tstzspan(const Temporal *temp, const Span *s); */
  tfloat_result = temporal_at_tstzspan(tfloat1, tstzspan1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_tstzspan(%s, %s): %s\n", tfloat1_out, tstzspan1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_tstzspanset(const Temporal *temp, const SpanSet *ss); */
  tfloat_result = temporal_at_tstzspanset(tfloat1, tstzspanset1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_tstzspanset(%s, %s): %s\n", tfloat1_out, tstzspanset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_at_values(const Temporal *temp, const Set *set); */
  tfloat_result = temporal_at_values(tfloat1, fset1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_at_values(%s, %s): %s\n", tfloat1_out, fset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_minus_max(const Temporal *temp); */
  tfloat_result = temporal_minus_max(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_minus_max(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_minus_min(const Temporal *temp); */
  tfloat_result = temporal_minus_min(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_minus_min(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_minus_timestamptz(const Temporal *temp, TimestampTz t); */
  tfloat_result = temporal_minus_timestamptz(tfloat1, tstz1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_minus_timestamptz(%s, %s): %s\n", tfloat1_out, tstz1_out, char_result);
  if (tfloat_result)
    free(tfloat_result);
  free(char_result);

  /* Temporal *temporal_minus_tstzset(const Temporal *temp, const Set *s); */
  tfloat_result = temporal_minus_tstzset(tfloat1, tstzset1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_minus_tstzset(%s, %s): %s\n", tfloat1_out, tstzset1_out, char_result);
  if (tfloat_result)
    free(tfloat_result);
  free(char_result);

  /* Temporal *temporal_minus_tstzspan(const Temporal *temp, const Span *s); */
  tfloat_result = temporal_minus_tstzspan(tfloat1, tstzspan1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_minus_tstzspan(%s, %s): %s\n", tfloat1_out, tstzspan1_out, char_result);
  if (tfloat_result)
    free(tfloat_result);
  free(char_result);

  /* Temporal *temporal_minus_tstzspanset(const Temporal *temp, const SpanSet *ss); */
  tfloat_result = temporal_minus_tstzspanset(tfloat1, tstzspanset1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_minus_tstzspanset(%s, %s): %s\n", tfloat1_out, tstzspanset1_out, char_result);
  if (tfloat_result)
    free(tfloat_result);
  free(char_result);

  /* Temporal *temporal_minus_values(const Temporal *temp, const Set *set); */
  tfloat_result = temporal_minus_values(tfloat1, fset1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("temporal_minus_values(%s, %s): %s\n", tfloat1_out, fset1_out, char_result);
  if (tfloat_result)
    free(tfloat_result);
  free(char_result);

  /* Temporal *tfloat_at_value(const Temporal *temp, double d); */
  tfloat_result = tfloat_at_value(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_at_value(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_minus_value(const Temporal *temp, double d); */
  tfloat_result = tfloat_minus_value(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_minus_value(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tint_at_value(const Temporal *temp, int i); */
  tint_result = tint_at_value(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("tint_at_value(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tint_minus_value(const Temporal *temp, int i); */
  tint_result = tint_minus_value(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("tint_minus_value(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tnumber_at_span(const Temporal *temp, const Span *span); */
  tfloat_result = tnumber_at_span(tfloat1, fspan1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_at_span(%s, %s): %s\n", tfloat1_out, fspan1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_at_spanset(const Temporal *temp, const SpanSet *ss); */
  tfloat_result = tnumber_at_spanset(tfloat1, fspanset1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_at_spanset(%s, %s): %s\n", tfloat1_out, fspanset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_at_tbox(const Temporal *temp, const TBox *box); */
  tfloat_result = tnumber_at_tbox(tfloat1, tbox1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_at_tbox(%s, %s): %s\n", tfloat1_out, tbox1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_minus_span(const Temporal *temp, const Span *span); */
  tfloat_result = tnumber_minus_span(tfloat1, fspan1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("tnumber_minus_span(%s, %s): %s\n", tfloat1_out, fspan1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_minus_spanset(const Temporal *temp, const SpanSet *ss); */
  tfloat_result = tnumber_minus_spanset(tfloat1, fspanset1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("tnumber_minus_spanset(%s, %s): %s\n", tfloat1_out, fspanset1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_minus_tbox(const Temporal *temp, const TBox *box); */
  tfloat_result = tnumber_minus_tbox(tfloat1, tbox1);
  char_result = tfloat_result ? tfloat_out(tfloat_result, 6) : text_out(text_null);
  printf("tnumber_minus_tbox(%s, %s): %s\n", tfloat1_out, tbox1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *ttext_at_value(const Temporal *temp, text *txt); */
  ttext_result = ttext_at_value(ttext1, text1);
  char_result = ttext_result ? ttext_out(ttext_result) : text_out(text_null);
  printf("ttext_at_value(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *ttext_minus_value(const Temporal *temp, text *txt); */
  ttext_result = ttext_minus_value(ttext1, text1);
  char_result = ttext_result ? ttext_out(ttext_result) : text_out(text_null);
  printf("ttext_minus_value(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(ttext_result); free(char_result);

  /*****************************************************************************
   * Comparison functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Traditional comparison functions for temporal types */

  /* int temporal_cmp(const Temporal *temp1, const Temporal *temp2); */
  int32_result = temporal_cmp(tfloat1, tfloat2);
  printf("temporal_cmp(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* bool temporal_eq(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_eq(tfloat1, tfloat2);
  printf("temporal_eq(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool temporal_ge(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_ge(tfloat1, tfloat2);
  printf("temporal_ge(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool temporal_gt(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_gt(tfloat1, tfloat2);
  printf("temporal_gt(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool temporal_le(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_le(tfloat1, tfloat2);
  printf("temporal_le(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool temporal_lt(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_lt(tfloat1, tfloat2);
  printf("temporal_lt(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool temporal_ne(const Temporal *temp1, const Temporal *temp2); */
  bool_result = temporal_ne(tfloat1, tfloat2);
  printf("temporal_ne(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /*****************************************************************************/
  /* Ever and always comparison functions for temporal types */
  printf("****************************************************************\n");

  /* int always_eq_bool_tbool(bool b, const Temporal *temp); */
  int32_result = always_eq_bool_tbool(bool1, tbool1);
  printf("always_eq_bool_tbool(%s, %s): %d\n", bool1_out, tbool1_out, int32_result);

  /* int always_eq_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_eq_float_tfloat(float8_in1, tfloat1);
  printf("always_eq_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_eq_int_tint(int i, const Temporal *temp); */
  int32_result = always_eq_int_tint(int32_in1, tint1);
  printf("always_eq_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_eq_tbool_bool(const Temporal *temp, bool b); */
  int32_result = always_eq_tbool_bool(tbool1, bool1);
  printf("always_eq_tbool_bool(%s, %s): %d\n", tbool1_out, bool1_out, int32_result);

  /* int always_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_eq_temporal_temporal(tfloat1, tfloat2);
  printf("always_eq_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_eq_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_eq_text_ttext(text1, ttext1);
  printf("always_eq_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_eq_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_eq_tfloat_float(tfloat1, float8_in1);
  printf("always_eq_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_eq_tint_int(const Temporal *temp, int i); */
  int32_result = always_eq_tint_int(tint1, int32_in1);
  printf("always_eq_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_eq_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_eq_ttext_text(ttext1, text1);
  printf("always_eq_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int always_ge_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_ge_float_tfloat(float8_in1, tfloat1);
  printf("always_ge_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_ge_int_tint(int i, const Temporal *temp); */
  int32_result = always_ge_int_tint(int32_in1, tint1);
  printf("always_ge_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_ge_temporal_temporal(tfloat1, tfloat2);
  printf("always_ge_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_ge_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_ge_text_ttext(text1, ttext1);
  printf("always_ge_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_ge_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_ge_tfloat_float(tfloat1, float8_in1);
  printf("always_ge_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_ge_tint_int(const Temporal *temp, int i); */
  int32_result = always_ge_tint_int(tint1, int32_in1);
  printf("always_ge_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_ge_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_ge_ttext_text(ttext1, text1);
  printf("always_ge_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int always_gt_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_gt_float_tfloat(float8_in1, tfloat1);
  printf("always_gt_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_gt_int_tint(int i, const Temporal *temp); */
  int32_result = always_gt_int_tint(int32_in1, tint1);
  printf("always_gt_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_gt_temporal_temporal(tfloat1, tfloat2);
  printf("always_gt_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_gt_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_gt_text_ttext(text1, ttext1);
  printf("always_gt_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_gt_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_gt_tfloat_float(tfloat1, float8_in1);
  printf("always_gt_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_gt_tint_int(const Temporal *temp, int i); */
  int32_result = always_gt_tint_int(tint1, int32_in1);
  printf("always_gt_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_gt_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_gt_ttext_text(ttext1, text1);
  printf("always_gt_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int always_le_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_le_float_tfloat(float8_in1, tfloat1);
  printf("always_le_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_le_int_tint(int i, const Temporal *temp); */
  int32_result = always_le_int_tint(int32_in1, tint1);
  printf("always_le_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_le_temporal_temporal(tfloat1, tfloat2);
  printf("always_le_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_le_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_le_text_ttext(text1, ttext1);
  printf("always_le_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_le_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_le_tfloat_float(tfloat1, float8_in1);
  printf("always_le_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_le_tint_int(const Temporal *temp, int i); */
  int32_result = always_le_tint_int(tint1, int32_in1);
  printf("always_le_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_le_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_le_ttext_text(ttext1, text1);
  printf("always_le_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int always_lt_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_lt_float_tfloat(float8_in1, tfloat1);
  printf("always_lt_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_lt_int_tint(int i, const Temporal *temp); */
  int32_result = always_lt_int_tint(int32_in1, tint1);
  printf("always_lt_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_lt_temporal_temporal(tfloat1, tfloat2);
  printf("always_lt_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_lt_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_lt_text_ttext(text1, ttext1);
  printf("always_lt_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_lt_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_lt_tfloat_float(tfloat1, float8_in1);
  printf("always_lt_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_lt_tint_int(const Temporal *temp, int i); */
  int32_result = always_lt_tint_int(tint1, int32_in1);
  printf("always_lt_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_lt_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_lt_ttext_text(ttext1, text1);
  printf("always_lt_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int always_ne_bool_tbool(bool b, const Temporal *temp); */
  int32_result = always_ne_bool_tbool(bool1, tbool1);
  printf("always_ne_bool_tbool(%s, %s): %d\n", bool1_out, tbool1_out, int32_result);

  /* int always_ne_float_tfloat(double d, const Temporal *temp); */
  int32_result = always_ne_float_tfloat(float8_in1, tfloat1);
  printf("always_ne_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int always_ne_int_tint(int i, const Temporal *temp); */
  int32_result = always_ne_int_tint(int32_in1, tint1);
  printf("always_ne_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int always_ne_tbool_bool(const Temporal *temp, bool b); */
  int32_result = always_ne_tbool_bool(tbool1, bool1);
  printf("always_ne_tbool_bool(%s, %s): %d\n", tbool1_out, bool1_out, int32_result);

  /* int always_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_ne_temporal_temporal(tfloat1, tfloat2);
  printf("always_ne_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int always_ne_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = always_ne_text_ttext(text1, ttext1);
  printf("always_ne_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int always_ne_tfloat_float(const Temporal *temp, double d); */
  int32_result = always_ne_tfloat_float(tfloat1, float8_in1);
  printf("always_ne_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int always_ne_tint_int(const Temporal *temp, int i); */
  int32_result = always_ne_tint_int(tint1, int32_in1);
  printf("always_ne_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int always_ne_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = always_ne_ttext_text(ttext1, text1);
  printf("always_ne_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_eq_bool_tbool(bool b, const Temporal *temp); */
  int32_result = ever_eq_bool_tbool(bool1, tbool1);
  printf("ever_eq_bool_tbool(%s, %s): %d\n", bool1_out, tbool1_out, int32_result);

  /* int ever_eq_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_eq_float_tfloat(float8_in1, tfloat1);
  printf("ever_eq_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_eq_int_tint(int i, const Temporal *temp); */
  int32_result = ever_eq_int_tint(int32_in1, tint1);
  printf("ever_eq_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_eq_tbool_bool(const Temporal *temp, bool b); */
  int32_result = ever_eq_tbool_bool(tbool1, bool1);
  printf("ever_eq_tbool_bool(%s, %s): %d\n", tbool1_out, bool1_out, int32_result);

  /* int ever_eq_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_eq_temporal_temporal(tfloat1, tfloat2);
  printf("ever_eq_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_eq_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_eq_text_ttext(text1, ttext1);
  printf("ever_eq_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_eq_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_eq_tfloat_float(tfloat1, float8_in1);
  printf("ever_eq_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_eq_tint_int(const Temporal *temp, int i); */
  int32_result = ever_eq_tint_int(tint1, int32_in1);
  printf("ever_eq_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_eq_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_eq_ttext_text(ttext1, text1);
  printf("ever_eq_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_ge_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_ge_float_tfloat(float8_in1, tfloat1);
  printf("ever_ge_float_tfloat(%lf², %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_ge_int_tint(int i, const Temporal *temp); */
  int32_result = ever_ge_int_tint(int32_in1, tint1);
  printf("ever_ge_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_ge_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_ge_temporal_temporal(tfloat1, tfloat2);
  printf("ever_ge_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_ge_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_ge_text_ttext(text1, ttext1);
  printf("ever_ge_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_ge_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_ge_tfloat_float(tfloat1, float8_in1);
  printf("ever_ge_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_ge_tint_int(const Temporal *temp, int i); */
  int32_result = ever_ge_tint_int(tint1, int32_in1);
  printf("ever_ge_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_ge_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_ge_ttext_text(ttext1, text1);
  printf("ever_ge_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_gt_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_gt_float_tfloat(float8_in1, tfloat1);
  printf("ever_gt_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_gt_int_tint(int i, const Temporal *temp); */
  int32_result = ever_gt_int_tint(int32_in1, tint1);
  printf("ever_gt_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_gt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_gt_temporal_temporal(tfloat1, tfloat2);
  printf("ever_gt_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_gt_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_gt_text_ttext(text1, ttext1);
  printf("ever_gt_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_gt_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_gt_tfloat_float(tfloat1, float8_in1);
  printf("ever_gt_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_gt_tint_int(const Temporal *temp, int i); */
  int32_result = ever_gt_tint_int(tint1, int32_in1);
  printf("ever_gt_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_gt_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_gt_ttext_text(ttext1, text1);
  printf("ever_gt_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_le_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_le_float_tfloat(float8_in1, tfloat1);
  printf("ever_le_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_le_int_tint(int i, const Temporal *temp); */
  int32_result = ever_le_int_tint(int32_in1, tint1);
  printf("ever_le_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_le_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_le_temporal_temporal(tfloat1, tfloat2);
  printf("ever_le_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_le_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_le_text_ttext(text1, ttext1);
  printf("ever_le_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_le_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_le_tfloat_float(tfloat1, float8_in1);
  printf("ever_le_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_le_tint_int(const Temporal *temp, int i); */
  int32_result = ever_le_tint_int(tint1, int32_in1);
  printf("ever_le_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_le_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_le_ttext_text(ttext1, text1);
  printf("ever_le_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_lt_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_lt_float_tfloat(float8_in1, tfloat1);
  printf("ever_lt_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_lt_int_tint(int i, const Temporal *temp); */
  int32_result = ever_lt_int_tint(int32_in1, tint1);
  printf("ever_lt_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_lt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_lt_temporal_temporal(tfloat1, tfloat2);
  printf("ever_lt_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_lt_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_lt_text_ttext(text1, ttext1);
  printf("ever_lt_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_lt_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_lt_tfloat_float(tfloat1, float8_in1);
  printf("ever_lt_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_lt_tint_int(const Temporal *temp, int i); */
  int32_result = ever_lt_tint_int(tint1, int32_in1);
  printf("ever_lt_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_lt_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_lt_ttext_text(ttext1, text1);
  printf("ever_lt_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /* int ever_ne_bool_tbool(bool b, const Temporal *temp); */
  int32_result = ever_ne_bool_tbool(bool1, tbool1);
  printf("ever_ne_bool_tbool(%s, %s): %d\n", bool1_out, tbool1_out, int32_result);

  /* int ever_ne_float_tfloat(double d, const Temporal *temp); */
  int32_result = ever_ne_float_tfloat(float8_in1, tfloat1);
  printf("ever_ne_float_tfloat(%lf, %s): %d\n", float8_in1, tfloat1_out, int32_result);

  /* int ever_ne_int_tint(int i, const Temporal *temp); */
  int32_result = ever_ne_int_tint(int32_in1, tint1);
  printf("ever_ne_int_tint(%d, %s): %d\n", int32_in1, tint1_out, int32_result);

  /* int ever_ne_tbool_bool(const Temporal *temp, bool b); */
  int32_result = ever_ne_tbool_bool(tbool1, bool1);
  printf("ever_ne_tbool_bool(%s, %s): %d\n", tbool1_out, bool1_out, int32_result);

  /* int ever_ne_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_ne_temporal_temporal(tfloat1, tfloat2);
  printf("ever_ne_temporal_temporal(%s, %s): %d\n", tfloat1_out, tfloat2_out, int32_result);

  /* int ever_ne_text_ttext(const text *txt, const Temporal *temp); */
  int32_result = ever_ne_text_ttext(text1, ttext1);
  printf("ever_ne_text_ttext(%s, %s): %d\n", text1_out, ttext1_out, int32_result);

  /* int ever_ne_tfloat_float(const Temporal *temp, double d); */
  int32_result = ever_ne_tfloat_float(tfloat1, float8_in1);
  printf("ever_ne_tfloat_float(%s, %lf): %d\n", tfloat1_out, float8_in1, int32_result);

  /* int ever_ne_tint_int(const Temporal *temp, int i); */
  int32_result = ever_ne_tint_int(tint1, int32_in1);
  printf("ever_ne_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int ever_ne_ttext_text(const Temporal *temp, const text *txt); */
  int32_result = ever_ne_ttext_text(ttext1, text1);
  printf("ever_ne_ttext_text(%s, %s): %d\n", ttext1_out, text1_out, int32_result);

  /*****************************************************************************/
  /* Temporal comparison functions for temporal types */
  printf("****************************************************************\n");

  /* Temporal *teq_bool_tbool(bool b, const Temporal *temp); */
  tbool_result = teq_bool_tbool(bool1, tbool1);
  char_result = tbool_out(tbool_result);
  printf("teq_bool_tbool(%s, %s): %s\n", bool1_out, tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_float_tfloat(double d, const Temporal *temp); */
  tbool_result = teq_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("teq_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_int_tint(int i, const Temporal *temp); */
  tbool_result = teq_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("teq_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_tbool_bool(const Temporal *temp, bool b); */
  tbool_result = teq_tbool_bool(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("teq_tbool_bool(%s, %s): %s\n", tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = teq_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("teq_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = teq_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("teq_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_tfloat_float(const Temporal *temp, double d); */
  tbool_result = teq_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("teq_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_tint_int(const Temporal *temp, int i); */
  tbool_result = teq_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("teq_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = teq_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("teq_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_float_tfloat(double d, const Temporal *temp); */
  tbool_result = tge_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("tge_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_int_tint(int i, const Temporal *temp); */
  tbool_result = tge_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("tge_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tge_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("tge_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = tge_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("tge_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_tfloat_float(const Temporal *temp, double d); */
  tbool_result = tge_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("tge_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_tint_int(const Temporal *temp, int i); */
  tbool_result = tge_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("tge_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tge_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = tge_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("tge_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_float_tfloat(double d, const Temporal *temp); */
  tbool_result = tgt_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("tgt_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_int_tint(int i, const Temporal *temp); */
  tbool_result = tgt_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("tgt_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tgt_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("tgt_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = tgt_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("tgt_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_tfloat_float(const Temporal *temp, double d); */
  tbool_result = tgt_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("tgt_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_tint_int(const Temporal *temp, int i); */
  tbool_result = tgt_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("tgt_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tgt_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = tgt_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("tgt_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_float_tfloat(double d, const Temporal *temp); */
  tbool_result = tle_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("tle_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_int_tint(int i, const Temporal *temp); */
  tbool_result = tle_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("tle_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tle_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("tle_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = tle_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("tle_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_tfloat_float(const Temporal *temp, double d); */
  tbool_result = tle_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("tle_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_tint_int(const Temporal *temp, int i); */
  tbool_result = tle_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("tle_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tle_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = tle_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("tle_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_float_tfloat(double d, const Temporal *temp); */
  tbool_result = tlt_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("tlt_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_int_tint(int i, const Temporal *temp); */
  tbool_result = tlt_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("tlt_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tlt_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("tlt_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = tlt_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("tlt_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_tfloat_float(const Temporal *temp, double d); */
  tbool_result = tlt_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("tlt_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_tint_int(const Temporal *temp, int i); */
  tbool_result = tlt_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("tlt_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tlt_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = tlt_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("tlt_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_bool_tbool(bool b, const Temporal *temp); */
  tbool_result = tne_bool_tbool(bool1, tbool1);
  char_result = tbool_out(tbool_result);
  printf("tne_bool_tbool(%s, %s): %s\n", bool1_out, tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_float_tfloat(double d, const Temporal *temp); */
  tbool_result = tne_float_tfloat(float8_in1, tfloat1);
  char_result = tbool_out(tbool_result);
  printf("tne_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_int_tint(int i, const Temporal *temp); */
  tbool_result = tne_int_tint(int32_in1, tint1);
  char_result = tbool_out(tbool_result);
  printf("tne_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tbool_bool(const Temporal *temp, bool b); */
  tbool_result = tne_tbool_bool(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("tne_tbool_bool(%s, %s): %s\n", tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tne_temporal_temporal(tfloat1, tfloat2);
  char_result = tbool_out(tbool_result);
  printf("tne_temporal_temporal(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_text_ttext(const text *txt, const Temporal *temp); */
  tbool_result = tne_text_ttext(text1, ttext1);
  char_result = tbool_out(tbool_result);
  printf("tne_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tfloat_float(const Temporal *temp, double d); */
  tbool_result = tne_tfloat_float(tfloat1, float8_in1);
  char_result = tbool_out(tbool_result);
  printf("tne_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tint_int(const Temporal *temp, int i); */
  tbool_result = tne_tint_int(tint1, int32_in1);
  char_result = tbool_out(tbool_result);
  printf("tne_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_ttext_text(const Temporal *temp, const text *txt); */
  tbool_result = tne_ttext_text(ttext1, text1);
  char_result = tbool_out(tbool_result);
  printf("tne_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(tbool_result); free(char_result);

  /*****************************************************************************
   * Bounding box functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Split functions */

  /* Span *temporal_spans(const Temporal *temp, int *count); */
  tstzspanarray_result = temporal_spans(tfloat1, &count);
  printf("temporal_spans(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tstzspan_out(&tstzspanarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzspanarray_result);

  /* Span *temporal_split_each_n_spans(const Temporal *temp, int elem_count, int *count); */
  tstzspanarray_result = temporal_split_each_n_spans(tfloat1, int32_in1, &count);
  printf("temporal_split_each_n_spans(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tstzspan_out(&tstzspanarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzspanarray_result);

  /* Span *temporal_split_n_spans(const Temporal *temp, int span_count, int *count); */
  tstzspanarray_result = temporal_split_n_spans(tfloat1, int32_in1, &count);
  printf("temporal_split_n_spans(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tstzspan_out(&tstzspanarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzspanarray_result);

  /* TBox *tnumber_split_each_n_tboxes(const Temporal *temp, int elem_count, int *count); */
  tboxarray_result = tnumber_split_each_n_tboxes(tfloat1, int32_in1, &count);
  printf("tnumber_split_each_n_tboxes(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tnumber_split_n_tboxes(const Temporal *temp, int box_count, int *count); */
  tboxarray_result = tnumber_split_n_tboxes(tfloat1, int32_in1, &count);
  printf("tnumber_split_n_tboxes(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tnumber_tboxes(const Temporal *temp, int *count); */
  tboxarray_result = tnumber_tboxes(tfloat1, &count);
  printf("tnumber_tboxes(%s): {", tfloat1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Topological functions for temporal types */

  /* bool adjacent_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = adjacent_numspan_tnumber(fspan1, tfloat1);
  printf("adjacent_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = adjacent_tbox_tnumber(tbox1, tfloat1);
  printf("adjacent_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = adjacent_temporal_temporal(tfloat1, tfloat2);
  printf("adjacent_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool adjacent_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = adjacent_temporal_tstzspan(tfloat1, tstzspan1);
  printf("adjacent_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = adjacent_tnumber_numspan(tfloat1, fspan1);
  printf("adjacent_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = adjacent_tnumber_tbox(tfloat1, tbox1);
  printf("adjacent_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = adjacent_tnumber_tnumber(tfloat1, tfloat2);
  printf("adjacent_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool adjacent_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = adjacent_tstzspan_temporal(tstzspan1, tfloat1);
  printf("adjacent_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool contained_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = contained_numspan_tnumber(fspan1, tfloat1);
  printf("contained_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool contained_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = contained_tbox_tnumber(tbox1, tfloat1);
  printf("contained_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contained_temporal_temporal(tfloat1, tfloat2);
  printf("contained_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool contained_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = contained_temporal_tstzspan(tfloat1, tstzspan1);
  printf("contained_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool contained_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = contained_tnumber_numspan(tfloat1, fspan1);
  printf("contained_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool contained_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = contained_tnumber_tbox(tfloat1, tbox1);
  printf("contained_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool contained_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contained_tnumber_tnumber(tfloat1, tfloat2);
  printf("contained_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool contained_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = contained_tstzspan_temporal(tstzspan1, tfloat1);
  printf("contained_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool contains_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = contains_numspan_tnumber(fspan1, tfloat1);
  printf("contains_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool contains_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = contains_tbox_tnumber(tbox1, tfloat1);
  printf("contains_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool contains_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = contains_temporal_tstzspan(tfloat1, tstzspan1);
  printf("contains_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contains_temporal_temporal(tfloat1, tfloat2);
  printf("contains_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool contains_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = contains_tnumber_numspan(tfloat1, fspan1);
  printf("contains_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool contains_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = contains_tnumber_tbox(tfloat1, tbox1);
  printf("contains_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool contains_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contains_tnumber_tnumber(tfloat1, tfloat2);
  printf("contains_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool contains_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = contains_tstzspan_temporal(tstzspan1, tfloat1);
  printf("contains_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overlaps_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = overlaps_numspan_tnumber(fspan1, tfloat1);
  printf("overlaps_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = overlaps_tbox_tnumber(tbox1, tfloat1);
  printf("overlaps_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overlaps_temporal_temporal(tfloat1, tfloat2);
  printf("overlaps_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overlaps_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = overlaps_temporal_tstzspan(tfloat1, tstzspan1);
  printf("overlaps_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = overlaps_tnumber_numspan(tfloat1, fspan1);
  printf("overlaps_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = overlaps_tnumber_tbox(tfloat1, tbox1);
  printf("overlaps_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overlaps_tnumber_tnumber(tfloat1, tfloat2);
  printf("overlaps_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overlaps_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = overlaps_tstzspan_temporal(tstzspan1, tfloat1);
  printf("overlaps_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool same_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = same_numspan_tnumber(fspan1, tfloat1);
  printf("same_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool same_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = same_tbox_tnumber(tbox1, tfloat1);
  printf("same_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool same_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = same_temporal_temporal(tfloat1, tfloat2);
  printf("same_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool same_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = same_temporal_tstzspan(tfloat1, tstzspan1);
  printf("same_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool same_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = same_tnumber_numspan(tfloat1, fspan1);
  printf("same_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool same_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = same_tnumber_tbox(tfloat1, tbox1);
  printf("same_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool same_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = same_tnumber_tnumber(tfloat1, tfloat2);
  printf("same_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool same_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = same_tstzspan_temporal(tstzspan1, tfloat1);
  printf("same_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /*****************************************************************************/
  /* Position functions for temporal types */
  printf("****************************************************************\n");

  /* bool after_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = after_tbox_tnumber(tbox1, tfloat1);
  printf("after_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool after_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = after_temporal_tstzspan(tfloat1, tstzspan1);
  printf("after_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool after_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = after_temporal_temporal(tfloat1, tfloat2);
  printf("after_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool after_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = after_tnumber_tbox(tfloat1, tbox1);
  printf("after_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool after_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = after_tnumber_tnumber(tfloat1, tfloat2);
  printf("after_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool after_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = after_tstzspan_temporal(tstzspan1, tfloat1);
  printf("after_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool before_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = before_tbox_tnumber(tbox1, tfloat1);
  printf("before_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool before_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = before_temporal_tstzspan(tfloat1, tstzspan1);
  printf("before_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool before_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = before_temporal_temporal(tfloat1, tfloat2);
  printf("before_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool before_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = before_tnumber_tbox(tfloat1, tbox1);
  printf("before_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool before_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = before_tnumber_tnumber(tfloat1, tfloat2);
  printf("before_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool before_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = before_tstzspan_temporal(tstzspan1, tfloat1);
  printf("before_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool left_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = left_tbox_tnumber(tbox1, tfloat1);
  printf("left_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool left_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = left_numspan_tnumber(fspan1, tfloat1);
  printf("left_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool left_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = left_tnumber_numspan(tfloat1, fspan1);
  printf("left_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool left_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = left_tnumber_tbox(tfloat1, tbox1);
  printf("left_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool left_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = left_tnumber_tnumber(tfloat1, tfloat2);
  printf("left_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overafter_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = overafter_tbox_tnumber(tbox1, tfloat1);
  printf("overafter_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overafter_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = overafter_temporal_tstzspan(tfloat1, tstzspan1);
  printf("overafter_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overafter_temporal_temporal(tfloat1, tfloat2);
  printf("overafter_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overafter_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = overafter_tnumber_tbox(tfloat1, tbox1);
  printf("overafter_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool overafter_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overafter_tnumber_tnumber(tfloat1, tfloat2);
  printf("overafter_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overafter_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = overafter_tstzspan_temporal(tstzspan1, tfloat1);
  printf("overafter_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overbefore_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = overbefore_tbox_tnumber(tbox1, tfloat1);
  printf("overbefore_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overbefore_temporal_tstzspan(const Temporal *temp, const Span *s); */
  bool_result = overbefore_temporal_tstzspan(tfloat1, tstzspan1);
  printf("overbefore_temporal_tstzspan(%s, %s): %c\n", tfloat1_out, tstzspan1_out, bool_result ? 't' : 'n');

  /* bool overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overbefore_temporal_temporal(tfloat1, tfloat2);
  printf("overbefore_temporal_temporal(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overbefore_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = overbefore_tnumber_tbox(tfloat1, tbox1);
  printf("overbefore_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool overbefore_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overbefore_tnumber_tnumber(tfloat1, tfloat2);
  printf("overbefore_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overbefore_tstzspan_temporal(const Span *s, const Temporal *temp); */
  bool_result = overbefore_tstzspan_temporal(tstzspan1, tfloat1);
  printf("overbefore_tstzspan_temporal(%s, %s): %c\n", tstzspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overleft_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = overleft_numspan_tnumber(fspan1, tfloat1);
  printf("overleft_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overleft_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = overleft_tbox_tnumber(tbox1, tfloat1);
  printf("overleft_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overleft_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = overleft_tnumber_numspan(tfloat1, fspan1);
  printf("overleft_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool overleft_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = overleft_tnumber_tbox(tfloat1, tbox1);
  printf("overleft_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool overleft_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overleft_tnumber_tnumber(tfloat1, tfloat2);
  printf("overleft_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool overright_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = overright_numspan_tnumber(fspan1, tfloat1);
  printf("overright_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overright_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = overright_tbox_tnumber(tbox1, tfloat1);
  printf("overright_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool overright_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = overright_tnumber_numspan(tfloat1, fspan1);
  printf("overright_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool overright_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = overright_tnumber_tbox(tfloat1, tbox1);
  printf("overright_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool overright_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overright_tnumber_tnumber(tfloat1, tfloat2);
  printf("overright_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /* bool right_numspan_tnumber(const Span *s, const Temporal *temp); */
  bool_result = right_numspan_tnumber(fspan1, tfloat1);
  printf("right_numspan_tnumber(%s, %s): %c\n", fspan1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool right_tbox_tnumber(const TBox *box, const Temporal *temp); */
  bool_result = right_tbox_tnumber(tbox1, tfloat1);
  printf("right_tbox_tnumber(%s, %s): %c\n", tbox1_out, tfloat1_out, bool_result ? 't' : 'n');

  /* bool right_tnumber_numspan(const Temporal *temp, const Span *s); */
  bool_result = right_tnumber_numspan(tfloat1, fspan1);
  printf("right_tnumber_numspan(%s, %s): %c\n", tfloat1_out, fspan1_out, bool_result ? 't' : 'n');

  /* bool right_tnumber_tbox(const Temporal *temp, const TBox *box); */
  bool_result = right_tnumber_tbox(tfloat1, tbox1);
  printf("right_tnumber_tbox(%s, %s): %c\n", tfloat1_out, tbox1_out, bool_result ? 't' : 'n');

  /* bool right_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  bool_result = right_tnumber_tnumber(tfloat1, tfloat2);
  printf("right_tnumber_tnumber(%s, %s): %c\n", tfloat1_out, tfloat2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Boolean functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tand_bool_tbool(bool b, const Temporal *temp); */
  tbool_result = tand_bool_tbool(bool1, tbool1);
  char_result = tbool_out(tbool_result);
  printf("tand_bool_tbool(%s, %s): %s\n", bool1_out, tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tand_tbool_bool(const Temporal *temp, bool b); */
  tbool_result = tand_tbool_bool(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("tand_tbool_bool(%s, %s): %s\n", tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tand_tbool_tbool(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tand_tbool_tbool(tbool1, tbool2);
  char_result = tbool_out(tbool_result);
  printf("tand_tbool_tbool(%s, %s): %s\n", tbool1_out, tbool2_out, char_result);
  free(tbool_result); free(char_result);

  /* SpanSet *tbool_when_true(const Temporal *temp); */
  tstzspanset_result = tbool_when_true(tbool1);
  char_result = tstzspanset_out(tstzspanset_result);
  printf("tbool_when_true(%s): %s\n", tbool1_out, char_result);
  free(tstzspanset_result); free(char_result);

  /* Temporal *tnot_tbool(const Temporal *temp); */
  tbool_result = tnot_tbool(tbool1);
  char_result = tbool_out(tbool_result);
  printf("tnot_tbool(%s): %s\n", tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tor_bool_tbool(bool b, const Temporal *temp); */
  tbool_result = tor_bool_tbool(bool1, tbool1);
  char_result = tbool_out(tbool_result);
  printf("tor_bool_tbool(%s, %s): %s\n", bool1_out, tbool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tor_tbool_bool(const Temporal *temp, bool b); */
  tbool_result = tor_tbool_bool(tbool1, bool1);
  char_result = tbool_out(tbool_result);
  printf("tor_tbool_bool(%s, %s): %s\n",  tbool1_out, bool1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tor_tbool_tbool(const Temporal *temp1, const Temporal *temp2); */
  tbool_result = tor_tbool_tbool(tbool1, tbool2);
  char_result = tbool_out(tbool_result);
  printf("tor_tbool_tbool(%s, %s): %s\n", tbool1_out, tbool2_out, char_result);
  free(tbool_result); free(char_result);

  /*****************************************************************************
   * Mathematical functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *add_float_tfloat(double d, const Temporal *tnumber); */
  tfloat_result = add_float_tfloat(float8_in1, tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("add_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *add_int_tint(int i, const Temporal *tnumber); */
  tint_result = add_int_tint(int32_in1, tint1);
  char_result = tint_out(tint_result);
  printf("add_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tint_result); free(char_result);

  /* Temporal *add_tfloat_float(const Temporal *tnumber, double d); */
  tfloat_result = add_tfloat_float(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("add_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *add_tint_int(const Temporal *tnumber, int i); */
  tint_result = add_tint_int(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("add_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *add_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2); */
  tfloat_result = add_tnumber_tnumber(tfloat1, tfloat2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("add_tnumber_tnumber(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *div_float_tfloat(double d, const Temporal *tnumber); */
  tfloat_result = div_float_tfloat(float8_in1, tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("div_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *div_int_tint(int i, const Temporal *tnumber); */
  tint_result = div_int_tint(int32_in1, tint1);
  char_result = tint_out(tint_result);
  printf("div_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tint_result); free(char_result);

  /* Temporal *div_tfloat_float(const Temporal *tnumber, double d); */
  tfloat_result = div_tfloat_float(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("div_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *div_tint_int(const Temporal *tnumber, int i); */
  tint_result = div_tint_int(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("div_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *div_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2); */
  tfloat_result = div_tnumber_tnumber(tfloat1, tfloat2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("div_tnumber_tnumber(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *mult_float_tfloat(double d, const Temporal *tnumber); */
  tfloat_result = mult_float_tfloat(float8_in1, tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("mult_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *mult_int_tint(int i, const Temporal *tnumber); */
  tint_result = mult_int_tint(int32_in1, tint1);
  char_result = tint_out(tint_result);
  printf("mult_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tint_result); free(char_result);

  /* Temporal *mult_tfloat_float(const Temporal *tnumber, double d); */
  tfloat_result = mult_tfloat_float(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("mult_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *mult_tint_int(const Temporal *tnumber, int i); */
  tint_result = mult_tint_int(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("mult_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *mult_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2); */
  tfloat_result = mult_tnumber_tnumber(tfloat1, tfloat2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("mult_tnumber_tnumber(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *sub_float_tfloat(double d, const Temporal *tnumber); */
  tfloat_result = sub_float_tfloat(float8_in1, tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("sub_float_tfloat(%lf, %s): %s\n", float8_in1, tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *sub_int_tint(int i, const Temporal *tnumber); */
  tint_result = sub_int_tint(int32_in1, tint1);
  char_result = tint_out(tint_result);
  printf("sub_int_tint(%d, %s): %s\n", int32_in1, tint1_out, char_result);
  free(tint_result); free(char_result);

  /* Temporal *sub_tfloat_float(const Temporal *tnumber, double d); */
  tfloat_result = sub_tfloat_float(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("sub_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *sub_tint_int(const Temporal *tnumber, int i); */
  tint_result = sub_tint_int(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("sub_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *sub_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2); */
  tfloat_result = sub_tnumber_tnumber(tfloat1, tfloat2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("sub_tnumber_tnumber(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_derivative(const Temporal *temp); */
  tfloat_result = temporal_derivative(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_derivative(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_exp(const Temporal *temp); */
  tfloat_result = tfloat_exp(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_exp(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_ln(const Temporal *temp); */
  tfloat_result = tfloat_ln(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_ln(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tfloat_log10(const Temporal *temp); */
  tfloat_result = tfloat_log10(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_log10(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_abs(const Temporal *temp); */
  tfloat_result = tnumber_abs(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_abs(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_angular_difference(const Temporal *temp); */
  tfloat_result = tnumber_angular_difference(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_angular_difference(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tnumber_delta_value(const Temporal *temp); */
  tfloat_result = tnumber_delta_value(tfloat1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_delta_value(%s): %s\n", tfloat1_out, char_result);
  free(tfloat_result); free(char_result);

  /*****************************************************************************
   * Text functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *textcat_text_ttext(const text *txt, const Temporal *temp); */
  ttext_result = textcat_text_ttext(text1, ttext1);
  char_result = ttext_out(ttext_result);
  printf("textcat_text_ttext(%s, %s): %s\n", text1_out, ttext1_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *textcat_ttext_text(const Temporal *temp, const text *txt); */
  ttext_result = textcat_ttext_text(ttext1, text1);
  char_result = ttext_out(ttext_result);
  printf("textcat_ttext_text(%s, %s): %s\n", ttext1_out, text1_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *textcat_ttext_ttext(const Temporal *temp1, const Temporal *temp2); */
  ttext_result = textcat_ttext_ttext(ttext1, ttext2);
  char_result = ttext_out(ttext_result);
  printf("textcat_ttext_ttext(%s, %s): %s\n", ttext1_out, ttext2_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *ttext_initcap(const Temporal *temp); */
  ttext_result = ttext_initcap(ttext1);
  char_result = ttext_out(ttext_result);
  printf("ttext_initcap(%s): %s\n", ttext1_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *ttext_upper(const Temporal *temp); */
  ttext_result = ttext_upper(ttext1);
  char_result = ttext_out(ttext_result);
  printf("ttext_upper(%s): %s\n", ttext1_out, char_result);
  free(ttext_result); free(char_result);

  /* Temporal *ttext_lower(const Temporal *temp); */
  ttext_result = ttext_lower(ttext1);
  char_result = ttext_out(ttext_result);
  printf("ttext_lower(%s): %s\n", ttext1_out, char_result);
  free(ttext_result); free(char_result);

  /*****************************************************************************
   * Distance functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tdistance_tfloat_float(const Temporal *temp, double d); */
  tfloat_result = tdistance_tfloat_float(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tfloat_float(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tint_int(const Temporal *temp, int i); */
  tint_result = tdistance_tint_int(tint1, int32_in1);
  char_result = tint_out(tint_result);
  printf("tdistance_tint_int(%s, %d): %s\n", tint1_out, int32_in1, char_result);
  free(tint_result); free(char_result);

  /* Temporal *tdistance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2); */
  tfloat_result = tdistance_tnumber_tnumber(tfloat1, tfloat2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tnumber_tnumber(%s, %s): %s\n", tfloat1_out, tfloat2_out, char_result);
  free(tfloat_result); free(char_result);

  /* double nad_tboxfloat_tboxfloat(const TBox *box1, const TBox *box2); */
  float8_result = nad_tboxfloat_tboxfloat(tbox1, tbox2);
  printf("nad_tboxfloat_tboxfloat(%s, %s): %lf\n", tbox1_out, tbox2_out, float8_result);

  /* int nad_tboxint_tboxint(const TBox *box1, const TBox *box2); */
  int32_result = nad_tboxint_tboxint(tintbox1, tintbox2);
  printf("nad_tboxint_tboxint(%s, %s): %d\n", tintbox1_out, tintbox2_out, int32_result);

  /* double nad_tfloat_float(const Temporal *temp, double d); */
  float8_result = nad_tfloat_float(tfloat1, float8_in1);
  printf("nad_tfloat_float(%s, %lf): %lf\n", tfloat1_out, float8_in1, float8_result);

  /* double nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2); */
  float8_result = nad_tfloat_tfloat(tfloat1, tfloat2);
  printf("nad_tfloat_tfloat(%s, %s): %lf\n", tfloat1_out, tfloat2_out, float8_result);

  /* double nad_tfloat_tbox(const Temporal *temp, const TBox *box); */
  float8_result = nad_tfloat_tbox(tfloat1, tbox1);
  printf("nad_tfloat_tbox(%s, %s): %lf\n", tfloat1_out, tbox1_out, float8_result);

  /* int nad_tint_int(const Temporal *temp, int i); */
  int32_result = nad_tint_int(tint1, int32_in1);
  printf("nad_tint_int(%s, %d): %d\n", tint1_out, int32_in1, int32_result);

  /* int nad_tint_tbox(const Temporal *temp, const TBox *box); */
  int32_result = nad_tint_tbox(tint1, tintbox1);
  printf("nad_tint_tbox(%s, %s): %d\n", tint1_out, tintbox1_out, int32_result);

  /* int nad_tint_tint(const Temporal *temp1, const Temporal *temp2); */
  int32_result = nad_tint_tint(tint1, tint2);
  printf("nad_tint_tint(%s, %s): %d\n", tint1_out, tint2_out, int32_result);

  /*****************************************************************************
   * Aggregate functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* SkipList *tbool_tand_transfn(SkipList *state, const Temporal *temp); */
  sklist = tbool_tand_transfn(NULL, tbool1);
  sklist = tbool_tand_transfn(sklist, tbool2);
  tbool_result = temporal_tagg_finalfn(sklist);
  char_result = tbool_out(tbool_result);
  printf("tbool_tand aggregate\n");
  printf("%s\n", tbool1_out);
  printf("%s\n", tbool2_out);
  printf("tbool_tand result\n");
  printf("%s\n", char_result);
  free(tbool_result); free(char_result);

  /* SkipList *tbool_tor_transfn(SkipList *state, const Temporal *temp); */
  sklist = tbool_tor_transfn(NULL, tbool1);
  sklist = tbool_tor_transfn(sklist, tbool2);
  tbool_result = temporal_tagg_finalfn(sklist);
  char_result = tbool_out(tbool_result);
  printf("tbool_tor aggregate\n");
  printf("%s\n", tbool1_out);
  printf("%s\n", tbool2_out);
  printf("tbool_tand result\n");
  printf("%s\n", char_result);
  free(tbool_result); free(char_result);

  /* Span *temporal_extent_transfn(Span *s, const Temporal *temp); */
  Span *result_agg = temporal_extent_transfn(NULL, tbool1);
  result_agg = temporal_extent_transfn(result_agg, tbool2);
  char_result = tstzspan_out(result_agg);
  printf("temporal_extent aggregate\n");
  printf("%s\n", tbool1_out);
  printf("%s\n", tbool2_out);
  printf("temporal_extent result\n");
  printf("%s\n", char_result);
  free(result_agg); free(char_result);

  /* SkipList *temporal_tcount_transfn(SkipList *state, const Temporal *temp); */
  sklist = temporal_tcount_transfn(NULL, tfloat1);
  sklist = temporal_tcount_transfn(sklist, tfloat2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("temporal_tcount aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("temporal_tcount result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tfloat_tmax_transfn(SkipList *state, const Temporal *temp); */
  sklist = tfloat_tmax_transfn(NULL, tfloat1);
  sklist = tfloat_tmax_transfn(sklist, tfloat2);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_tmax aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_tmax result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tfloat_tmin_transfn(SkipList *state, const Temporal *temp); */
  sklist = tfloat_tmin_transfn(NULL, tfloat1);
  sklist = tfloat_tmin_transfn(sklist, tfloat2);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_tmin aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_tmin result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tfloat_tsum_transfn(SkipList *state, const Temporal *temp); */
  sklist = tfloat_tsum_transfn(NULL, tfloat1);
  sklist = tfloat_tsum_transfn(sklist, tfloat2);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_tsum aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_tsum result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tfloat_wmax_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  sklist = tfloat_wmax_transfn(NULL, tfloat1, interv3);
  sklist = tfloat_wmax_transfn(sklist, tfloat2, interv3);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_wmax aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_wmax result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tfloat_wmin_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  sklist = tfloat_wmin_transfn(NULL, tfloat1, interv3);
  sklist = tfloat_wmin_transfn(sklist, tfloat2, interv3);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_wmin aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_wmin result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tfloat_wsum_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  sklist = tfloat_wsum_transfn(NULL, tfloat1, interv3);
  sklist = tfloat_wsum_transfn(sklist, tfloat2, interv3);
  tfloat_result = temporal_tagg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tfloat_wsum aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tfloat_wsum result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *timestamptz_tcount_transfn(SkipList *state, TimestampTz t); */
  sklist = timestamptz_tcount_transfn(NULL, tstz1);
  sklist = timestamptz_tcount_transfn(sklist, tstz2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("timestamptz_tcount aggregate\n");
  printf("%s\n", tstz1_out);
  printf("%s\n", tstz2_out);
  printf("timestamptz_tcount result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_tmax_transfn(SkipList *state, const Temporal *temp); */
  sklist = tint_tmax_transfn(NULL, tint1);
  sklist = tint_tmax_transfn(sklist, tint2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_tmax aggregate\n");
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_tmax result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_tmin_transfn(SkipList *state, const Temporal *temp); */
  sklist = tint_tmin_transfn(NULL, tint1);
  sklist = tint_tmin_transfn(sklist, tint2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_tmin aggregate\n");
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_tmin result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_tsum_transfn(SkipList *state, const Temporal *temp); */
  sklist = tint_tsum_transfn(NULL, tint1);
  sklist = tint_tsum_transfn(sklist, tint2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_tsum aggregate\n");
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_tsum result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_wmax_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  SkipList *tint_wmax_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
  sklist = tint_wmax_transfn(NULL, tint1, interv3);
  sklist = tint_wmax_transfn(sklist, tint2, interv3);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_wmax aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_wmax result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_wmin_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  SkipList *tint_wmin_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
  sklist = tint_wmin_transfn(NULL, tint1, interv3);
  sklist = tint_wmin_transfn(sklist, tint2, interv3);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_wmin aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_wmin result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tint_wsum_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  SkipList *tint_wsum_transfn(SkipList *state, const Temporal *temp, const Interval *interv);
  sklist = tint_wsum_transfn(NULL, tint1, interv3);
  sklist = tint_wsum_transfn(sklist, tint2, interv3);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tint_wsum aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tint1_out);
  printf("%s\n", tint2_out);
  printf("tint_wsum result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* TBox *tnumber_extent_transfn(TBox *box, const Temporal *temp); */
  TBox *tbox_agg = tnumber_extent_transfn(NULL, tfloat1);
  tbox_agg = tnumber_extent_transfn(tbox_agg, tfloat2);
  char_result = tbox_out(tbox_agg, 6);
  printf("tnumber_extent aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tnumber_extent result\n");
  printf("%s\n", char_result);
  free(tbox_agg); free(char_result);

  /* SkipList *tnumber_tavg_transfn(SkipList *state, const Temporal *temp); */
  sklist = tnumber_tavg_transfn(NULL, tfloat1);
  sklist = tnumber_tavg_transfn(sklist, tfloat2);
  tfloat_result = tnumber_tavg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_tavg aggregate\n");
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tnumber_tavg result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tnumber_wavg_transfn(SkipList *state, const Temporal *temp, const Interval *interv); */
  sklist = tnumber_wavg_transfn(NULL, tfloat1, interv3);
  sklist = tnumber_wavg_transfn(sklist, tfloat2, interv3);
  tfloat_result = tnumber_tavg_finalfn(sklist);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnumber_wavg aggregate with interval = %s\n", interv3_out);
  printf("%s\n", tfloat1_out);
  printf("%s\n", tfloat2_out);
  printf("tnumber_wavg result\n");
  printf("%s\n", char_result);
  free(tfloat_result); free(char_result);

  /* SkipList *tstzset_tcount_transfn(SkipList *state, const Set *s); */
  sklist = tstzset_tcount_transfn(NULL, tstzset1);
  sklist = tstzset_tcount_transfn(sklist, tstzset2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tstzset_tcount aggregate\n");
  printf("%s\n", tstzspan1_out);
  printf("%s\n", tstzspan2_out);
  printf("tstzset_tcount result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tstzspan_tcount_transfn(SkipList *state, const Span *s); */
  sklist = tstzspan_tcount_transfn(NULL, tstzspan1);
  sklist = tstzspan_tcount_transfn(sklist, tstzspan2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tstzspan_tcount aggregate\n");
  printf("%s\n", tstzspan1_out);
  printf("%s\n", tstzspan2_out);
  printf("tstzspan_tcount result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *tstzspanset_tcount_transfn(SkipList *state, const SpanSet *ss); */
  sklist = tstzspanset_tcount_transfn(NULL, tstzspanset1);
  sklist = tstzspanset_tcount_transfn(sklist, tstzspanset2);
  tint_result = temporal_tagg_finalfn(sklist);
  char_result = tint_out(tint_result);
  printf("tstzspanset_tcount aggregate\n");
  printf("%s\n", tstzspanset1_out);
  printf("%s\n", tstzspanset2_out);
  printf("tstzspanset_tcount result\n");
  printf("%s\n", char_result);
  free(tint_result); free(char_result);

  /* SkipList *ttext_tmax_transfn(SkipList *state, const Temporal *temp); */
  sklist = ttext_tmax_transfn(NULL, ttext1);
  sklist = ttext_tmax_transfn(sklist, ttext2);
  ttext_result = temporal_tagg_finalfn(sklist);
  char_result = ttext_out(ttext_result);
  printf("ttext_tmax aggregate\n");
  printf("%s\n", ttext1_out);
  printf("%s\n", ttext2_out);
  printf("ttext_tmax result\n");
  printf("%s\n", char_result);
  free(ttext_result); free(char_result);

  /* SkipList *ttext_tmin_transfn(SkipList *state, const Temporal *temp); */
  sklist = ttext_tmin_transfn(NULL, ttext1);
  sklist = ttext_tmin_transfn(sklist, ttext2);
  ttext_result = temporal_tagg_finalfn(sklist);
  char_result = ttext_out(ttext_result);
  printf("ttext_tmin aggregate\n");
  printf("%s\n", ttext1_out);
  printf("%s\n", ttext2_out);
  printf("ttext_tmin result\n");
  printf("%s\n", char_result);
  free(ttext_result); free(char_result);

  /*****************************************************************************
   * Analytics functions for temporal types
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Simplification functions for temporal types */

  /* Temporal *temporal_simplify_dp(const Temporal *temp, double dist, bool synchronized); */
  tfloat_result = temporal_simplify_dp(tfloat1, float8_in1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_simplify_dp(%s, %lf, true): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_simplify_max_dist(const Temporal *temp, double dist, bool synchronized); */
  tfloat_result = temporal_simplify_max_dist(tfloat1, float8_in1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_simplify_max_dist(%s, %lf, true): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_simplify_min_dist(const Temporal *temp, double dist); */
  tfloat_result = temporal_simplify_min_dist(tfloat1, float8_in1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_simplify_min_dist(%s, %lf): %s\n", tfloat1_out, float8_in1, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_simplify_min_tdelta(const Temporal *temp, const Interval *mint); */
  tfloat_result = temporal_simplify_min_tdelta(tfloat1, interv1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_simplify_min_tdelta(%s, %s): %s\n", tfloat1_out, interv1_out, char_result);
  free(tfloat_result); free(char_result);

  /*****************************************************************************/
  /* Reduction functions for temporal types */
  printf("****************************************************************\n");

  /* Temporal *temporal_tprecision(const Temporal *temp, const Interval *duration, TimestampTz origin); */
  tfloat_result = temporal_tprecision(tfloat1, interv1, tstz1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_tprecision(%s, %s, %s): %s\n", tfloat1_out, interv1_out, tstz1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *temporal_tsample(const Temporal *temp, const Interval *duration, TimestampTz origin, interpType interp); */
  tfloat_result = temporal_tsample(tfloat1, interv1, tstz1, LINEAR);
  char_result = tfloat_out(tfloat_result, 6);
  printf("temporal_tsample(%s, %s, %s, LINEAR): %s\n", tfloat1_out, interv1_out, tstz1_out, char_result);
  free(tfloat_result); free(char_result);

  /*****************************************************************************/
  /* Similarity functions for temporal types */
  printf("****************************************************************\n");

  /* double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_dyntimewarp_distance(tfloat1, tfloat2);
  printf("temporal_dyntimewarp_distance(%s, %s): %lf\n", tfloat1_out, tfloat2_out, float8_result);

  /* Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count); */
  matches = temporal_dyntimewarp_path(tfloat1, tfloat2, &count);
  printf("temporal_dyntimewarp_path(%s, %s): {", tfloat1_out, tfloat2_out);
  for (int i = 0; i < count; i++)
  {
    printf("(%d, %d)", matches[i].i, matches[i].j);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(matches);

  /* double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_frechet_distance(tfloat1, tfloat2);
  printf("temporal_frechet_distance(%s, %s): %lf\n", tfloat1_out, tfloat2_out, float8_result);

  /* Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count); */
  matches = temporal_frechet_path(tfloat1, tfloat2, &count);
  printf("temporal_frechet_path(%s, %s): {", tfloat1_out, tfloat2_out);
  for (int i = 0; i < count; i++)
  {
    printf("(%d, %d)", matches[i].i, matches[i].j);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(matches);

  /* double temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_hausdorff_distance(tfloat1, tfloat2);
  printf("temporal_hausdorff_distance(%s, %s): %lf\n", tfloat1_out, tfloat2_out, float8_result);

  /*****************************************************************************/
  /* Tile functions for temporal types */
  printf("****************************************************************\n");

  /* Span *temporal_time_bins(const Temporal *temp, const Interval *duration, TimestampTz origin, int *count); */
  tstzspanarray_result = temporal_time_bins(tfloat1, interv3, tstz1, &count);
  printf("temporal_time_bins(%s, %s, %s, %d): {", tfloat1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tstzspan_out(&tstzspanarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tstzspanarray_result);

  /* Temporal **temporal_time_split(const Temporal *temp, const Interval *duration, TimestampTz torigin, TimestampTz **time_bins, int *count); */
  tfloatarray_result = temporal_time_split(tfloat1, interv3, tstz1, &tstzarray_result, &count);
  printf("temporal_time_split(%s, %s, %s, &tstzarray_result, %d): {", tfloat1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    char1_result = timestamptz_out(tstzarray_result[i]);
    printf("%s: %s", char_result, char1_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
    free(char1_result);
  }
  free(tfloatarray_result);
  free(tstzarray_result);

  /* TBox *tfloat_time_boxes(const Temporal *temp, const Interval *duration, TimestampTz torigin, int *count); */
  tboxarray_result = tfloat_time_boxes(tfloat1, interv3, tstz1, &count);
  printf("tfloat_time_boxes(%s, %s, %s, %d): {", tfloat1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Span *tfloat_value_bins(const Temporal *temp, double vsize, double vorigin, int *count); */
  fspanarray_result = tfloat_value_bins(tfloat1, float8_in1, float8_in2, &count);
  printf("tfloat_value_bins(%s, %lf, %lf, %s %d): {", tfloat1_out, float8_in1, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = floatspan_out(&fspanarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(fspanarray_result);

  /* TBox *tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin, int *count); */
  tboxarray_result = tfloat_value_boxes(tfloat1, float8_in1, float8_in2, &count);
  printf("tfloat_value_boxes(%s, %lf, %lf, %s, %d): {", tfloat1_out, float8_in1, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Temporal **tfloat_value_split(const Temporal *temp, double size, double origin, double **bins, int *count); */
  tfloatarray_result = tfloat_value_split(tfloat1, float8_in1, float8_in2, &float8array_result, &count);
  printf("tfloat_value_split(%s, %lf, %lf, %s, &float8array_result, %d): {", tfloat1_out, float8_in1, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    char1_result = float8_out(float8array_result[i], 6);
    printf("%s:%s", char_result, char1_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
    free(char1_result);
  }
  free(tfloatarray_result);
  free(float8array_result);

  /* TBox *tfloat_value_time_boxes(const Temporal *temp, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, int *count); */
  tboxarray_result = tfloat_value_time_boxes(tfloat1, float8_in1, interv3, float8_in2, tstz1, &count);
  printf("tfloat_value_time_boxes(%s, %lf, %s, %lf, %s, %d): {", tfloat1_out, float8_in1, interv3_out, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Temporal **tfloat_value_time_split(const Temporal *temp, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, double **value_bins, TimestampTz **time_bins, int *count); */
  tfloatarray_result = tfloat_value_time_split(tfloat1, float8_in1, interv3, float8_in2, tstz1, &float8array_result, &tstzarray_result, &count);
  printf("tfloat_value_time_split(%s, %lf, %s, %lf, %s, &float8array_result, %d): {", tfloat1_out, float8_in1, interv3_out, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tfloat_out(tfloatarray_result[i], 6);
    char1_result = float8_out(float8array_result[i], 6);
    char2_result = timestamptz_out(tstzarray_result[i]);
    printf("%s:%s,%s", char_result, char1_result, char2_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tfloatarray_result[i]);
    free(char_result);
    free(char1_result);
    free(char2_result);
  }
  free(tfloatarray_result);
  free(float8array_result);
  free(tstzarray_result);

  /* TBox *tfloatbox_time_tiles(const TBox *box, const Interval *duration, TimestampTz torigin, int *count); */
  tboxarray_result = tfloatbox_time_tiles(tbox1, interv3, tstz1, &count);
  printf("tfloatbox_time_tiles(%s, %s, %s, %d): {", tbox1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tfloatbox_value_tiles(const TBox *box, double vsize, double vorigin, int *count); */
  tboxarray_result = tfloatbox_value_tiles(tbox1, float8_in1, float8_in2, &count);
  printf("tfloatbox_value_tiles(%s, %lf, %lf, %d): {", tbox1_out, float8_in1, float8_in2, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tfloatbox_value_time_tiles(const TBox *box, double vsize, const Interval *duration, double vorigin, TimestampTz torigin, int *count); */
  tboxarray_result = tfloatbox_value_time_tiles(tbox1, float8_in1, interv3, float8_in2, tstz1, &count);
  printf("tfloatbox_value_time_tiles(%s, %lf, %s, %lf, %s, %d): {", tbox1_out, float8_in1, interv3_out, float8_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tint_time_boxes(const Temporal *temp, const Interval *duration, TimestampTz torigin, int *count); */
  tboxarray_result = tint_time_boxes(tint1, interv3, tstz1, &count);
  printf("tint_time_boxes(%s, %s, %s, %d): {", tint1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Span *tint_value_bins(const Temporal *temp, int vsize, int vorigin, int *count); */
  ispanarray_result = tint_value_bins(tint1, int32_in1, int32_in2, &count);
  printf("tint_value_bins(%s, %s, %s, %d): {", tint1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = intspan_out(&ispanarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(ispanarray_result);

  /* TBox *tint_value_boxes(const Temporal *temp, int vsize, int vorigin, int *count); */
  tboxarray_result = tint_value_boxes(tint1, int32_in1, int32_in2, &count);
  printf("tint_value_boxes(%s, %d, %d, %d): {", tint1_out, int32_in2, int32_in2, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Temporal **tint_value_split(const Temporal *temp, int vsize, int vorigin, int **bins, int *count); */
  tintarray_result = tint_value_split(tint1, int32_in1, int32_in2, &int32array_result, &count);
  printf("tint_value_split(%s, %d, %d, &ispanarray_result, %d): {", tfloat1_out, int32_in1, int32_in2, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tint_out(tintarray_result[i]);
    printf("%s: %d", char_result, int32array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tintarray_result[i]);
    free(char_result);
  }
  free(int32array_result);
  free(tintarray_result);

  /* TBox *tint_value_time_boxes(const Temporal *temp, int vsize, const Interval *duration, int vorigin, TimestampTz torigin, int *count); */
  tboxarray_result = tint_value_time_boxes(tint1, int32_in1, interv3, int32_in2, tstz1, &count);
  printf("tint_value_time_boxes(%s, %d, %s, %d, %s, %d): {", tfloat1_out, int32_in1, interv3_out, int32_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* Temporal **tint_value_time_split(const Temporal *temp, int size, const Interval *duration, int vorigin, TimestampTz torigin, int **value_bins, TimestampTz **time_bins, int *count); */
  tintarray_result = tint_value_time_split(tint1, int32_in1, interv3, int32_in2, tstz1, &int32array_result, &tstzarray_result, &count);
  printf("tint_value_time_split(%s, %d, %s, %d, &ispanarray_result, &tstzspanarray_result, %d): {", tint1_out, int32_in1, interv3_out, int32_in2, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tint_out(tintarray_result[i]);
    char1_result = timestamptz_out(tstzarray_result[i]);
    printf("%s: %d: %s", char_result, int32array_result[i], char1_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tintarray_result[i]);
    free(char1_result);
    free(char_result);
  }
  free(int32array_result);
  free(tstzarray_result);
  free(tintarray_result);

  /* TBox *tintbox_time_tiles(const TBox *box, const Interval *duration, TimestampTz torigin, int *count); */
  tboxarray_result = tintbox_time_tiles(tintbox1, interv3, tstz1, &count);
  printf("tintbox_time_tiles(%s, %s, %s, %d): {", tintbox1_out, interv3_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tintbox_value_tiles(const TBox *box, int xsize, int xorigin, int *count); */
  tboxarray_result = tintbox_value_tiles(tintbox1, int32_in1, int32_in2, &count);
  printf("tintbox_value_tiles(%s, %d, %d, %d): {", tintbox1_out, int32_in1, int32_in2, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  /* TBox *tintbox_value_time_tiles(const TBox *box, int xsize, const Interval *duration, int xorigin, TimestampTz torigin, int *count); */
  tboxarray_result = tintbox_value_time_tiles(tintbox1, int32_in1, interv3, int32_in2, tstz1, &count);
  printf("tintbox_value_time_tiles(%s, %d, %s, %d, %s, %d): {", tintbox1_out, int32_in1, interv3_out, int32_in2, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tbox_out(&tboxarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(tboxarray_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(bool1_out);
  free(text1); free(text1_out);
  free(text_null);
  free(tstz1_out); free(tstz2_out);
  free(interv1); free(interv1_out);
  free(interv2); free(interv2_out);
  free(interv3); free(interv3_out);
  free(fset1); free(fset1_out);
  free(fspan1); free(fspan1_out);
  free(fspanset1); free(fspanset1_out);
  free(tstzset1); free(tstzset1_out);
  free(tstzset2); free(tstzset2_out);
  free(tstzspan1); free(tstzspan1_out);
  free(tstzspan2); free(tstzspan2_out);
  free(tstzspanset1); free(tstzspanset1_out);
  free(tstzspanset2); free(tstzspanset2_out);
  free(tbox1); free(tbox1_out); free(tbox1_hexwkb); free(tbox1_wkb);
  free(tbox2); free(tbox2_out);
  free(tintbox1); free(tintbox1_out);
  free(tintbox2); free(tintbox2_out);
  free(tbool1); free(tbool1_out);
  free(tbool2); free(tbool2_out);
  free(tint1); free(tint1_out); free(tint1_mfjson);
  free(tint2); free(tint2_out);
  free(tfloat1); free(tfloat1_out);
    free(tfloat1_hexwkb); free(tfloat1_wkb); free(tfloat1_mfjson);
  free(tfloatinst1); free(tfloatinst1_out);
  free(tfloatseq1); free(tfloatseq1_out);
  free(tfloat2); free(tfloat2_out);
  free(tfloatinst2); free(tfloatinst2_out);
  free(tfloatseq2); free(tfloatseq2_out);
  free(ttext1); free(ttext1_out); free(ttext1_mfjson);
  free(ttext2); free(ttext2_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
