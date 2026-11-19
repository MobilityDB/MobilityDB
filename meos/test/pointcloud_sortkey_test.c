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
 * @brief A program that tests that the comparison of point cloud values is a
 * total order consistent with their equality that orders them first by their
 * pcid, and that their sort keys order the sets and the temporal values of
 * these types as their comparison does
 * @details pgPointCloud defines no order on a point or a patch, and the only
 * order it states, along the decoded dimensions, needs the schema of the pcid.
 * A comparison reads no catalog, so it orders two values first by their pcid
 * as a number and then by the stored bytes that follow it, as unsigned bytes,
 * a value prefixing another sorting first, and holds them equal exactly when
 * their meaningful bytes are. A sort key is the string of bytes whose unsigned
 * byte order is the order of the comparison. The program therefore checks,
 * over points and patches of the pcids 1, 2, 256 and 257, whose schemas have
 * three dimensions, and 3, whose schema has two:
 * - over every pair, that the comparison answers 0 exactly when the equality
 *   holds, that swapping the operands swaps the sign, and that two values of
 *   different pcids are ordered as their pcids are;
 * - over every triple, that the comparison is transitive;
 * - over every pair of sets of one and of two values, and of temporal values
 *   of one and of two instants, that the byte order of the keys is the order
 *   of #set_cmp and #temporal_cmp, which refuses two temporal values of
 *   mixed 2D and 3D dimensions, those of the pcid 3 and those of the others.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o pointcloud_sortkey_test pointcloud_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_pointcloud.h>

#define MAXV 256

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

/*****************************************************************************
 * The comparison of the base values
 *****************************************************************************/

typedef int (*cmpfn)(const void *, const void *);
typedef bool (*eqfn)(const void *, const void *);
typedef uint32_t (*pcidfn)(const void *);

/* Check that a comparison is antisymmetric, transitive, consistent with the
 * equality of the type and ordering two values of different pcids as their
 * pcids, over every pair and every triple of values */
static void
base_check(const char *name, void **v, int n, cmpfn cmp, eqfn eq,
  pcidfn pcid)
{
  long pairs = 0, inconsistent = 0, asymmetric = 0, bypcid = 0, triples = 0,
    intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(cmp(v[i], v[j]));
      if ((c == 0) != eq(v[i], v[j]))
        inconsistent++;
      if (c != -order_sign(cmp(v[j], v[i])))
        asymmetric++;
      uint32_t p1 = pcid(v[i]), p2 = pcid(v[j]);
      if (p1 != p2 && c != ((p1 > p2) - (p1 < p2)))
        bypcid++;
      pairs++;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int cij = order_sign(cmp(v[i], v[j]));
      if (cij > 0)
        continue;
      for (int k = 0; k < n; k++)
      {
        int cjk = order_sign(cmp(v[j], v[k]));
        if (cjk > 0)
          continue;
        /* i <= j and j <= k, so i <= k, and i == k only when both are equal */
        int cik = order_sign(cmp(v[i], v[k]));
        if (cik > 0 || (cik == 0 && (cij != 0 || cjk != 0)))
          intransitive++;
        triples++;
      }
    }
  printf("%s: %ld pairs, %ld whose comparison answers 0 otherwise than their "
    "equality, %ld whose swapped comparison is not the opposite, %ld of "
    "different pcids ordered otherwise than their pcids; %ld ordered triples, "
    "%ld not transitive\n", name, pairs, inconsistent, asymmetric, bypcid,
    triples, intransitive);
  failures += inconsistent + asymmetric + bypcid + intransitive;
}

static int
pcpoint_cmp_v(const void *a, const void *b)
{
  return pcpoint_cmp((const Pcpoint *) a, (const Pcpoint *) b);
}

static bool
pcpoint_eq_v(const void *a, const void *b)
{
  return pcpoint_eq((const Pcpoint *) a, (const Pcpoint *) b);
}

static uint32_t
pcpoint_pcid_v(const void *a)
{
  return pcpoint_get_pcid((const Pcpoint *) a);
}

