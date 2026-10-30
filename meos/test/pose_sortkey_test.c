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
 * @brief A program that tests that the comparison of poses and of pose chains
 * is a total order consistent with their equality, and that their sort keys
 * order the sets and the temporal values of these types as their comparison
 * does
 * @details A sort key is the string of bytes whose unsigned byte order is the
 * order of a comparison and whose byte equality is the equality of the type,
 * and it exists only when the comparison is a total order that holds two
 * values equal exactly when the equality of the type does. The program reads
 * poses that are planar or spatial, geodetic or not, of two SRIDs, whose
 * positions are negative, zero of both signs or large and whose angles and
 * quaternions vary, and pose chains of one to three of them, and checks:
 * - which of the special inputs the input functions accept, a NaN coordinate
 *   being refused as every MEOS input refuses it;
 * - over every pair, that the comparison answers 0 exactly when the equality
 *   holds, and that swapping the operands swaps the sign;
 * - over every triple, that the comparison is transitive;
 * - over every pair of sets of one and of two values and of temporal values
 *   of one and of two instants, that the byte order of the keys is the order
 *   of #set_cmp and #temporal_cmp over the pairs these comparisons accept;
 * - over pairs of temporal rigid geometries with the same poses and two
 *   reference geometries, whether #temporal_cmp and #temporal_eq agree,
 *   which it reports, and that a temporal rigid geometry has no sort key,
 *   since #temporal_cmp reads none of its reference geometry.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o pose_sortkey_test pose_sortkey_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_pose.h>
#include <meos_rgeo.h>

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

typedef int (*cmpfn)(const void *, const void *);
typedef bool (*eqfn)(const void *, const void *);

/* Check that a comparison is antisymmetric, transitive and consistent with
 * the equality of the type over every pair and every triple of values */
static void
base_check(const char *name, void **v, int n, cmpfn cmp, eqfn eq)
{
  long pairs = 0, inconsistent = 0, asymmetric = 0, triples = 0,
    intransitive = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      int c = order_sign(cmp(v[i], v[j]));
      if ((c == 0) != eq(v[i], v[j]))
        inconsistent++;
      if (c != -order_sign(cmp(v[j], v[i])))
        asymmetric++;
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
    "equality, %ld whose swapped comparison is not the opposite; %ld ordered "
    "triples, %ld not transitive\n", name, pairs, inconsistent, asymmetric,
    triples, intransitive);
  failures += inconsistent + asymmetric + intransitive;
}

static int
pose_cmp_v(const void *a, const void *b)
{
  return pose_cmp((const Pose *) a, (const Pose *) b);
}

static bool
pose_eq_v(const void *a, const void *b)
{
  return pose_eq((const Pose *) a, (const Pose *) b);
}

static int
posechain_cmp_v(const void *a, const void *b)
{
  return posechain_cmp((const PoseChain *) a, (const PoseChain *) b);
}

static bool
posechain_eq_v(const void *a, const void *b)
{
  return posechain_eq((const PoseChain *) a, (const PoseChain *) b);
}

/* Read a value, report whether the input function accepts it, and keep it */
static void *
input_add(void **v, int *n, const char *str, void *(*in)(const char *),
  bool report)
{
  meos_errno_reset();
  void *value = in(str);
  if (report)
    printf("  %-48s %s\n", str, value ? "accepted" : "refused");
  meos_errno_reset();
  if (value && *n < MAXV)
    v[(*n)++] = value;
  else
    free(value);
  return value;
}

static void *
pose_in_v(const char *str)
{
  return pose_in(str);
}

static void *
posechain_in_v(const char *str)
{
  return posechain_in(str);
}

/* Check that the keys of every pair of sets order them as #set_cmp does */
static void
set_check(const char *name, Set **sets, int n)
{
  long pairs = 0, bad = 0, refused = 0, mixed = 0;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      size_t na, nb;
      meos_errno_reset();
      uint8_t *ka = set_sortkey(sets[i], &na);
      uint8_t *kb = set_sortkey(sets[j], &nb);
      int c = set_cmp(sets[i], sets[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      /* The comparison refuses two values of different SRIDs */
      else if (c == INT_MAX)
        mixed++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld set pairs, %ld without a key, %ld of different SRIDs, "
    "%ld ordered or equated otherwise than set_cmp\n", name, pairs, refused,
    mixed, bad);
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
      int c = temporal_cmp(temps[i], temps[j]);
      meos_errno_reset();
      if (! ka || ! kb)
        refused++;
      /* The comparison refuses two values of different SRIDs */
      else if (c == INT_MAX)
        mixed++;
      else if (sortkey_order(ka, na, kb, nb) != order_sign(c))
        bad++;
      free(ka); free(kb);
      pairs++;
    }
  printf("%s: %ld temporal pairs, %ld without a key, %ld of different "
    "SRIDs, %ld ordered or equated otherwise than temporal_cmp\n", name,
    pairs, refused, mixed, bad);
  failures += refused + bad;
}

