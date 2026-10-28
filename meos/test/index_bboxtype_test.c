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
 * @brief A program that tests the key every host indexes a value by
 * @details The GiST and SP-GiST operator classes of MobilityDB and the
 * RTree and SPTree of MEOS, which MobilityDuck's indexes are, store for a value
 * the box #type_bboxtype answers for its type. The program checks that
 * box for each of the 47 indexed types, and that #meostype_from_string answers
 * the type of every name #meostype_name gives.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o index_bboxtype_test index_bboxtype_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_catalog.h>

/* An indexed type and the key the index stores */
typedef struct
{
  MeosType type;
  MeosType key;
} index_key;

static const index_key KEYS[] =
{
  /* Box types, the spans and the boxes: their own key */
  {T_INTSPAN, T_INTSPAN}, {T_BIGINTSPAN, T_BIGINTSPAN},
  {T_FLOATSPAN, T_FLOATSPAN}, {T_DATESPAN, T_DATESPAN},
  {T_TSTZSPAN, T_TSTZSPAN},
  {T_TBOX, T_TBOX}, {T_STBOX, T_STBOX}, {T_TPCBOX, T_TPCBOX},
  /* Number and time sets: the span their bounds state */
  {T_INTSET, T_INTSPAN}, {T_BIGINTSET, T_BIGINTSPAN},
  {T_FLOATSET, T_FLOATSPAN}, {T_DATESET, T_DATESPAN},
  {T_TSTZSET, T_TSTZSPAN},
  /* Span sets: the span their bounds state */
  {T_INTSPANSET, T_INTSPAN}, {T_BIGINTSPANSET, T_BIGINTSPAN},
  {T_FLOATSPANSET, T_FLOATSPAN}, {T_DATESPANSET, T_DATESPAN},
  {T_TSTZSPANSET, T_TSTZSPAN},
  /* Spatial sets: the spatiotemporal box they store */
  {T_GEOMSET, T_STBOX}, {T_GEOGSET, T_STBOX}, {T_NPOINTSET, T_STBOX},
  {T_POSESET, T_STBOX}, {T_POSECHAINSET, T_STBOX}, {T_CBUFFERSET, T_STBOX},
  {T_H3INDEXSET, T_STBOX}, {T_QUADBINSET, T_STBOX}, {T_S2CELLSET, T_STBOX},
  /* Temporal values: the bounding box they store */
  {T_TBOOL, T_TSTZSPAN}, {T_TTEXT, T_TSTZSPAN}, {T_TJSONB, T_TSTZSPAN},
  {T_TINT, T_TBOX}, {T_TBIGINT, T_TBOX}, {T_TFLOAT, T_TBOX},
  {T_TGEOMPOINT, T_STBOX}, {T_TGEOGPOINT, T_STBOX}, {T_TGEOMETRY, T_STBOX},
  {T_TGEOGRAPHY, T_STBOX}, {T_TCBUFFER, T_STBOX}, {T_TNPOINT, T_STBOX},
  {T_TPOSE, T_STBOX}, {T_TPOSECHAIN, T_STBOX}, {T_TRGEOMETRY, T_STBOX},
  {T_TH3INDEX, T_STBOX},
  {T_TQUADBIN, T_STBOX}, {T_TS2CELL, T_STBOX},
  {T_TPCPOINT, T_TPCBOX}, {T_TPCPATCH, T_TPCBOX},
};

/* Main program */
int main(void)
{
  meos_initialize();
  int failures = 0;

  int n = (int) (sizeof(KEYS) / sizeof(KEYS[0]));
  for (int i = 0; i < n; i++)
  {
    MeosType key = type_bboxtype(KEYS[i].type);
    if (key != KEYS[i].key)
    {
      printf("FAILED: %s is indexed by %s, expected %s\n",
        meostype_name(KEYS[i].type), meostype_name(key),
        meostype_name(KEYS[i].key));
      failures++;
    }
  }
  printf("%d indexed types, each keyed by its box\n", n);

  /* Every type with a box is listed above, except the internal states of the
   * temporal averages, which no value carries */
  for (int t = 0; t < (int) NUM_MEOS_TYPES; t++)
  {
    if (type_bboxtype((MeosType) t) == T_UNKNOWN || t == T_TDOUBLE2 ||
        t == T_TDOUBLE3 || t == T_TDOUBLE4)
      continue;
    bool listed = false;
    for (int i = 0; i < n && ! listed; i++)
      listed = (KEYS[i].type == (MeosType) t);
    if (! listed)
    {
      printf("FAILED: %s has a box and is not listed\n",
        meostype_name((MeosType) t));
      failures++;
    }
  }

  /* Types with no key: a base value and an alphanumeric set */
  if (type_bboxtype(T_INT4) != T_UNKNOWN ||
      type_bboxtype(T_TEXTSET) != T_UNKNOWN)
  {
    printf("FAILED: a type with no box is keyed\n");
    failures++;
  }

  /* Every name meostype_name gives answers its type */
  int names = 0;
  for (int t = 0; t < (int) NUM_MEOS_TYPES; t++)
  {
    const char *name = meostype_name((MeosType) t);
    if (strcmp(name, "unknown") == 0)
      continue;
    names++;
    if (meostype_from_string(name) != (MeosType) t)
    {
      printf("FAILED: %s answers %d, expected %d\n", name,
        meostype_from_string(name), t);
      failures++;
    }
  }
  printf("%d type names answer their type\n", names);
  if (meostype_from_string("nosuchtype") != T_UNKNOWN)
  {
    printf("FAILED: an unknown name answers a type\n");
    failures++;
  }

  meos_finalize();
  return failures == 0 ? 0 : 1;
}
