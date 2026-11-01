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
 * @brief A program that tests how the temporal JSON and JSONB set functions of
 * the MEOS API report an invalid argument under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a
 * binding calling it with a null pointer or a value of the wrong type receives
 * an error it can raise in its host language. An assertion cannot carry that
 * contract: it is compiled out under NDEBUG, which leaves the release build a
 * binding links against with no check at all.
 *
 * The program verifies that #tjsonb_to_ttext, #ttext_to_tjsonb,
 * #tjsonb_pretty, #tjsonb_strip_nulls, #tjson_strip_nulls, #jsonbset_pretty,
 * #jsonbset_strip_nulls, #jsonbset_to_alphanumset, #jsonbset_to_textset,
 * #textset_to_jsonbset, #tjsonb_to_tbool,
 * #tjsonb_to_tint, #tjsonb_to_tfloat and #tjsonb_to_ttext_key report a null
 * argument and an argument of another type by returning NULL and setting
 * #meos_errno, that #jsonbset_to_alphanumset reports a result base type that
 * is not alphanumeric the same way, that #null_handle_type_from_string reports
 * a null string by answering NULL_INVALID and setting #meos_errno, and that a
 * valid call still answers with no error left behind.
 *
 * It verifies the same of the field, element and path extractions, one public
 * function per operation as PostgreSQL's jsonb_object_field and
 * jsonb_object_field_text: #jsonbset_object_field,
 * #jsonbset_object_field_text, #jsonbset_array_element,
 * #jsonbset_array_element_text, #jsonbset_extract_path,
 * #jsonbset_extract_path_text, #tjson_object_field, #tjsonb_object_field,
 * #tjsonb_object_field_text, #tjsonb_array_element,
 * #tjsonb_array_element_text, #tjsonb_extract_path and
 * #tjsonb_extract_path_text, and that a valid call answers the value the
 * PostgreSQL suite answers for the operator over the same input: the JSONB form
 * keeps a string quoted, the text form unquotes it.
 *
 * It verifies that the replacements of the value at a path, two public
 * functions per type as PostgreSQL's jsonb_set and jsonb_set_lax, report a
 * null value given to #jsonbset_set and #tjsonb_set and a null treatment given
 * to #jsonbset_set_lax and #tjsonb_set_lax, and that the lax form treats a null
 * value as its treatment states.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_validity_test json_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>

/**
 * @brief Check that a call answered NULL with the expected error, then clear it
 */
static void
expect_error(void *res, int code, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == code);
  meos_errno_reset();
}

/**
 * @brief Check that a call answered a value and left no error behind
 */
static void
expect_value(void *res, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res != NULL);
  assert(meos_errno() == 0);
  free(res);
}

/**
 * @brief Check that a call answered the expected text and left no error
 * behind, then free the text
 * @details The output comparison of timezone_switch_test.c's check(), with
 * the errno contract of #expect_value
 */
