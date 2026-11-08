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
 * @brief A program that tests the CamelCase name of every type, the name the
 * bindings give its class
 * @details The program checks the names of a few types, and that every name
 * #meostype_camel_name answers is made of letters and digits, starts with a
 * capital, is the name of no other type, and is the name #meostype_name
 * answers when written in lowercase, except for the three base types whose
 * name is the one of PostgreSQL: `int4`, `int8` and `float8` are `Int`,
 * `BigInt` and `Float`.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o type_camel_name_test type_camel_name_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_catalog.h>

/* A type and its CamelCase name */
typedef struct
{
  MeosType type;
  const char *camel;
} type_camel;

static const type_camel CASES[] =
{
  {T_TSTZSPANSET, "TsTzSpanSet"},
  {T_BIGINTSPANSET, "BigIntSpanSet"},
  {T_INT8, "BigInt"},
  {T_TBIGINT, "TBigInt"},
  {T_TIMESTAMPTZ, "TimestampTz"},
  {T_STBOX, "STBox"},
  {T_TGEOMPOINT, "TGeomPoint"},
  {T_TRGEOMETRY, "TRGeometry"},
  {T_TPCBOX, "TPCBox"},
  {T_TNPOINT, "TNPoint"},
  {T_TCBUFFER, "TCBuffer"},
  {T_H3INDEXSET, "H3IndexSet"},
  {T_TJSONB, "TJsonb"},
};

/* The base types whose CamelCase name is not their name in lowercase */
static const type_camel BASES[] =
{
  {T_INT4, "Int"},
  {T_INT8, "BigInt"},
  {T_FLOAT8, "Float"},
};

/* Return true when the two strings are equal ignoring case */
static bool
same_letters(const char *a, const char *b)
{
  if (strlen(a) != strlen(b))
    return false;
  for (size_t i = 0; a[i]; i++)
    if (tolower((unsigned char) a[i]) != tolower((unsigned char) b[i]))
      return false;
  return true;
}

int
main(void)
{
  meos_initialize();
  int failures = 0;

  int n = (int) (sizeof(CASES) / sizeof(CASES[0]));
  for (int i = 0; i < n; i++)
  {
    const char *camel = meostype_camel_name(CASES[i].type);
    if (! camel || strcmp(camel, CASES[i].camel) != 0)
    {
      printf("FAILED: %s answers %s, expected %s\n",
        meostype_name(CASES[i].type), camel ? camel : "NULL", CASES[i].camel);
      failures++;
    }
  }
  printf("%d types answer their CamelCase name\n", n);

  /* A type no binding names, and a value no type has, answer NULL */
  if (meostype_camel_name(T_INT4RANGE) != NULL ||
      meostype_camel_name(T_UNKNOWN) != NULL ||
      meostype_camel_name((MeosType) NUM_MEOS_TYPES) != NULL)
  {
    printf("FAILED: a type with no CamelCase name answers one\n");
    failures++;
  }

  int named = 0;
  int nbases = (int) (sizeof(BASES) / sizeof(BASES[0]));
  for (int t = 0; t < (int) NUM_MEOS_TYPES; t++)
  {
    const char *camel = meostype_camel_name((MeosType) t);
    if (! camel)
      continue;
    named++;
    const char *name = meostype_name((MeosType) t);
    bool alnum = isupper((unsigned char) camel[0]);
    for (const char *c = camel; *c && alnum; c++)
      alnum = isalnum((unsigned char) *c);
    if (! alnum)
    {
      printf("FAILED: %s has the CamelCase name %s\n", name, camel);
      failures++;
    }
    const char *spelled = name;
    for (int i = 0; i < nbases; i++)
      if (BASES[i].type == (MeosType) t)
        spelled = BASES[i].camel;
    if (! same_letters(camel, spelled))
    {
      printf("FAILED: the CamelCase name %s does not spell %s\n", camel,
        spelled);
      failures++;
    }
    for (int u = 0; u < t; u++)
    {
      const char *other = meostype_camel_name((MeosType) u);
      if (other && strcmp(other, camel) == 0)
      {
        printf("FAILED: %s and %s share the CamelCase name %s\n",
          meostype_name((MeosType) u), name, camel);
        failures++;
      }
    }
  }
  printf("%d types have a CamelCase name\n", named);

  meos_finalize();
  return failures == 0 ? 0 : 1;
}
