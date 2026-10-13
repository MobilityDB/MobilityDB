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
 * @brief A program that tests the value restrictions of the temporal big
 * integers, cell indices and pose chains
 * @details Each of tbigint, th3index, tquadbin, ts2cell and tposechain
 * restricts a temporal value to a base value, and to its complement, through
 * a typed public function, as tint_at_value and tint_minus_value do for a
 * temporal integer. Over a discrete sequence taking a value at its first and
 * last instants and another between them, the program verifies that each
 * answers what the set restriction answers over the one-value set, that a
 * value the sequence never takes restricts it to nothing with no error, and
 * that a null temporal value, a temporal integer and an invalid cell are
 * reported with MEOS_ERR_INVALID_ARG, MEOS_ERR_INVALID_ARG_TYPE and
 * MEOS_ERR_INVALID_ARG_VALUE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o value_restrict_test value_restrict_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_pose.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/* Return true when two temporal values are both absent or equal */
static bool
same(const Temporal *temp1, const Temporal *temp2)
{
  if (! temp1 || ! temp2)
    return temp1 == temp2;
  return temporal_eq(temp1, temp2);
}

/* Return the number of instants of a temporal value, 0 for none */
static int
num_instants(const Temporal *temp)
{
  return temp ? temporal_num_instants(temp) : 0;
}

/* The restriction to a value and to its complement answer what the set
 * restriction answers over the one-value set, with the instant counts given,
 * and leave no error behind */
static void
check(const char *name, const Temporal *temp, const Set *set, Temporal *at,
  Temporal *minus, int at_count, int minus_count)
{
  Temporal *set_at = temporal_at_values(temp, set);
  Temporal *set_minus = temporal_minus_values(temp, set);
  printf("%s: at %d instant(s), minus %d instant(s), errno %d\n", name,
    num_instants(at), num_instants(minus), meos_errno());
  assert(meos_errno() == 0);
  assert(same(at, set_at) && same(minus, set_minus));
  assert(num_instants(at) == at_count && num_instants(minus) == minus_count);
  free(at); free(minus); free(set_at); free(set_minus);
}

