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
 * @brief A program that tests the text of the values of the enumerations a
 * host passes to MEOS
 * @details For each enumeration a public function takes, the interpolation,
 * the handling of a SQL NULL, the pixel type, the search operator and the
 * kind of an in-memory index, the program checks that the name of every value
 * reads back as that value, in lower and upper case alike, and that an
 * unknown name and an unknown value are reported under the noexit error
 * handler.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o enum_token_test enum_token_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>
#include <meos_raster.h>

static int failures = 0;

/* Return the name in upper case, in a buffer of the caller */
static const char *
upper(const char *name, char *buf, size_t size)
{
  size_t i = 0;
  for (; name[i] && i + 1 < size; i++)
    buf[i] = (char) toupper((unsigned char) name[i]);
  buf[i] = '\0';
  return buf;
}

/* Check that the name of a value reads back as the value in both cases */
static void
check(const char *enumname, int value, const char *name, int back, int upback,
  const char *want)
{
  if (! name || strcmp(name, want) != 0 || back != value || upback != value)
  {
    printf("FAILED: %s %d named %s reads back as %d and %d, expected %s\n",
      enumname, value, name ? name : "NULL", back, upback, want);
    failures++;
  }
}

/* Check that a call reported an invalid argument value, and reset it */
static void
check_error(const char *call)
{
  if (meos_errno() != MEOS_ERR_INVALID_ARG_VALUE)
  {
    printf("FAILED: %s reports errno %d, expected %d\n", call, meos_errno(),
      MEOS_ERR_INVALID_ARG_VALUE);
    failures++;
  }
  meos_errno_reset();
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();
  char buf[64];

  static const char *INTERP[] = {"None", "Discrete", "Step", "Linear"};
  for (int i = INTERP_NONE; i <= LINEAR; i++)
  {
    const char *name = interptype_name((interpType) i);
    check("interpType", i, name, name ? (int) interptype_from_string(name) : -1,
      name ? (int) interptype_from_string(upper(name, buf, sizeof(buf))) : -1,
      INTERP[i]);
  }

  static const char *NULLHANDLE[] = {"invalid_value", "raise_exception",
    "use_json_null", "delete_key", "return_null"};
  for (int i = NULL_INVALID; i <= NULL_RETURN; i++)
  {
    const char *name = nullhandletype_name((nullHandleType) i);
    check("nullHandleType", i, name,
      name ? (int) nullhandletype_from_string(name) : -1,
      name ? (int) nullhandletype_from_string(upper(name, buf, sizeof(buf))) : -1,
      NULLHANDLE[i]);
  }

  static const struct { MeosPixType type; const char *name; } PIX[] =
  {
    {MEOS_PT_UINT8, "uint8"}, {MEOS_PT_INT16, "int16"},
    {MEOS_PT_INT32, "int32"}, {MEOS_PT_FLOAT32, "float32"},
    {MEOS_PT_FLOAT64, "float64"}, {MEOS_PT_INT8, "int8"},
    {MEOS_PT_UINT16, "uint16"}, {MEOS_PT_UINT32, "uint32"},
    {MEOS_PT_INT64, "int64"}, {MEOS_PT_UINT64, "uint64"},
    {MEOS_PT_FLOAT16, "float16"},
  };
  for (size_t i = 0; i < sizeof(PIX) / sizeof(PIX[0]); i++)
  {
    const char *name = meospixtype_name(PIX[i].type);
    check("MeosPixType", (int) PIX[i].type, name,
      name ? (int) meospixtype_from_string(name) : -1,
      name ? (int) meospixtype_from_string(upper(name, buf, sizeof(buf))) : -1,
      PIX[i].name);
  }

  static const char *SEARCHOP[] = {"overlaps", "contains", "contained",
    "left", "overleft", "right", "overright", "below", "overbelow", "above",
    "overabove", "front", "overfront", "back", "overback", "before",
    "overbefore", "after", "overafter", "same", "adjacent"};
  for (int i = INDEX_OVERLAPS; i <= INDEX_ADJACENT; i++)
  {
    const char *name = indexsearchop_name((IndexSearchOp) i);
    check("IndexSearchOp", i, name,
      name ? (int) indexsearchop_from_string(name) : -1,
      name ? (int) indexsearchop_from_string(upper(name, buf, sizeof(buf))) : -1,
      SEARCHOP[i]);
  }

  static const char *SPTREE[] = {"quadtree", "kdtree"};
  for (int i = SPTREE_QUADTREE; i <= SPTREE_KDTREE; i++)
  {
    const char *name = sptreekind_name((SPTreeKind) i);
    check("SPTreeKind", i, name, name ? (int) sptreekind_from_string(name) : -1,
      name ? (int) sptreekind_from_string(upper(name, buf, sizeof(buf))) : -1,
      SPTREE[i]);
  }
  printf("every name reads back as its value\n");

  /* An unknown name and an unknown value are reported */
  if ((int) indexsearchop_from_string("nosuchop") != -1)
  {
    printf("FAILED: an unknown search operator answers a value\n");
    failures++;
  }
  check_error("indexsearchop_from_string(nosuchop)");
  if ((int) sptreekind_from_string("rtree") != -1)
  {
    printf("FAILED: an unknown kind of index answers a value\n");
    failures++;
  }
  check_error("sptreekind_from_string(rtree)");
  interptype_from_string("cubic");
  check_error("interptype_from_string(cubic)");
  nullhandletype_from_string("ignore");
  check_error("nullhandletype_from_string(ignore)");
  if (interptype_name((interpType) 99) != NULL ||
      indexsearchop_name((IndexSearchOp) 99) != NULL ||
      sptreekind_name((SPTreeKind) 99) != NULL ||
      nullhandletype_name((nullHandleType) 99) != NULL)
  {
    printf("FAILED: an unknown value answers a name\n");
    failures++;
  }
  meos_errno_reset();
  printf("unknown names and values are reported\n");

  meos_finalize();
  return failures == 0 ? 0 : 1;
}