/*****************************************************************************/

int
main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Poses: the special inputs, then a grid */
  void *poses[MAXV];
  int nposes = 0;
  printf("Pose inputs:\n");
  const char *pose_special[] = {
    "Pose(Point(-0 1), 0.5)", "Pose(Point(1 1), -0)",
    "Pose(Point(NaN 1), 0.5)", "Pose(Point Z(1 1 NaN), 1, 0, 0, 0)",
    "Pose(Point Z(1 1 1), -0, 1, 0, 0)", "Pose(Point(Infinity 1), 0.5)",
  };
  for (size_t i = 0; i < sizeof(pose_special) / sizeof(char *); i++)
    input_add(poses, &nposes, pose_special[i], pose_in_v, true);
  const char *prefixes[] = {"Pose", "SRID=4326;GeodPose", "SRID=3857;Pose"};
  const char *coords[] = {"-1e300", "-2", "-0", "0", "0.5", "256"};
  const char *angles[] = {"-3", "0", "0.5", "1.5707963267948966"};
  const char *quats[] = {"1, 0, 0, 0", "0, 1, 0, 0",
    "0.7071067811865476, 0.7071067811865476, 0, 0"};
  char str[512];
  for (int p = 0; p < 3; p++)
    for (int x = 0; x < 6; x++)
      for (int y = 0; y < 6; y += 3)
      {
        for (int a = 0; a < 4; a++)
        {
          snprintf(str, sizeof(str), "%s(Point(%s %s), %s)", prefixes[p],
            coords[x], coords[y], angles[a]);
          input_add(poses, &nposes, str, pose_in_v, false);
        }
        for (int q = 0; q < 3; q++)
        {
          snprintf(str, sizeof(str), "%s(Point Z(%s %s 1), %s)", prefixes[p],
            coords[x], coords[y], quats[q]);
          input_add(poses, &nposes, str, pose_in_v, false);
        }
      }
  base_check("pose", poses, nposes, pose_cmp_v, pose_eq_v);

  /* Pose chains of one to three planar poses, and of spatial ones */
  void *chains[MAXV];
  int nchains = 0;
  const char *links[] = {"Pose(Point(0 0), 0)", "Pose(Point(-0 0), 0.5)",
    "Pose(Point(1 0), 0)", "Pose(Point(1 0), 0.5)", "Pose(Point(2 1), -3)",
    "Pose(Point(1e300 0), 0)"};
  for (int i = 0; i < 6; i++)
  {
    snprintf(str, sizeof(str), "PoseChain(%s)", links[i]);
    input_add(chains, &nchains, str, posechain_in_v, false);
    for (int j = 0; j < 6; j++)
    {
      snprintf(str, sizeof(str), "PoseChain(%s, %s)", links[i], links[j]);
      input_add(chains, &nchains, str, posechain_in_v, false);
      snprintf(str, sizeof(str), "PoseChain(%s, %s, %s)", links[i], links[j],
        links[(i + j) % 6]);
      input_add(chains, &nchains, str, posechain_in_v, false);
    }
  }
  input_add(chains, &nchains, "SRID=4326;PoseChain(Pose(Point(0 0), 0))",
    posechain_in_v, false);
  input_add(chains, &nchains, "PoseChain(Pose(Point Z(1 1 1), 1, 0, 0, 0), "
    "Pose(Point Z(1 1 2), 0, 1, 0, 0))", posechain_in_v, false);
  base_check("posechain", chains, nchains, posechain_cmp_v, posechain_eq_v);

  /* Sets of one and of two poses, and of one and of two pose chains */
  Set *sets[MAXV];
  int nsets = 0;
  for (int i = 0; i < nposes && nsets < MAXV - 1; i += 3)
  {
    const Pose *one[1] = {(Pose *) poses[i]};
    Set *s = poseset_make(one, 1);
    if (s)
      sets[nsets++] = s;
    const Pose *two[2] = {(Pose *) poses[i],
      (Pose *) poses[(i * 7 + 5) % nposes]};
    s = poseset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("poseset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);
  nsets = 0;
  for (int i = 0; i < nchains && nsets < MAXV - 1; i += 2)
  {
    const PoseChain *one[1] = {(PoseChain *) chains[i]};
    Set *s = posechainset_make(one, 1);
    if (s)
      sets[nsets++] = s;
    const PoseChain *two[2] = {(PoseChain *) chains[i],
      (PoseChain *) chains[(i * 7 + 5) % nchains]};
    s = posechainset_make(two, 2);
    if (s)
      sets[nsets++] = s;
    meos_errno_reset();
  }
  set_check("posechainset", sets, nsets);
  for (int i = 0; i < nsets; i++)
    free(sets[i]);

  /* Temporal poses and pose chains of one and of two instants */
  Temporal *temps[MAXV];
  int ntemps = 0;
  for (int i = 0; i < nposes && ntemps < MAXV - 1; i += 5)
  {
    char *v1 = pose_out((Pose *) poses[i], 17);
    char *v2 = pose_out((Pose *) poses[(i * 7 + 5) % nposes], 17);
    snprintf(str, sizeof(str), "%s@2001-01-01", v1);
    Temporal *t = tpose_in(str);
    if (t)
      temps[ntemps++] = t;
    snprintf(str, sizeof(str), "Interp=Step;[%s@2001-01-01, %s@2001-01-02]",
      v1, v2);
    t = tpose_in(str);
    if (t)
      temps[ntemps++] = t;
    meos_errno_reset();
    free(v1); free(v2);
  }
  temporal_check("tpose", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);
  ntemps = 0;
  for (int i = 0; i < nchains && ntemps < MAXV - 1; i += 3)
  {
    char *v1 = posechain_out((PoseChain *) chains[i], 17);
    char *v2 = posechain_out((PoseChain *) chains[(i * 7 + 5) % nchains], 17);
    snprintf(str, sizeof(str), "%s@2001-01-01", v1);
    Temporal *t = tposechain_in(str);
    if (t)
      temps[ntemps++] = t;
    snprintf(str, sizeof(str), "Interp=Step;[%s@2001-01-01, %s@2001-01-02]",
      v1, v2);
    t = tposechain_in(str);
    if (t)
      temps[ntemps++] = t;
    meos_errno_reset();
    free(v1); free(v2);
  }
  temporal_check("tposechain", temps, ntemps);
  for (int i = 0; i < ntemps; i++)
    free(temps[i]);

  /* Temporal rigid geometries with the same poses and two reference
   * geometries: temporal_cmp reads no reference geometry, so it and
   * temporal_eq disagree, which the program reports, and a temporal rigid
   * geometry must have no sort key */
  const char *shapes[] = {"Polygon((0 0, 1 0, 1 1, 0 1, 0 0))",
    "Polygon((0 0, 2 0, 2 2, 0 2, 0 0))"};
  const char *motions[] = {"Pose(Point(5 5), 0)@2001-01-01",
    "[Pose(Point(0 0), 0)@2001-01-01, Pose(Point(10 0), 0)@2001-01-02]"};
  long rpairs = 0, rinconsistent = 0, rkeyed = 0;
  for (int m = 0; m < 2; m++)
  {
    Temporal *r[2];
    for (int s = 0; s < 2; s++)
    {
      snprintf(str, sizeof(str), "%s;%s", shapes[s], motions[m]);
      r[s] = trgeometry_in(str);
    }
    if (r[0] && r[1])
    {
      bool eq = temporal_eq(r[0], r[1]);
      int c = temporal_cmp(r[0], r[1]);
      if ((c == 0) != eq)
        rinconsistent++;
      size_t nk;
      meos_errno_reset();
      uint8_t *k = temporal_sortkey(r[0], &nk);
      if (k)
        rkeyed++;
      free(k);
      rpairs++;
    }
    meos_errno_reset();
    free(r[0]); free(r[1]);
  }
  printf("trgeometry: %ld pairs of two reference geometries, %ld whose "
    "comparison answers 0 otherwise than their equality, %ld with a sort "
    "key\n", rpairs, rinconsistent, rkeyed);
  failures += rkeyed;

  for (int i = 0; i < nposes; i++)
    free(poses[i]);
  for (int i = 0; i < nchains; i++)
    free(chains[i]);
  printf("%ld check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
