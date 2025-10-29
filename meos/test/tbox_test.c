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
 * @brief A simple program that tests the TBox functions in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tbox_test tbox_test.c -L/usr/local/lib -lmeos
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

  int32 int32_in1 = 1;
  int32 int32_in2 = 2;
  double float8_in1 = 1;
  double float8_in2 = 2;

  // text *text_null = text_in("NULL");

  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);

  char *interv1_in = "3 days";
  char *interv2_in = "3 hours";
  Interval *interv1 = interval_in(interv1_in, -1);
  Interval *interv2 = interval_in(interv2_in, -1);
  char *interv1_out = interval_out(interv1);
  char *interv2_out = interval_out(interv2);

  char *fset1_in = "{1,3,5}";
  char *fspan1_in = "[1,3]";
  char *fspanset1_in = "{[1,3],[4,6]}";
  Set *fset1 = floatset_in(fset1_in);
  Span *fspan1 = floatspan_in(fspan1_in);
  SpanSet *fspanset1 = floatspanset_in(fspanset1_in);
  char *fset1_out = floatset_out(fset1, 6);
  char *fspan1_out = floatspan_out(fspan1, 6);
  char *fspanset1_out = floatspanset_out(fspanset1, 6);

  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);

  char *tbox1_in = "TBOXFLOAT XT([1, 3],[2001-01-01, 2001-01-03])";
  char *tbox2_in = "TBOXFLOAT XT([2, 4],[2001-01-02, 2001-01-04])";
  TBox *tbox1 = tbox_in(tbox1_in);
  TBox *tbox2 = tbox_in(tbox2_in);
  char *tbox1_out = tbox_out(tbox1, 6);
  char *tbox2_out = tbox_out(tbox2, 6);

  char *tintbox1_in = "TBOXINT XT([1, 3],[2001-01-01, 2001-01-03])";
  TBox *tintbox1 = tbox_in(tintbox1_in);
  char *tintbox1_out = tbox_out(tintbox1, 6);

  /* Create the result types for the functions of the API */

  bool bool_result;
  bool bool1_result;
  int32 int32_result;
  // uint32 uint32_result;
  // uint64 uint64_result;
  double float8_result;
  TimestampTz tstz_result;
  char *char_result;
  // uint8_t *binchar_result;
  size_t size;

  TBox *tbox_result;
  Span *ispan_result;
  Span *fspan_result;
  Span *tstzspan_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Tbox type *\n");
  printf("****************************************************************\n");

  /*****************************************************************************
   * Input and output functions for box types
   *****************************************************************************/

  /* char *tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size); */
  char_result = tbox_as_hexwkb(tbox1, 1, &size);
  printf("tbox_as_hexwkb(%s, 1, %ld): %s\n", tbox1_out, size, char_result);
  free(char_result);

  // /* uint8_t *tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out); */
  // binchar_result = tbox_as_wkb(tbox1, 1, &size);
  // printf("tbox_as_wkb(%s, 1, %ld): %s\n", tbox1_out, size, char_result);
  // free(char_result);

  // /* TBox *tbox_from_hexwkb(const char *hexwkb); */
  // tbox_result = tbox_from_hexwkb(const char *hexwkb);
  // char_result = tbox_out(tbox_result, 6);
  // printf("tbox_from_hexwkb(%s, %s): %s\n", xxx, char_result);
  // free(char_result);
  
  // /* TBox *tbox_from_wkb(const uint8_t *wkb, size_t size); */
  // tbox_result = tbox_from_wkb(const uint8_t *wkb, &size);
  // char_result = tbox_out(tbox_result, 6);
  // printf("tbox_from_wkb(%s, %s): %s\n", xxx, char_result);
  // free(char_result);

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

  printf("****************************************************************\n");

  /* Clean up */

  free(tstz1_out);

  free(interv1); free(interv1_out);
  free(interv2); free(interv2_out);

  free(fset1); free(fset1_out);
  free(fspan1); free(fspan1_out);
  free(fspanset1); free(fspanset1_out);

  free(tstzspan1); free(tstzspan1_out);
  free(tbox1); free(tbox1_out);
  free(tbox2); free(tbox2_out);

  free(tintbox1); free(tintbox1_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