/* A refused call answers NULL with the given error, which is then reset */
static void
refused(const char *name, const Temporal *result, int err)
{
  printf("%s: %s, errno %d\n", name, result ? "a value" : "NULL",
    meos_errno());
  assert(result == NULL && meos_errno() == err);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tint = tint_in("{1@2001-01-01, 2@2001-01-02}");
  assert(tint);

  /* Temporal big integer, a value beyond the range of an integer */
  Temporal *tbi = tbigint_in(
    "{5000000000@2001-01-01, 7@2001-01-02, 5000000000@2001-01-03}");
  Set *bis = bigint_to_set(5000000000);
  check("tbigint", tbi, bis, tbigint_at_value(tbi, 5000000000),
    tbigint_minus_value(tbi, 5000000000), 2, 1);
  free(bis);
  bis = bigint_to_set(8);
  check("tbigint absent", tbi, bis, tbigint_at_value(tbi, 8),
    tbigint_minus_value(tbi, 8), 0, 3);
  refused("tbigint_at_value(NULL)", tbigint_at_value(NULL, 7),
    MEOS_ERR_INVALID_ARG);
  refused("tbigint_minus_value(tint)", tbigint_minus_value(tint, 7),
    MEOS_ERR_INVALID_ARG_TYPE);
  free(bis); free(tbi);

  /* Temporal H3 cell */
  H3Index h3a = h3index_in("8a04c22a5b5ffff");
  H3Index h3b = h3index_in("8a0a4e06618ffff");
  Temporal *th3 = th3index_in("{8a04c22a5b5ffff@2001-01-01, "
    "8a0a4e06618ffff@2001-01-02, 8a04c22a5b5ffff@2001-01-03}");
  Set *h3s = h3index_to_set(h3a);
  check("th3index", th3, h3s, th3index_at_value(th3, h3a),
    th3index_minus_value(th3, h3a), 2, 1);
  free(h3s);
  h3s = h3index_to_set(h3b);
  check("th3index other", th3, h3s, th3index_at_value(th3, h3b),
    th3index_minus_value(th3, h3b), 1, 2);
  refused("th3index_at_value(NULL)", th3index_at_value(NULL, h3a),
    MEOS_ERR_INVALID_ARG);
  refused("th3index_at_value(tint)", th3index_at_value(tint, h3a),
    MEOS_ERR_INVALID_ARG_TYPE);
  refused("th3index_minus_value(0)", th3index_minus_value(th3, 0),
    MEOS_ERR_INVALID_ARG_VALUE);
  free(h3s); free(th3);

  /* Temporal QUADBIN cell */
  Quadbin qba = quadbin_in("4856dbffffffffff");
  Temporal *tqb = tquadbin_in("{4856dbffffffffff@2001-01-01, "
    "48636bffffffffff@2001-01-02, 4856dbffffffffff@2001-01-03}");
  Set *qbs = quadbin_to_set(qba);
  check("tquadbin", tqb, qbs, tquadbin_at_value(tqb, qba),
    tquadbin_minus_value(tqb, qba), 2, 1);
  refused("tquadbin_at_value(NULL)", tquadbin_at_value(NULL, qba),
    MEOS_ERR_INVALID_ARG);
  refused("tquadbin_at_value(tint)", tquadbin_at_value(tint, qba),
    MEOS_ERR_INVALID_ARG_TYPE);
  refused("tquadbin_minus_value(0)", tquadbin_minus_value(tqb, 0),
    MEOS_ERR_INVALID_ARG_VALUE);
  free(qbs); free(tqb);

  /* Temporal S2 cell */
  S2CellId s2a = s2cell_in("47c3c3");
  Temporal *ts2 = ts2cell_in(
    "{47c3c3@2001-01-01, 54b5c9@2001-01-02, 47c3c3@2001-01-03}");
  Set *s2s = s2cell_to_set(s2a);
  check("ts2cell", ts2, s2s, ts2cell_at_value(ts2, s2a),
    ts2cell_minus_value(ts2, s2a), 2, 1);
  refused("ts2cell_at_value(NULL)", ts2cell_at_value(NULL, s2a),
    MEOS_ERR_INVALID_ARG);
  refused("ts2cell_at_value(tint)", ts2cell_at_value(tint, s2a),
    MEOS_ERR_INVALID_ARG_TYPE);
  refused("ts2cell_minus_value(0)", ts2cell_minus_value(ts2, 0),
    MEOS_ERR_INVALID_ARG_VALUE);
  free(s2s); free(ts2);

  /* Temporal pose chain */
  const char *pca_str = "POSECHAIN(POSE (0101000020E40E0000687CA91F3D902040"
    "72CCDE836F715340, 0.000142862782913))";
  const char *pcb_str = "POSECHAIN(POSE (0101000020E40E00009E28652B2BA640C0"
    "48241385189632C0, 0.011590135461471))";
  char str[1024];
  snprintf(str, sizeof(str), "{%s@2001-01-01, %s@2001-01-02, %s@2001-01-03}",
    pca_str, pcb_str, pca_str);
  PoseChain *pca = posechain_in(pca_str);
  Temporal *tpc = tposechain_in(str);
  Set *pcs = posechain_to_set(pca);
  check("tposechain", tpc, pcs, tposechain_at_value(tpc, pca),
    tposechain_minus_value(tpc, pca), 2, 1);
  refused("tposechain_at_value(NULL)", tposechain_at_value(NULL, pca),
    MEOS_ERR_INVALID_ARG);
  refused("tposechain_minus_value(NULL value)",
    tposechain_minus_value(tpc, NULL), MEOS_ERR_INVALID_ARG);
  refused("tposechain_at_value(tint)", tposechain_at_value(tint, pca),
    MEOS_ERR_INVALID_ARG_TYPE);
  free(pcs); free(tpc); free(pca);

  free(tint);
  /* Finalize MEOS */
  meos_finalize();
  return 0;
}