static void
expect_text(char *out, const char *want, const char *call)
{
  printf("%s: %s, errno %d\n", call, out ? out : "NULL", meos_errno());
  assert(out != NULL);
  assert(strcmp(out, want) == 0);
  assert(meos_errno() == 0);
  free(out);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tjsonb = tjsonb_in("\"{\\\"a\\\": 1, \\\"b\\\": null}\"@2001-01-01");
  Temporal *tjson = ttext_in("\"{\\\"a\\\": 1, \\\"b\\\": null}\"@2001-01-01");
  Set *jsonbset = jsonbset_in("{\"{\\\"a\\\": 1, \\\"b\\\": null}\"}");
  Temporal *tjsonbb = tjsonb_in("\"{\\\"c\\\": true}\"@2001-01-01");
  Temporal *tint = tint_in("1@2001-01-01");
  Set *intset = intset_in("{1, 2}");
  assert(tjsonb); assert(tjson); assert(jsonbset); assert(tjsonbb);
  assert(tint); assert(intset);
  meos_errno_reset();

  /* A null argument is reported rather than dereferenced */
  expect_error(tjsonb_to_ttext(NULL), MEOS_ERR_INVALID_ARG,
    "tjsonb_to_ttext(NULL)");
  expect_error(ttext_to_tjsonb(NULL), MEOS_ERR_INVALID_ARG,
    "ttext_to_tjsonb(NULL)");
  expect_error(tjsonb_pretty(NULL), MEOS_ERR_INVALID_ARG,
    "tjsonb_pretty(NULL)");
  expect_error(tjsonb_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "tjsonb_strip_nulls(NULL, true)");
  expect_error(tjson_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "tjson_strip_nulls(NULL, true)");
  expect_error(jsonbset_pretty(NULL), MEOS_ERR_INVALID_ARG,
    "jsonbset_pretty(NULL)");
  expect_error(jsonbset_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "jsonbset_strip_nulls(NULL, true)");
  expect_error(jsonbset_to_textset(NULL), MEOS_ERR_INVALID_ARG,
    "jsonbset_to_textset(NULL)");
  expect_error(textset_to_jsonbset(NULL), MEOS_ERR_INVALID_ARG,
    "textset_to_jsonbset(NULL)");
  expect_error(jsonbset_to_alphanumset(NULL, "a", T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "jsonbset_to_alphanumset(NULL, a, int4)");
  expect_error(jsonbset_to_alphanumset(jsonbset, NULL, T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "jsonbset_to_alphanumset(jsonbset, NULL, int4)");
  expect_error(tjsonb_to_tbool(NULL, "c", NULL_RETURN), MEOS_ERR_INVALID_ARG,
    "tjsonb_to_tbool(NULL, c)");
  expect_error(tjsonb_to_tint(NULL, "a", NULL_RETURN), MEOS_ERR_INVALID_ARG,
    "tjsonb_to_tint(NULL, a)");
  expect_error(tjsonb_to_tfloat(NULL, "a", LINEAR, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "tjsonb_to_tfloat(NULL, a)");
  expect_error(tjsonb_to_ttext_key(NULL, "a", NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "tjsonb_to_ttext_key(NULL, a)");
  expect_error(tjsonb_to_tbool(tjsonbb, NULL, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "tjsonb_to_tbool(tjsonb, NULL)");
  expect_error(tjsonb_to_tint(tjsonb, NULL, NULL_RETURN), MEOS_ERR_INVALID_ARG,
    "tjsonb_to_tint(tjsonb, NULL)");
  expect_error(tjsonb_to_tfloat(tjsonb, NULL, LINEAR, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "tjsonb_to_tfloat(tjsonb, NULL)");
  expect_error(tjsonb_to_ttext_key(tjsonb, NULL, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "tjsonb_to_ttext_key(tjsonb, NULL)");
  /* A null string is reported rather than compared with the names */
  nullHandleType nh = null_handle_type_from_string(NULL);
  printf("null_handle_type_from_string(NULL): %d, errno %d\n", (int) nh,
    meos_errno());
  assert(nh == NULL_INVALID);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* An argument of another type is reported as a type error: each function
   * reads the values of its argument as JSON or text, so accepting an integer
   * would read one type's bytes as another's */
  expect_error(tjsonb_to_ttext(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_to_ttext(1@2001-01-01)");
  expect_error(ttext_to_tjsonb(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "ttext_to_tjsonb(1@2001-01-01)");
  expect_error(tjsonb_pretty(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_pretty(1@2001-01-01)");
  expect_error(tjsonb_strip_nulls(tint, true), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_strip_nulls(1@2001-01-01, true)");
  expect_error(tjson_strip_nulls(tint, true), MEOS_ERR_INVALID_ARG_TYPE,
    "tjson_strip_nulls(1@2001-01-01, true)");
  expect_error(jsonbset_pretty(intset), MEOS_ERR_INVALID_ARG_TYPE,
    "jsonbset_pretty({1, 2})");
  expect_error(jsonbset_strip_nulls(intset, true), MEOS_ERR_INVALID_ARG_TYPE,
    "jsonbset_strip_nulls({1, 2}, true)");
  expect_error(jsonbset_to_textset(intset), MEOS_ERR_INVALID_ARG_TYPE,
    "jsonbset_to_textset({1, 2})");
  expect_error(textset_to_jsonbset(intset), MEOS_ERR_INVALID_ARG_TYPE,
    "textset_to_jsonbset({1, 2})");
  expect_error(jsonbset_to_alphanumset(intset, "a", T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_to_alphanumset({1, 2}, a, int4)");
  expect_error(tjsonb_to_tbool(tint, "c", NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_to_tbool(1@2001-01-01, c)");
  expect_error(tjsonb_to_tint(tint, "a", NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_to_tint(1@2001-01-01, a)");
  expect_error(tjsonb_to_tfloat(tint, "a", LINEAR, NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_to_tfloat(1@2001-01-01, a)");
  expect_error(tjsonb_to_ttext_key(tint, "a", NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_to_ttext_key(1@2001-01-01, a)");
  /* A result base type that is not alphanumeric is reported alike: the values
   * extracted from the JSON are converted to that type */
  expect_error(jsonbset_to_alphanumset(jsonbset, "a", T_TSTZSPAN, NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_to_alphanumset(jsonbset, a, tstzspan)");

  /* A valid call still answers, and the guards leave no error behind */
  expect_value(tjsonb_to_ttext(tjsonb), "tjsonb_to_ttext(tjsonb)");
  expect_value(ttext_to_tjsonb(tjson), "ttext_to_tjsonb(ttext)");
  expect_value(tjsonb_pretty(tjsonb), "tjsonb_pretty(tjsonb)");
  expect_value(tjsonb_strip_nulls(tjsonb, true),
    "tjsonb_strip_nulls(tjsonb, true)");
  expect_value(tjson_strip_nulls(tjson, true), "tjson_strip_nulls(ttext, true)");
  expect_value(jsonbset_pretty(jsonbset), "jsonbset_pretty(jsonbset)");
  expect_value(jsonbset_strip_nulls(jsonbset, true),
    "jsonbset_strip_nulls(jsonbset, true)");
  expect_value(jsonbset_to_alphanumset(jsonbset, "a", T_INT4, NULL_RETURN),
    "jsonbset_to_alphanumset(jsonbset, a, int4)");
  expect_value(tjsonb_to_tbool(tjsonbb, "c", NULL_RETURN),
    "tjsonb_to_tbool(tjsonb, c)");
  expect_value(tjsonb_to_tint(tjsonb, "a", NULL_RETURN),
    "tjsonb_to_tint(tjsonb, a)");
  expect_value(tjsonb_to_tfloat(tjsonb, "a", LINEAR, NULL_RETURN),
    "tjsonb_to_tfloat(tjsonb, a)");
  expect_value(tjsonb_to_ttext_key(tjsonb, "a", NULL_RETURN),
    "tjsonb_to_ttext_key(tjsonb, a)");
  /* A JSONB set converted into text and back is the set it was */
  Set *textset = jsonbset_to_textset(jsonbset);
  char *textset_str = textset_out(textset);
  printf("jsonbset_to_textset(jsonbset): %s, errno %d\n", textset_str,
    meos_errno());
  assert(textset != NULL);
  assert(meos_errno() == 0);
  Set *back = textset_to_jsonbset(textset);
  printf("textset_to_jsonbset(textset): %s, errno %d\n",
    back ? "a value" : "NULL", meos_errno());
  assert(back != NULL);
  assert(set_eq(back, jsonbset));
  assert(meos_errno() == 0);
  free(textset); free(textset_str); free(back);
  nh = null_handle_type_from_string("return_null");
  printf("null_handle_type_from_string(return_null): %d, errno %d\n", (int) nh,
    meos_errno());
  assert(nh == NULL_RETURN);
  assert(meos_errno() == 0);

  /* The field, element and path extractions, one public function per
   * operation. Their inputs are those the PostgreSQL suite gives the -> and
   * ->>, #> and #>> operators (454_tjsonb_jsonfuncs, 451_jsonbset_jsonfuncs),
   * whose string values tell the two forms apart */
  Temporal *tobj = tjsonb_in(
    "\"{\\\"speed\\\": 10, \\\"units\\\": \\\"km/h\\\"}\"@2001-01-01");
  Temporal *tobjtxt = ttext_in(
    "\"{\\\"speed\\\": 10, \\\"units\\\": \\\"km/h\\\"}\"@2001-01-01");
  Temporal *tarr = tjsonb_in(
    "\"[\\\"Grand Place\\\", \\\"La Bourse\\\"]\"@2001-01-01");
  Set *sobj = jsonbset_in(
    "{\"{\\\"speed\\\": 10, \\\"units\\\": \\\"km/h\\\"}\"}");
  Set *sarr = jsonbset_in("{\"[\\\"Grand Place\\\", \\\"La Bourse\\\"]\"}");
  text *key = text_in("units");
  text *path[1] = { key };
  assert(tobj); assert(tobjtxt); assert(tarr); assert(sobj); assert(sarr);
  assert(key);
  meos_errno_reset();

  /* A null argument is reported rather than dereferenced */
  expect_error(jsonbset_object_field(NULL, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_object_field(NULL, units)");
  expect_error(jsonbset_object_field(sobj, NULL, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_object_field(jsonbset, NULL)");
  expect_error(jsonbset_object_field_text(NULL, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_object_field_text(NULL, units)");
  expect_error(jsonbset_object_field_text(sobj, NULL, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_object_field_text(jsonbset, NULL)");
  expect_error(jsonbset_array_element(NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_array_element(NULL, 1)");
  expect_error(jsonbset_array_element_text(NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_array_element_text(NULL, 1)");
  expect_error(jsonbset_extract_path(NULL, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_extract_path(NULL, {units})");
  expect_error(jsonbset_extract_path(sobj, NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_extract_path(jsonbset, NULL)");
  expect_error(jsonbset_extract_path_text(NULL, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_extract_path_text(NULL, {units})");
  expect_error(jsonbset_extract_path_text(sobj, NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_extract_path_text(jsonbset, NULL)");
  expect_error(tjson_object_field(NULL, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjson_object_field(NULL, units)");
  expect_error(tjson_object_field(tobjtxt, NULL, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjson_object_field(ttext, NULL)");
  expect_error(tjsonb_object_field(NULL, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_object_field(NULL, units)");
  expect_error(tjsonb_object_field(tobj, NULL, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_object_field(tjsonb, NULL)");
  expect_error(tjsonb_object_field_text(NULL, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_object_field_text(NULL, units)");
  expect_error(tjsonb_object_field_text(tobj, NULL, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_object_field_text(tjsonb, NULL)");
  expect_error(tjsonb_array_element(NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_array_element(NULL, 1)");
  expect_error(tjsonb_array_element_text(NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_array_element_text(NULL, 1)");
  expect_error(tjsonb_extract_path(NULL, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_extract_path(NULL, {units})");
  expect_error(tjsonb_extract_path(tobj, NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_extract_path(tjsonb, NULL)");
  expect_error(tjsonb_extract_path_text(NULL, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_extract_path_text(NULL, {units})");
  expect_error(tjsonb_extract_path_text(tobj, NULL, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_extract_path_text(tjsonb, NULL)");

  /* An argument of another type is reported as a type error */
  expect_error(jsonbset_object_field(intset, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_object_field({1, 2}, units)");
  expect_error(jsonbset_object_field_text(intset, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_object_field_text({1, 2}, units)");
  expect_error(jsonbset_array_element(intset, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_array_element({1, 2}, 1)");
  expect_error(jsonbset_array_element_text(intset, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_array_element_text({1, 2}, 1)");
  expect_error(jsonbset_extract_path(intset, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_extract_path({1, 2}, {units})");
  expect_error(jsonbset_extract_path_text(intset, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_extract_path_text({1, 2}, {units})");
  expect_error(tjson_object_field(tint, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "tjson_object_field(1@2001-01-01, units)");
  expect_error(tjsonb_object_field(tint, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_object_field(1@2001-01-01, units)");
  expect_error(tjsonb_object_field_text(tint, key, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_object_field_text(1@2001-01-01, units)");
  expect_error(tjsonb_array_element(tint, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_array_element(1@2001-01-01, 1)");
  expect_error(tjsonb_array_element_text(tint, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_array_element_text(1@2001-01-01, 1)");
  expect_error(tjsonb_extract_path(tint, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE, "tjsonb_extract_path(1@2001-01-01, {units})");
  expect_error(tjsonb_extract_path_text(tint, path, 1, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_extract_path_text(1@2001-01-01, {units})");
  /* A path of no element is reported as an invalid value */
  expect_error(jsonbset_extract_path(sobj, path, 0, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonbset_extract_path(jsonbset, {}, 0)");
  expect_error(tjsonb_extract_path_text(tobj, path, 0, NULL_JSON_NULL),
    MEOS_ERR_INVALID_ARG_VALUE, "tjsonb_extract_path_text(tjsonb, {}, 0)");

  /* A valid call answers what the PostgreSQL suite answers: the JSONB form
   * keeps the string quoted, the text form unquotes it */
  Set *s; Temporal *t;
  s = jsonbset_object_field(sobj, key, NULL_JSON_NULL);
  expect_text(jsonbset_out(s), "{\"\\\"km/h\\\"\"}",
    "jsonbset_object_field(jsonbset, units)");
  free(s);
  s = jsonbset_object_field_text(sobj, key, NULL_JSON_NULL);
  expect_text(textset_out(s), "{\"km/h\"}",
    "jsonbset_object_field_text(jsonbset, units)");
  free(s);
  s = jsonbset_array_element(sarr, 1, NULL_JSON_NULL);
  expect_text(jsonbset_out(s), "{\"\\\"La Bourse\\\"\"}",
    "jsonbset_array_element(jsonbset, 1)");
  free(s);
  s = jsonbset_array_element_text(sarr, 1, NULL_JSON_NULL);
  expect_text(textset_out(s), "{\"La Bourse\"}",
    "jsonbset_array_element_text(jsonbset, 1)");
  free(s);
  s = jsonbset_extract_path(sobj, path, 1, NULL_JSON_NULL);
  expect_text(jsonbset_out(s), "{\"\\\"km/h\\\"\"}",
    "jsonbset_extract_path(jsonbset, {units})");
  free(s);
  s = jsonbset_extract_path_text(sobj, path, 1, NULL_JSON_NULL);
  expect_text(textset_out(s), "{\"km/h\"}",
    "jsonbset_extract_path_text(jsonbset, {units})");
  free(s);
  t = tjson_object_field(tobjtxt, key, NULL_JSON_NULL);
  expect_text(ttext_out(t), "\"\\\"km/h\\\"\"@2001-01-01 00:00:00+00",
    "tjson_object_field(ttext, units)");
  free(t);
  t = tjsonb_object_field(tobj, key, NULL_JSON_NULL);
  expect_text(tjsonb_out(t), "\"\\\"km/h\\\"\"@2001-01-01 00:00:00+00",
    "tjsonb_object_field(tjsonb, units)");
  free(t);
  t = tjsonb_object_field_text(tobj, key, NULL_JSON_NULL);
  expect_text(ttext_out(t), "\"km/h\"@2001-01-01 00:00:00+00",
    "tjsonb_object_field_text(tjsonb, units)");
  free(t);
  t = tjsonb_array_element(tarr, 1, NULL_JSON_NULL);
  expect_text(tjsonb_out(t), "\"\\\"La Bourse\\\"\"@2001-01-01 00:00:00+00",
    "tjsonb_array_element(tjsonb, 1)");
  free(t);
  t = tjsonb_array_element_text(tarr, 1, NULL_JSON_NULL);
  expect_text(ttext_out(t), "\"La Bourse\"@2001-01-01 00:00:00+00",
    "tjsonb_array_element_text(tjsonb, 1)");
  free(t);
  t = tjsonb_extract_path(tobj, path, 1, NULL_JSON_NULL);
  expect_text(tjsonb_out(t), "\"\\\"km/h\\\"\"@2001-01-01 00:00:00+00",
    "tjsonb_extract_path(tjsonb, {units})");
  free(t);
  t = tjsonb_extract_path_text(tobj, path, 1, NULL_JSON_NULL);
  expect_text(ttext_out(t), "\"km/h\"@2001-01-01 00:00:00+00",
    "tjsonb_extract_path_text(tjsonb, {units})");
  free(t);

  /* The replacements of the value at a path, one function taking a value and
   * one treating a null value, as jsonb_set and jsonb_set_lax */
  Jsonb *mph = jsonb_in("\"mph\"");
  text *delete_key = text_in("delete_key");
  assert(mph); assert(delete_key);
  meos_errno_reset();
  expect_error(jsonbset_set(sobj, path, 1, NULL, true), MEOS_ERR_INVALID_ARG,
    "jsonbset_set(jsonbset, {units}, NULL)");
  expect_error(tjsonb_set(tobj, path, 1, NULL, true), MEOS_ERR_INVALID_ARG,
    "tjsonb_set(tjsonb, {units}, NULL)");
  expect_error(jsonbset_set_lax(sobj, path, 1, NULL, true, NULL),
    MEOS_ERR_INVALID_ARG, "jsonbset_set_lax(jsonbset, {units}, NULL, NULL)");
  expect_error(tjsonb_set_lax(tobj, path, 1, NULL, true, NULL),
    MEOS_ERR_INVALID_ARG, "tjsonb_set_lax(tjsonb, {units}, NULL, NULL)");
  Set *ss = jsonbset_set(sobj, path, 1, mph, true);
  expect_text(jsonbset_out(ss),
    "{\"{\\\"speed\\\": 10, \\\"units\\\": \\\"mph\\\"}\"}",
    "jsonbset_set(jsonbset, {units}, mph)");
  free(ss);
  t = tjsonb_set_lax(tobj, path, 1, NULL, true, delete_key);
  expect_text(tjsonb_out(t),
    "\"{\\\"speed\\\": 10}\"@2001-01-01 00:00:00+00",
    "tjsonb_set_lax(tjsonb, {units}, NULL, delete_key)");
  free(t);
  free(mph); free(delete_key);

  free(tobj); free(tobjtxt); free(tarr); free(sobj); free(sarr); free(key);
  free(tjsonb); free(tjson); free(jsonbset); free(tjsonbb); free(tint);
  free(intset);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
