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
 * @brief A simple program that tests the text functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o text_test text_test.c -L/usr/local/lib -lmeos
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
  int32 int32_in1 = 32;
  int64 int64_in1 = 64;
  text *text1 = text_in("abcdef");
  text *text2 = text_in("ghijkl");
  char *text1_out = text_out(text1);
  char *text2_out = text_out(text2);

  /* Create the result types for the functions of the API */
  bool bool_result;
  int32 int32_result;
  uint32 uint32_result;
  uint64 uint64_result;
  char *char_result;
  text *text_result;
  
  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Text *\n");
  printf("****************************************************************\n");

  /* uint32 char_hash(char c); */
  uint32_result = char_hash('c');
  printf("char_hash('c'): %u\n", uint32_result);

  /* uint64 char_hash_extended(char c, uint64 seed); */
  uint64_result = char_hash_extended('c', 1);
  printf("char_hash_extended('c', 1): %lu\n", uint64_result);

  /* text *cstring_to_text(const char *str); */
  text_result = cstring_to_text("azerty");
  char_result = text_out(text_result);
  printf("cstring_to_text(\"azerty\"): %s\n", char_result);
  free(text_result); free(char_result);

  /* text *icu_unicode_version(void); */
  text_result = icu_unicode_version();
  char_result = text_result ? text_out(text_result) : "NULL";
  printf("icu_unicode_version(): %s\n", char_result);
  if (text_result)
  {
    free(text_result); free(char_result);
  }

  /* text *int32_to_bin(int32 num); */
  text_result = int32_to_bin(int32_in1);
  char_result = text_out(text_result);
  printf("int32_to_bin(%d): %s\n", int32_in1, char_result);
  free(text_result); free(char_result);

  /* text *int32_to_hex(int32 num); */
  text_result = int32_to_hex(int32_in1);
  char_result = text_out(text_result);
  printf("int32_to_hex(%d): %s\n", int32_in1, char_result);
  free(text_result); free(char_result);

  /* text *int32_to_oct(int32 num); */
  text_result = int32_to_oct(int32_in1);
  char_result = text_out(text_result);
  printf("int32_to_oct(%d): %s\n", int32_in1, char_result);
  free(text_result); free(char_result);

  /* text *int64_to_bin(int64 num); */
  text_result = int64_to_bin(int64_in1);
  char_result = text_out(text_result);
  printf("int64_to_bin(%ld): %s\n", int64_in1, char_result);
  free(text_result); free(char_result);

  /* text *int64_to_hex(int64 num); */
  text_result = int64_to_hex(int64_in1);
  char_result = text_out(text_result);
  printf("int64_to_hex(%ld): %s\n", int64_in1, char_result);
  free(text_result); free(char_result);

  /* text *int64_to_oct(int64 num); */
  text_result = int64_to_oct(int64_in1);
  char_result = text_out(text_result);
  printf("int64_to_oct(%ld): %s\n", int64_in1, char_result);
  free(text_result); free(char_result);

  /* char *text_to_cstring(const text *txt); */
  char_result = text_to_cstring(text1);
  printf("text2cstring(%s): %s\n", text1_out, char_result);
  free(char_result);

  /* text *text_cat(const text *txt1, const text *txt2); */
  text_result = text_cat(text1, text2);
  char_result = text_out(text_result);
  printf("text_cat(%s, %s): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* int text_cmp(const text *txt1, const text *txt2, Oid collid); */
  uint32_result = text_cmp(text1, text2, 100);
  printf("text_cmp(%s, %s, 100): %d\n", text1_out, text2_out, uint32_result);

  /* text *text_concat(text **textarr, int count); */
  text *text_concat(text **textarr, int count);
  text *textarr[2] = {text1, text2};
  text_result = text_concat((text **) textarr, 2);
  char_result = text_out(text_result);
  printf("text_concat([%s, %s], 2): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* text *text_concat_ws(text **textarr, int count, const text *sep); */
  text *text_concat_ws(text **textarr, int count, const text *sep);
  text *sep = text_in(", ");
  text_result = text_concat_ws(textarr, 2, sep);
  char_result = text_out(text_result);
  printf("text_concat_ws([%s, %s], 2, \", \"): %s\n", text1_out, text2_out, char_result);
  free(sep); free(char_result); free(text_result);

  /* text *text_copy(const text *txt); */
  text_result = text_copy(text1);
  char_result = text_out(text_result);
  printf("text_copy(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result); 

  /* bool text_eq(const text *txt1, const text *txt2); */
  bool_result = text_eq(text1, text2);
  printf("text_eq(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* bool text_ge(const text *txt1, const text *txt2); */
  bool_result = text_ge(text1, text2);
  printf("text_ge(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* bool text_gt(const text *txt1, const text *txt2); */
  bool_result = text_gt(text1, text2);
  printf("text_gt(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* uint32 text_hash(const text *txt, Oid collid); */
  uint32_result = text_hash(text1, 100);
  printf("text_hash(%s, 100): %d\n", text1_out, uint32_result);

  /* uint64 text_hash_extended(const text *txt, uint64 seed, Oid collid); */
  uint64_result = text_hash_extended(text1, 1, 100);
  printf("text_hash_extended(%s, 1, 100): %lud\n", text1_out, uint64_result);

  /* text *text_in(const char *str); */
  text_result = text_in("azerty");
  char_result = text_out(text_result);
  printf("text_in(\"azerty\"): %s\n", char_result);
  free(text_result); free(char_result);

  /* text *text_initcap(const text *txt); */
  text_result = text_initcap(text1);
  char_result = text_out(text_result);
  printf("text_initcap(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *text_larger(const text *txt1, const text *txt2); */
  text_result = text_larger(text1, text2);
  char_result = text_out(text_result);
  printf("text_larger(%s, %s): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* bool text_le(const text *txt1, const text *txt2); */
  bool_result = text_le(text1, text2);
  printf("text_le(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* text *text_left(const text *txt, int n); */
  text_result = text_left(text1, 3);
  char_result = text_out(text_result);
  printf("text_left(%s, 3): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* int32 text_len(const text *txt); */
  int32_result = text_len(text1);
  printf("text_len(%s): %d\n", text1_out, int32_result);

  /* text *text_lower(const text *txt); */
  text_result = text_lower(text1);
  char_result = text_out(text_result);
  printf("text_lower(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* bool text_lt(const text *txt1, const text *txt2); */
  bool_result = text_lt(text1, text2);
  printf("text_lt(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* bool text_ne(const text *txt1, const text *txt2); */
  bool_result = text_ne(text1, text2);
  printf("text_ne(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* int32 text_octetlen(const text *txt); */
  int32_result = text_octetlen(text1);
  printf("text_octetlen(%s): %d\n", text1_out, int32_result);

  /* char *text_out(const text *txt); */
  char_result = text_out(text1);
  printf("text_out(%s): %s\n", text1_out, char_result);
  free(char_result);

  /* text *text_overlay(const text *txt1, const text *txt2, int from, int count); */
  text_result = text_overlay(text1, text2, 2, 2);
  char_result = text_out(text_result);
  printf("text_overlay(%s, %s, 2, 2): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* text *text_overlay_no_len(const text *txt1, const text *txt2, int from); */
  text_result = text_overlay_no_len(text1, text2, 2);
  char_result = text_out(text_result);
  printf("text_overlay_no_len(%s, %s, 2): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* bool text_pattern_ge(const text *txt1, const text *txt2); */
  bool_result = text_pattern_ge(text1, text2);
  printf("text_pattern_ge(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* bool text_pattern_gt(const text *txt1, const text *txt2); */
  bool_result = text_pattern_gt(text1, text2);
  printf("text_pattern_gt(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* bool text_pattern_le(const text *txt1, const text *txt2); */
  bool_result = text_pattern_le(text1, text2);
  printf("text_pattern_le(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /*  bool text_pattern_lt(const text *txt1, const text *txt2); */
  bool_result = text_pattern_lt(text1, text2);
  printf("text_pattern_lt(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* int32 text_pos(const text *txt, const text *search); */
  text *search = text_in("c");
  int32_result = text_pos(text1, search);
  printf("text_pos(%s, \"c\"): %d\n", text1_out, int32_result);
  free(search); 

  /* text *text_replace(const text *txt, const text *from, const text *to); */
  text *from = text_in("c");
  text *to = text_in("X");
  char *from_out = text_to_cstring(from);
  char *to_out = text_to_cstring(to);
  text_result = text_replace(text1, from, to);
  char_result = text_out(text_result);
  printf("text_replace(%s, %s, %s): %s\n", text1_out, from_out, to_out, char_result);
  free(from); free(from_out); free(to); free(to_out);
  free(text_result); free(char_result);

  /* text *text_reverse(const text *txt); */
  text_result = text_reverse(text1);
  char_result = text_out(text_result);
  printf("text_reverse(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *text_right(const text *txt, int n); */
  text_result = text_right(text1, 3);
  char_result = text_out(text_result);
  printf("text_right(%s, 3): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *text_smaller(const text *txt1, const text *txt2); */
  text_result = text_smaller(text1, text2);
  char_result = text_out(text_result);
  printf("text_smaller(%s, %s): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  /* text *text_split_part(const text *txt, const text *sep, int fldnum); */
  text *text_sep = text_in(", ");
  text_result = text_split_part(text1, text_sep, 1);
  char_result = text_out(text_result);
  printf("text_split_part(%s, \", \", 1): %s\n", text1_out, char_result);
  free(text_sep); free(text_result); free(char_result);

  /* bool text_starts_with(const text *txt1, const text *txt2); */
  bool_result = text_starts_with(text1, text2);
  printf("text_starts_with(%s, %s): %c\n", text1_out, text2_out, bool_result ? 't' : 'n');

  /* text *text_substr(const text *txt, int32 start, int32 length); */
  text_result = text_substr(text1, 3, 2);
  char_result = text_out(text_result);
  printf("text_substr(%s, 3, 2): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *text_substr_no_len(const text *txt, int32 start); */
  text_result = text_substr_no_len(text1, 3);
  char_result = text_out(text_result);
  printf("text_substr_no_len(%s, 3): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *text_upper(const text *txt); */
  text_result = text_upper(text1);
  char_result = text_out(text_result);
  printf("text_upper(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  /* text *textcat_text_text(const text *txt1, const text *txt2); */
  text_result = textcat_text_text(text1, text2);
  char_result = text_out(text_result);
  printf("textcat_text_text(%s, %s): %s\n", text1_out, text2_out, char_result);
  free(text_result); free(char_result);

  // /* bool unicode_assigned(const text *txt); */
  // bool_result = unicode_assigned(text1);
  // printf("unicode_assigned(%s): %c\n", text1_out, bool_result ? 't' : 'n');

  // /* bool unicode_is_normalized(const text *txt, const text *fmt); */
  // text *fmt = text_in("NFC");
  // bool_result = unicode_is_normalized(text1, fmt);
  // printf("unicode_is_normalized(%s, \"NFC\"): %c\n", text1_out, bool_result ? 't' : 'n');

  // /* text *unicode_normalize_func(const text *txt, const text *fmt); */
  // text_result = unicode_normalize_func(text1, fmt);
  // char_result = text_out(text_result);
  // printf("unicode_normalize_func(%s, \"NFC\"): %s\n", text1_out, char_result);
  // free(fmt); free(text_result); free(char_result);

  /* text *unicode_version(void); */
  text_result = unicode_version();
  char_result = text_out(text_result);
  printf("unicode_version(): %s\n", char_result);
  free(text_result); free(char_result);

  /* text *unistr(const text *txt); */
  text_result = unistr(text1);
  char_result = text_out(text_result);
  printf("unistr(%s): %s\n", text1_out, char_result);
  free(text_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(b1_out);
  free(text1); free(text2);
  free(text1_out); free(text2_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