static int
pcpatch_cmp_v(const void *a, const void *b)
{
  return pcpatch_cmp((const Pcpatch *) a, (const Pcpatch *) b);
}

static bool
pcpatch_eq_v(const void *a, const void *b)
{
  return pcpatch_eq((const Pcpatch *) a, (const Pcpatch *) b);
}

static uint32_t
pcpatch_pcid_v(const void *a)
{
  return pcpatch_get_pcid((const Pcpatch *) a);
}

/*****************************************************************************
 * The keys of the sets and of the temporal values
 *****************************************************************************/

/* Check that the keys of every pair of sets order them as #set_cmp does */
static void
set_check(const char *name, Set **sets, int n)
{
  long pairs = 0, bad = 0, refused = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = set_sortkey(sets[i], &na);
      uint8_t *kb = set_sortkey(sets[j], &nb);
      meos_errno_reset();
      int c = set_cmp(sets[i], sets[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld set pairs, %ld without a key, %ld ordered or equated "
    "otherwise than set_cmp\n", name, pairs, refused, bad);
  failures += refused + bad;
}

/* Check that the keys of every pair of temporal values order them as
 * #temporal_cmp does */
static void
temporal_check(const char *name, Temporal **temps, int n)
{
  long pairs = 0, bad = 0, refused = 0, mixed = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = temporal_sortkey(temps[i], &na);
      uint8_t *kb = temporal_sortkey(temps[j], &nb);
      meos_errno_reset();
      int c = temporal_cmp(temps[i], temps[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      /* The comparison refuses two values of mixed 2D and 3D dimensions,
       * those of the pcid 3 and those of the other pcids */
      else if (c == INT_MAX)
        mixed++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld temporal pairs, %ld without a key, %ld of mixed 2D and 3D "
    "dimensions, %ld ordered or equated otherwise than temporal_cmp\n", name,
    pairs, refused, mixed, bad);
  failures += refused + bad;
}

/* The instant of a value at a timestamp, through the typed constructor of
 * the point or of the patch */
typedef TInstant *(*instfn)(const void *, TimestampTz);

static TInstant *
tpcpointinst_v(const void *v, TimestampTz t)
{
  return tpcpointinst_make((const Pcpoint *) v, t);
}

static TInstant *
tpcpatchinst_v(const void *v, TimestampTz t)
{
  return tpcpatchinst_make((const Pcpatch *) v, t);
}

/* The temporal values of one and of two instants of a list of values, step
 * interpolation */
static int
temporal_make(Temporal **temps, void **v, int n, instfn inst)
{
  int ntemps = 0;
  TimestampTz t1 = timestamptz_in("2001-01-01", -1);
  TimestampTz t2 = timestamptz_in("2001-01-02", -1);
  for (int i = 0; i < n && ntemps < MAXV - 1; i += 2)
  {
    TInstant *inst1 = inst(v[i], t1);
    TInstant *inst2 = inst(v[(i * 7 + 5) % n], t2);
    if (! inst1 || ! inst2)
    {
      free(inst1); free(inst2);
      meos_errno_reset();
      continue;
    }
    temps[ntemps++] = (Temporal *) inst1;
    TInstant *insts[2] = {inst1, inst2};
    TSequence *seq = tsequence_make(insts, 2, true, true, STEP, false);
    if (seq)
      temps[ntemps++] = (Temporal *) seq;
    meos_errno_reset();
    free(inst2);
  }
  return ntemps;
}

/*****************************************************************************/

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  meos_pc_schema_clear();

  /* The schemas: three dimensions for the pcids 1, 2, 256 and 257, so that
   * their points have the same size and only the pcid tells them apart, and
   * two for the pcid 3 */
  PCDimensionSpec dims3[3] = {
    { "X", NULL, 1, "int32_t", 0.01, 0, true },
    { "Y", NULL, 2, "int32_t", 0.01, 0, true },
    { "Z", NULL, 3, "int32_t", 0.01, 0, true }
  };
  PCDimensionSpec dims2[2] = {
    { "X", NULL, 1, "int32_t", 0.01, 0, true },
    { "Y", NULL, 2, "int32_t", 0.01, 0, true }
  };
  const uint32_t pcids3[4] = {1, 2, 256, 257};
  for (int p = 0; p < 4; p++)
    if (! meos_pc_schema_register_dims(pcids3[p], 0, "none", dims3, 3))
    {
      printf("the schema of pcid %u is refused\n", pcids3[p]);
      return EXIT_FAILURE;
    }
  if (! meos_pc_schema_register_dims(3, 0, "none", dims2, 2))
  {
    printf("the schema of pcid 3 is refused\n");
    return EXIT_FAILURE;
  }

  /* Points of every pcid over values that differ in their low and in their
   * high bytes, and that are negative, zero and positive */
  void *points[MAXV];
  int npoints = 0;
  const double vals[] = {-300, -0.01, 0, 0.01, 2.56, 300};
  int nvals = (int) (sizeof(vals) / sizeof(vals[0]));
  for (int p = 0; p < 5; p++)
  {
    uint32_t pcid = (p < 4) ? pcids3[p] : 3;
    int ndims = (p < 4) ? 3 : 2;
    for (int x = 0; x < nvals; x++)
      for (int y = 0; y < nvals; y += 2)
      {
        double v[3] = {vals[x], vals[y], vals[(x + y) % nvals]};
        Pcpoint *pt = pcpoint_make(pcid, v, ndims);
        if (pt && npoints < MAXV)
          points[npoints++] = pt;
        meos_errno_reset();
      }
  }
  base_check("pcpoint", points, npoints, pcpoint_cmp_v, pcpoint_eq_v,
    pcpoint_pcid_v);

  /* Patches of one and of two points of the same pcid */
  void *patches[MAXV];
  int npatches = 0;
  for (int i = 0; i < npoints && npatches < MAXV - 1; i += 2)
  {
    const Pcpoint *one[1] = {(const Pcpoint *) points[i]};
    Pcpatch *pa = pcpatch_make(one, 1);
    if (pa)
      patches[npatches++] = pa;
    int j = (i + 3) % npoints;
    if (pcpoint_get_pcid(points[i]) == pcpoint_get_pcid(points[j]))
    {
      const Pcpoint *two[2] = {(const Pcpoint *) points[i],
        (const Pcpoint *) points[j]};
      pa = pcpatch_make(two, 2);
      if (pa)
        patches[npatches++] = pa;
    }
    meos_errno_reset();
  }
  base_check("pcpatch", patches, npatches, pcpatch_cmp_v, pcpatch_eq_v,
    pcpatch_pcid_v);

  /* Sets of one and of two values */
  Set *sets[MAXV];
  int nsets = 0;
  for (int i = 0; i < npoints && nsets < MAXV - 1; i += 3)
  {
    Pcpoint *one[1] = {(Pcpoint *) points[i]};
    sets[nsets++] = pcpointset_make(one, 1);
    Pcpoint *two[2] = {(Pcpoint *) points[i],
      (Pcpoint *) points[(i * 7 + 5) % npoints]};
    Set *s = pcpointset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("pcpointset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  nsets = 0;
  for (int i = 0; i < npatches && nsets < MAXV - 1; i += 2)
  {
    Pcpatch *one[1] = {(Pcpatch *) patches[i]};
    sets[nsets++] = pcpatchset_make(one, 1);
    Pcpatch *two[2] = {(Pcpatch *) patches[i],
      (Pcpatch *) patches[(i * 7 + 5) % npatches]};
    Set *s = pcpatchset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("pcpatchset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);

  /* Temporal values of one and of two instants */
  Temporal *temps[MAXV];
  int ntemps = temporal_make(temps, points, npoints, tpcpointinst_v);
  temporal_check("tpcpoint", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);
  ntemps = temporal_make(temps, patches, npatches, tpcpatchinst_v);
  temporal_check("tpcpatch", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);

  for (int i = 0; i < npoints; i++)
    free(points[i]);
  for (int i = 0; i < npatches; i++)
    free(patches[i]);
  meos_pc_schema_clear();
  meos_finalize();
  printf("%ld failure(s)\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
