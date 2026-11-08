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
 * @brief Test that the like_regex predicate of a JSON path answers and
 * leaves no memory behind
 * @details A like_regex predicate builds the text of its pattern on the first
 * string it tests, and the cache of compiled regular expressions keeps a copy
 * of that text of its own. The program evaluates paths holding a like_regex
 * on strings the pattern matches, on strings it does not match, and on a
 * value that is not a string, which the predicate answers as unknown without
 * building the text, and checks each answer. Run under valgrind with
 * --errors-for-leak-kinds=definite, as the MEOS smoke workflow runs every
 * load test, a pattern text left behind by an evaluation fails the run.
 *
 * The patterns number fewer than the 32 entries of the cache, so no compiled
 * expression is dropped from it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

/* Each evaluation is repeated, so the pattern text of every one of them, and
 * not the first alone, is accounted for */
#define NUM_ROUNDS 10

static int failures = 0;

/**
 * @brief Check that @p path answers @p expected on @p json
 */
static void
check(const char *json, const char *path, int expected)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *vars = jsonb_in("{}");
  if (! jb || ! jp || ! vars)
  {
    printf("  FAIL %s on %s: an input is not read\n", path, json);
    failures++;
  }
  else
  {
    int result = jsonb_path_exists(jb, jp, vars, false, false);
    if (result != expected)
    {
      printf("  FAIL %s on %s: %d, expected %d\n", path, json, result,
        expected);
      failures++;
    }
  }
  free(jb); free(jp); free(vars);
}

int
main(void)
{
  meos_initialize();

  for (int i = 0; i < NUM_ROUNDS; i++)
  {
    /* A string the pattern matches */
    check("\"abc\"", "$ ? (@ like_regex \"^a[[:alpha:]]+\")", 1);
    /* A string the pattern does not match */
    check("\"xyz\"", "$ ? (@ like_regex \"^a[[:alpha:]]+\")", 0);
    /* The flags of the pattern */
    check("\"ABC\"", "$ ? (@ like_regex \"^a\" flag \"i\")", 1);
    /* Every string of an array, the pattern text built once for them */
    check("[\"ab\", \"cd\", \"ae\"]", "$[*] ? (@ like_regex \"^a\")", 1);
    /* A value that is not a string, which builds no pattern text */
    check("1", "$ ? (@ like_regex \"^a\")", 0);
  }

  if (failures == 0)
    printf("json_like_regex_load test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
