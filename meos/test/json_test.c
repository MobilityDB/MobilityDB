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
 * @brief A simple program that tests the JSONB functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_test json_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_text.h>
#include <pg_json.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */
  text *js1 = json_in("{\"a\":1, \"b\":[1,2]}");
  text *js2 = json_in("{\"a\":5, \"b\":[\"c\",3]}");
  Jsonb *jb1 = jsonb_in("{\"a\":1, \"b\":[1,2]}");
  Jsonb *jb2 = jsonb_in("{\"a\":5, \"b\":[\"c\",3]}");
  char *js1_out = json_out(js1);
  char *js2_out = json_out(js2);
  char *jb1_out = jsonb_out(jb1);
  char *jb2_out = jsonb_out(jb2);

  /* Create result types for the functions of the API */
  bool bool_result;
  int32 int32_result;
  uint32 uint32_result;
  uint64 uint64_result;
  char *char_result;
  text *text_result;
  Jsonb *jsonb_result;
  
  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* JSON *\n");
  printf("****************************************************************\n");

  /* int json_array_length(const text *json) */
  text *array = text_in("[\"a\", \"b\", \"c\"]");
  int32_result = json_array_length(array);
  printf("json_array_length(%s): %d\n", js1_out, int32_result);
  free(array);

  /* Jsonb **json_array_elements(const text *txt, int *count); */
  int count;
  text *txtarray = json_in("[\"a\", \"b\", \"c\"]");
  text **textarray_result = json_array_elements(txtarray, &count);
  printf("json_array_elements([\"a\", \"b\", \"c\"]): {");
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
    free(textarray_result[i]);
  }
  free(txtarray); free(textarray_result);

  /* text **json_array_elements_text(const text *txt, int *count); */
  txtarray = json_in("[\"a\", \"b\", \"c\"]");
  textarray_result = json_array_elements_text(txtarray, &count);
  printf("jsonb_array_elements_text([\"a\", \"b\", \"c\"]): {");
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
  free(txtarray); free(textarray_result);

  /* text **json_each(const text *js, text **values, int *count); */
  text *values[2];
  textarray_result = json_each(js1, values, &count);
  printf("json_each(%s, values, count): {", jb1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s:", char_result);
    free(char_result);
    char_result = json_out(values[i]);
    printf("%s", char_result);
    free(char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(values[i]);
    free(textarray_result[i]);
  }
  free(textarray_result);
  
  /* text **json_each_text(const text *js, text **values, int *count); */
  textarray_result = json_each_text(js1, values, &count);
  printf("json_each_text(%s, values, count): {", jb1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s:", char_result);
    free(char_result);
    char_result = text_out(values[i]);
    printf("%s", char_result);
    free(char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(textarray_result[i]);
    free(values[i]);
  }
  free(textarray_result);

  /* text *json_in(const char *str); */
  text_result = json_in("{\"a\":1, \"b\":[1,2]}");
  char_result = text_out(text_result);
  printf("json_in('{\"a\":1, \"b\":[1,2]}'): %s\n", char_result);
  free(text_result); free(char_result);

  /* text *json_object(text **keys_vals, int count); */
  text *keys_vals[4];
  keys_vals[0] = text_in("a");
  keys_vals[1] = text_in("1");
  keys_vals[2] = text_in("b");
  keys_vals[3] = text_in("X");
  text_result = json_object((text **) keys_vals, 4);
  char_result = text_out(text_result);
  printf("json_object({\"a\",\"1\",\"b\",\"X\"}, 4): %s\n", char_result);
  free(keys_vals[0]); free(keys_vals[1]);
  free(keys_vals[2]); free(keys_vals[3]);
  free(text_result); free(char_result);

  /* text **json_object_keys(const text *json, int *count); */
  textarray_result = json_object_keys(js1, &count);
  printf("json_object_keys(%s): {", js1_out);
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

  /* text *json_object_two_arg(text **keys, text **values, int count); */
  text *keys[2];
  text *vals[2];
  keys[0] = text_in("a");
  vals[0] = text_in("1");
  keys[1] = text_in("b");
  vals[1] = text_in("X");
  text_result = json_object_two_arg((text **) keys, (text **) vals, 2);
  char_result = text_out(text_result);
  printf("jsonb_object_two_arg({\"a\",\"b\"}, {\"1\",\"X\"}, 2): %s\n", char_result);
  free(keys[0]); free(keys[1]);
  free(vals[0]); free(vals[1]);
  free(text_result); free(char_result);

  /* char *json_out(const text *json); */
  char_result = json_out(js1);
  printf("json_out(%s): %s\n", js1_out, char_result);
  free(char_result);

  /* text *json_strip_nulls(const text *json, bool strip_in_arrays); */
  text_result = json_strip_nulls(js1, true);
  char_result = text_out(text_result);
  printf("json_strip_nulls(%s, true): %s\n", js1_out, char_result);
  free(text_result); free(char_result);

  /* text *json_typeof(const text *json); */
  text_result = json_typeof(js1);
  char_result = text_out(text_result);
  printf("json_typeof(%s): %s\n", js1_out, char_result);
  free(text_result); free(char_result);

  /* Jsonb **jsonb_array_elements(const Jsonb *jb, int *count); */
  Jsonb *jbarray = jsonb_in("[\"a\", \"b\", \"c\"]");
  Jsonb **jbarray_result = jsonb_array_elements(jbarray, &count);
  printf("jsonb_array_elements([\"a\", \"b\", \"c\"]): {");
  for (int i = 0; i < count; i++)
  {
    char_result = jsonb_out(jbarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
    free(jbarray_result[i]);
  }
  free(jbarray);
  free(jbarray_result);

  /* text **jsonb_array_elements_text(const Jsonb *jb, int *count); */
  jbarray = jsonb_in("[\"a\", \"b\", \"c\"]");
  textarray_result = jsonb_array_elements_text(jbarray, &count);
  printf("jsonb_array_elements_text([\"a\", \"b\", \"c\"]): {");
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
    free(textarray_result[i]);
  }
  free(jbarray);
  free(textarray_result);

  /* int jsonb_cmp(const Jsonb *jb1, const Jsonb *jb2); */
  int32_result = jsonb_cmp(jb1, jb2);
  printf("jsonb_cmp(%s): %d\n", jb1_out, int32_result);

  /* Jsonb *jsonb_concat(const Jsonb *jb1, const Jsonb *jb2); */
  jsonb_result = jsonb_concat(jb1, jb2);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_concat(%s, %s): %s\n", jb1_out, jb2_out, char_result);
  free(jsonb_result); free(char_result);

  /* bool jsonb_contained(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_contained(jb1, jb2);
  printf("jsonb_contains(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* bool jsonb_contains(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_contains(jb1, jb2);
  printf("jsonb_contains(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* Jsonb *jsonb_copy(const Jsonb *jb); */
  jsonb_result = jsonb_copy(jb1);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_copy(%s): %s\n", jb1_out, char_result);
  free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_delete(const Jsonb *jb, const text *key); */
  text *key = text_in("a");
  jsonb_result = jsonb_delete(jb1, key);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_delete(%s, \"a\"): %s\n", jb1_out, char_result);
  free(key); free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_delete_array(const Jsonb *jb, text **keys_elems, int keys_len); */
  text *keys_elems[2];
  keys_elems[0] = text_in("a");
  keys_elems[1] = text_in("b");
  jsonb_result = jsonb_delete_array(jb1, (text **) keys_elems, 2);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_delete_array(%s, {\"a\",\"b\"}, 2): %s\n", jb1_out, char_result);
  free(keys_elems[0]); free(keys_elems[1]);
  free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_delete_idx(const Jsonb *in, int idx); */
  jbarray = jsonb_in("[\"a\", \"b\", \"c\"]");
  jsonb_result = jsonb_delete_idx(jbarray, 1);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_delete_idx(%s, 1): %s\n", jb1_out, char_result);
  free(jbarray); free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_delete_path(const Jsonb *jb, text **path_elems, int path_len); */
  text *path = text_in("a");
  jsonb_result = jsonb_delete_path(jb1, &path, 1);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_delete_path(%s, \"a\"): %s\n", jb1_out, char_result);
  free(path); free(jsonb_result); free(char_result);

  /* text **jsonb_each(const Jsonb *jb, Jsonb **values, int *count); */
  Jsonb *jsonbarr[2];
  textarray_result = jsonb_each(jb1, jsonbarr, &count);
  printf("jsonb_each(%s, values, count): {", jb1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s:", char_result);
    free(char_result);
    char_result = jsonb_out(jsonbarr[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
    free(jsonbarr[i]);
    free(textarray_result[i]);
  }
  free(textarray_result);
  
  /* text **jsonb_each_text(const Jsonb *jb, text **values, int *count); */
  textarray_result = jsonb_each_text(jb1, values, &count);
  printf("jsonb_each_text(%s, values, count): {", jb1_out);
  for (int i = 0; i < count; i++)
  {
    char_result = text_out(textarray_result[i]);
    printf("%s:", char_result);
    free(char_result);
    char_result = text_out(values[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
    free(textarray_result[i]);
    free(values[i]);
  }
  free(textarray_result);
  
  /* bool jsonb_eq(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_eq(jb1, jb2);
  printf("jsonb_eq(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* bool jsonb_exists(const Jsonb *jb, const text *key); */
  key = text_in("a");
  bool_result = jsonb_exists(jb1, key);
  printf("jsonb_exists(%s, \"a\"): %c\n", jb1_out, bool_result ? 't' : 'f');
  free(key);

  /* Jsonb *jsonb_extract_path(const Jsonb *jb, text **path_elems, int path_len); */
  text *path_elems[2];
  path_elems[0] = text_in("b");
  path_elems[1] = text_in("1");
  jsonb_result = jsonb_extract_path(jb1, (text **) path_elems, 2);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_extract_path(%s, {\"b\",1}, 2): %s\n", jb1_out, char_result);
  free(path_elems[0]); free(path_elems[1]);
  free(jsonb_result); free(char_result);
  
  /* text *jsonb_extract_path_text(const Jsonb *jb, text **path_elems, int path_len); */
  path_elems[0] = text_in("b");
  path_elems[1] = text_in("1");
  text_result = jsonb_extract_path_text(jb1, (text **) path_elems, 2);
  char_result = text_out(text_result);
  printf("jsonb_extract_path_text(%s, {\"b\",1}, 2): %s\n", jb1_out, char_result);
  free(path_elems[0]); free(path_elems[1]);
  free(text_result); free(char_result);

  /* Jsonb *jsonb_from_text(text *txt, bool unique_keys); */
  jsonb_result = jsonb_from_text(js1, true);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_from_text(%s, true): %s\n", js1_out, char_result);
  free(jsonb_result); free(char_result);

  /* uint32 jsonb_hash(const Jsonb *jb); */
  uint32_result = jsonb_hash(jb1);
  printf("jsonb_hash(%s): %u\n", jb1_out, uint32_result);

  /* uint64 jsonb_hash_extended(Jsonb *jb, uint64 seed); */
  uint64_result = jsonb_hash_extended(jb1, 1);
  printf("jsonb_hash_extended(%s, 1): %lu\n", jb1_out, uint64_result);

  /* Jsonb *jsonb_in(char *str);extern bool jsonb_gt(const Jsonb *jb1, const Jsonb *jb2); */
  jsonb_result = jsonb_in("{}");
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_in(\"{}\"): %s\n", char_result);
  free(jsonb_result); free(char_result);

  /* bool jsonb_ge(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_ge(jb1, jb2);
  printf("jsonb_ge(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* bool jsonb_gt(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_gt(jb1, jb2);
  printf("jsonb_gt(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* Jsonb *jsonb_insert(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool after); */
  path_elems[0] = text_in("b");
  path_elems[1] = text_in("1");
  Jsonb *newjb = jsonb_in("\"X\"");
  jsonb_result = jsonb_insert(jb1, (text **) path_elems, 2, newjb, true);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_insert(%s, {\"b\",1}, 2, \"X\", true): %s\n", jb1_out, char_result);
  free(path_elems[0]); free(path_elems[1]);
  free(newjb); free(jsonb_result); free(char_result);

  /* bool jsonb_ne(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_ne(jb1, jb2);
  printf("jsonb_ne(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* bool jsonb_lt(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_lt(jb1, jb2);
  printf("jsonb_lt(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* bool jsonb_le(const Jsonb *jb1, const Jsonb *jb2); */
  bool_result = jsonb_le(jb1, jb2);
  printf("jsonb_le(%s, %s): %c\n", jb1_out, jb2_out, bool_result ? 't' : 'f');

  /* Jsonb *jsonb_object(text **keys_vals, int count); */
  keys_vals[0] = text_in("a");
  keys_vals[1] = text_in("1");
  keys_vals[2] = text_in("b");
  keys_vals[3] = text_in("X");
  jsonb_result = jsonb_object((text **) keys_vals, 4);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_object({\"a\",\"1\",\"b\",\"X\"}, 4): %s\n", char_result);
  free(keys_vals[0]); free(keys_vals[1]);
  free(keys_vals[2]); free(keys_vals[3]);
  free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_object_field(const Jsonb *jb, const text *key); */
  key = text_in("a");
  jsonb_result = jsonb_object_field(jb1, key);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_object_field(%s, \"a\"): %s\n", jb1_out, char_result);
  free(key); free(jsonb_result); free(char_result);

  /* text *jsonb_object_field_text(const Jsonb *jb, const text *key); */
  key = text_in("a");
  text_result = jsonb_object_field_text(jb1, key);
  char_result = text_out(text_result);
  printf("jsonb_object_field_text(%s, \"a\"): %s\n", jb1_out, char_result);
  free(key); free(text_result); free(char_result);

  /* text **jsonb_object_keys(const Jsonb *jb, int *count); */
  textarray_result = jsonb_object_keys(jb1, &count);
  printf("jsonb_object_keys(%s): {", jb1_out);
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

  /* Jsonb *jsonb_object_two_arg(text **keys, text **values, int count); */
  keys[0] = text_in("a");
  vals[0] = text_in("1");
  keys[1] = text_in("b");
  vals[1] = text_in("X");
  jsonb_result = jsonb_object_two_arg((text **) keys, (text **) vals, 2);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_object_two_arg({\"a\",\"b\"}, {\"1\",\"X\"}, 2): %s\n", char_result);
  free(keys[0]); free(keys[1]);
  free(vals[0]); free(vals[1]);
  free(jsonb_result); free(char_result);

  /* char *jsonb_out(Jsonb *jb); */
  newjb = jsonb_in("{}");
  char_result = jsonb_out(newjb);
  printf("jsonb_out(\"{}\"): %s\n",  char_result);
  free(newjb); free(char_result);
  
  /* text *jsonb_pretty(const Jsonb *jb); */
  text_result = jsonb_pretty(jb1);
  char_result = text_out(text_result);
  printf("jsonb_pretty(%s): %s\n", jb1_out, char_result);
  free(text_result); free(char_result);

  /* Jsonb *jsonb_set(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool create); */
  path_elems[0] = text_in("b");
  path_elems[1] = text_in("2");
  newjb = jsonb_in("\"X\"");
  jsonb_result = jsonb_set(jb1, (text **) path_elems, 2, newjb, true);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_set(%s, {\"b\",2}, 2): %s\n", jb1_out, char_result);
  free(path_elems[0]); free(path_elems[1]);
  free(newjb); free(jsonb_result); free(char_result);

  /* Jsonb *jsonb_set_lax(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool create, const text *handle_null); */
  path_elems[0] = text_in("b");
  path_elems[1] = text_in("1");
  text *handle_null = text_in("use_json_null");
  jsonb_result = jsonb_set_lax(jb1, (text **) path_elems, 2, NULL, true, handle_null);
  char_result = jsonb_out(jsonb_result);
  printf("jsonb_set_lax(%s, {\"b\",1}, 2, NULL, true, \"use_json_null\"): %s\n", jb1_out, char_result);
  free(path_elems[0]); free(path_elems[1]);
  free(handle_null); free(jsonb_result); free(char_result);
  
  /* Jsonb *jsonb_strip_nulls(const Jsonb *jb, bool strip_in_arrays); */
  jsonb_result = jsonb_strip_nulls(jb1, true);
  char_result = jsonb_out(jsonb_result);
  printf("json_strip_nulls(%s, true): %s\n", js1_out, char_result);
  free(jsonb_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(js1); free(js2);
  free(jb1); free(jb2);
  free(js1_out); free(js2_out);
  free(jb1_out); free(jb2_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
