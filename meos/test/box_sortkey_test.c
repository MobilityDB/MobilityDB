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
 * @brief A program that tests the sort keys of temporal, spatiotemporal and
 * point cloud boxes
 * @details The sort key of a temporal box, of a spatiotemporal box and of a
 * point cloud box must order them as #tbox_cmp, #stbox_cmp and #tpcbox_cmp
 * do. Over every pair of a corpus of boxes of each box type, the sign of a byte
 * comparison of the two keys must be the sign of the comparison of the two
 * boxes, and two boxes the comparison holds equal must have identical keys
 * and be equal. The temporal boxes mix integer, big integer (with the 64-bit
 * extremes) and float value spans, with and without a period, and periods
 * alone. The spatiotemporal and point cloud boxes cover three SRIDs, every
 * combination of the space, z, time and geodetic flags, zero of both signs,
 * large magnitudes and infinities, and for point cloud boxes schema ids up to
 * the largest unsigned value. The program also verifies that #tbox_sortkey,
 * #stbox_sortkey and #tpcbox_sortkey report a null box and a null size.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o box_sortkey_test box_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pointcloud.h>

#define MAXB 2400

typedef int (*boxcorpus_cmp)(const void *, const void *);
typedef bool (*boxcorpus_eq)(const void *, const void *);
typedef uint8_t *(*boxcorpus_key)(const void *, size_t *);

static void *boxes[MAXB];
static int nboxes;
static long failures;

static int
order_sign(int x)
{
  return (x > 0) - (x < 0);
}

/* The sign of an unsigned byte comparison, a shorter prefix sorting first */
static int
sortkey_order(const uint8_t *a, size_t na, const uint8_t *b, size_t nb)
{
  size_t n = na < nb ? na : nb;
  int c = memcmp(a, b, n);
  if (c)
    return order_sign(c);
  return (na > nb) - (na < nb);
}

static void
boxcorpus_add(void *box)
{
  if (box && nboxes < MAXB)
    boxes[nboxes++] = box;
  meos_errno_reset();
}

/* Compare every pair of the corpus by key, by value and by equality, then
 * free it */
static void
boxcorpus_check(const char *name, boxcorpus_cmp cmp, boxcorpus_eq eq,
  boxcorpus_key key)
{
  long pairs = 0, bad = 0;
  for (int i = 0; i < nboxes; i++)
    for (int j = 0; j < nboxes; j++)
    {
      size_t na, nb;
      uint8_t *ka = key(boxes[i], &na);
      uint8_t *kb = key(boxes[j], &nb);
      int c = order_sign(cmp(boxes[i], boxes[j]));
      if (sortkey_order(ka, na, kb, nb) != c ||
          (c == 0) != eq(boxes[i], boxes[j]))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %d boxes, %ld pairs, %ld ordered or equated otherwise than "
    "their comparison\n", name, nboxes, pairs, bad);
  failures += bad;
  for (int i = 0; i < nboxes; i++)
    free(boxes[i]);
  nboxes = 0;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting, so that a box the constructors refuse is
   * skipped */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Span *periods[] = {
    NULL,
    tstzspan_make(-1000000000000, 0, true, false),
    tstzspan_make(0, 1000, true, true),
    tstzspan_make(0, 1000, false, true),
    tstzspan_make(1, 2, true, true),
  };
  const int np = sizeof(periods) / sizeof(periods[0]);

  /* Temporal boxes of integer, big integer and float values, which are one
   * type, and of a period alone */
  const int ints[] = {INT_MIN, -256, -1, 0, 1, 255, 256, INT_MAX - 1};
  const int ni = sizeof(ints) / sizeof(ints[0]);
  const int64 bigints[] = {INT64_MIN, -4294967296, -1, 0, 1, 4294967296,
    INT64_MAX - 1};
  const int nb = sizeof(bigints) / sizeof(bigints[0]);
  const double dbl[] = {-INFINITY, -1e300, -1.0, -0.0, 0.0, 0.5, 1e300,
    INFINITY};
  const int nd = sizeof(dbl) / sizeof(dbl[0]);
  for (int p = 0; p < np; p++)
  {
    if (periods[p])
      boxcorpus_add(tbox_make(NULL, periods[p]));
    for (int a = 0; a < ni; a++)
      for (int b = a; b < ni; b += 2)
      {
        Span *s = intspan_make(ints[a], ints[b] + 1, true, false);
        boxcorpus_add(tbox_make(s, periods[p]));
        free(s);
      }
    for (int a = 0; a < nb; a++)
      for (int b = a; b < nb; b += 2)
      {
        Span *s = bigintspan_make(bigints[a], bigints[b] + 1, true, false);
        boxcorpus_add(tbox_make(s, periods[p]));
        free(s);
      }
    for (int a = 0; a < nd; a++)
      for (int b = a; b < nd; b++)
        for (int f = 0; f < 2; f++)
        {
          Span *s = floatspan_make(dbl[a], dbl[b], true, (f || a == b));
          boxcorpus_add(tbox_make(s, periods[p]));
          free(s);
        }
  }
  boxcorpus_check("tbox", (boxcorpus_cmp) tbox_cmp, (boxcorpus_eq) tbox_eq,
    (boxcorpus_key) tbox_sortkey);

  /* Spatiotemporal boxes over several SRIDs, then point cloud boxes over the
   * same shapes and several schema ids, each naming a schema of one of these
   * SRIDs, the SRID a point cloud box takes from its schema id */
  const int32_t srids[] = {0, 3857, 4326};
  const double coords[] = {-INFINITY, -1e300, -1.0, -0.0, 0.0, 0.5, 1e300};
  const int nc = sizeof(coords) / sizeof(coords[0]);
  const struct { uint32_t pcid; int32_t srid; } schemas[] = {
    {0, 0}, {1, 0}, {2, 3857}, {3, 4326}, {0x80000000u, 3857},
    {UINT32_MAX, 4326}};
  PCDimensionSpec dims[2] = {
    { "X", NULL, 1, "int32_t", 1, 0, true },
    { "Y", NULL, 2, "int32_t", 1, 0, true }
  };
  /* Schema id 0 names no schema */
  for (int k = 1; k < 6; k++)
    if (! meos_pc_schema_register_dims(schemas[k].pcid, schemas[k].srid,
        "none", dims, 2))
    {
      printf("FAILED: the schema of pcid %u is not registered\n",
        schemas[k].pcid);
      return 1;
    }
  for (int pc = 0; pc < 2; pc++)
  {
    const int nk = pc ? 6 : 3;
    for (int k = 0; k < nk; k++)
      for (int p = 0; p < np; p++)
        for (int fl = 0; fl < 4; fl++)
        {
          bool hasx = fl > 0, hasz = fl > 1, geodetic = fl == 3;
          if (! hasx)
          {
            if (! periods[p])
              continue;
            boxcorpus_add(pc ?
              (void *) tpcbox_make(false, false, false,
                schemas[k].pcid, 0, 0, 0, 0, 0, 0, periods[p]) :
              (void *) stbox_make(false, false, false, srids[k], 0, 0, 0,
                0, 0, 0, periods[p]));
            continue;
          }
          for (int a = 0; a < nc; a += 1 + pc)
            for (int b = a; b < nc; b += 2 + pc)
            {
              double lo = coords[a], hi = coords[b];
              boxcorpus_add(pc ?
                (void *) tpcbox_make(true, hasz,
                  geodetic, schemas[k].pcid, lo, hi, -hi, -lo, lo, hi,
                  periods[p]) :
                (void *) stbox_make(true, hasz, geodetic, srids[k], lo, hi,
                  -hi, -lo, lo, hi, periods[p]));
            }
        }
    if (pc)
      boxcorpus_check("tpcbox", (boxcorpus_cmp) tpcbox_cmp,
        (boxcorpus_eq) tpcbox_eq, (boxcorpus_key) tpcbox_sortkey);
    else
      boxcorpus_check("stbox", (boxcorpus_cmp) stbox_cmp,
        (boxcorpus_eq) stbox_eq, (boxcorpus_key) stbox_sortkey);
  }

  /* A null box or a null size is reported rather than dereferenced */
  size_t size;
  TBox *tbox = tbox_make(NULL, periods[2]);
  STBox *stbox = stbox_make(true, false, false, 0, 0, 1, 0, 1, 0, 0, NULL);
  TPCBox *tpcbox = tpcbox_make(true, false, false, 1, 0, 1, 0, 1, 0,
    0, NULL);
  void *values[] = {tbox, stbox, tpcbox};
  boxcorpus_key keys[] = {(boxcorpus_key) tbox_sortkey,
    (boxcorpus_key) stbox_sortkey, (boxcorpus_key) tpcbox_sortkey};
  const char *names[] = {"tbox_sortkey", "stbox_sortkey", "tpcbox_sortkey"};
  for (int i = 0; i < 3; i++)
  {
    meos_errno_reset();
    uint8_t *key = keys[i](NULL, &size);
    printf("%s(NULL, size): %s, errno %d\n", names[i], key ? "a key" : "NULL",
      meos_errno());
    assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
    meos_errno_reset();
    key = keys[i](values[i], NULL);
    printf("%s(box, NULL): %s, errno %d\n", names[i], key ? "a key" : "NULL",
      meos_errno());
    assert(key == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
    meos_errno_reset();
    key = keys[i](values[i], &size);
    printf("%s(box, size): %s of %zu bytes, errno %d\n", names[i],
      key ? "a key" : "NULL", size, meos_errno());
    assert(key != NULL && meos_errno() == 0);
    free(key); free(values[i]);
  }
  for (int p = 0; p < np; p++)
    free(periods[p]);

  printf("%ld pair(s) ordered or equated otherwise than their comparison\n",
    failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
