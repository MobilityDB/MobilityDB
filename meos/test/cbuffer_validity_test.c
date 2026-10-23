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
 * @brief A program that tests how the ever/always relationships of a
 * geometry and a temporal circular buffer report an erroneous argument under
 * the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer or with operands in
 * two spatial reference systems receives an error it can raise in its host
 * language rather than a crash or an answer.
 *
 * The program verifies that #econtains_geo_tcbuffer, #acontains_geo_tcbuffer,
 * #ecovers_geo_tcbuffer and #acovers_geo_tcbuffer report a null geometry and a geometry whose SRID
 * differs from the one of the temporal circular buffer by returning -1 and
 * setting #meos_errno, as #econtains_tcbuffer_geo does, and that a valid call
 * still answers with no error left behind. It also verifies that each
 * commutative relationship of a geometry or a circular buffer and a temporal
 * circular buffer answers what the same relationship in the other argument
 * order answers.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_validity_test cbuffer_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>

/* The relationships of a geometry and a temporal circular buffer tested */
typedef int (*geo_tcbuffer_fn)(const GSERIALIZED *, const Temporal *);
typedef int (*tcbuffer_geo_fn)(const Temporal *, const GSERIALIZED *);
typedef int (*cbuffer_tcbuffer_fn)(const Cbuffer *, const Temporal *);
typedef int (*tcbuffer_cbuffer_fn)(const Temporal *, const Cbuffer *);

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp = tcbuffer_in("[Cbuffer(Point(1 1),0.5)@2001-01-01, "
    "Cbuffer(Point(2 2),0.5)@2001-01-02]");
  GSERIALIZED *geo = geom_in("Polygon((0 0,0 5,5 5,5 0,0 0))", -1);
  GSERIALIZED *other = geom_in("SRID=3857;Polygon((0 0,0 5,5 5,5 0,0 0))",
    -1);
  meos_errno_reset();

  static const char *names[] = {"econtains_geo_tcbuffer",
    "acontains_geo_tcbuffer", "ecovers_geo_tcbuffer", "acovers_geo_tcbuffer"};
  geo_tcbuffer_fn fns[] = {&econtains_geo_tcbuffer, &acontains_geo_tcbuffer,
    &ecovers_geo_tcbuffer, &acovers_geo_tcbuffer};
  for (int i = 0; i < 4; i++)
  {
    /* A null geometry is reported rather than read */
    int result = fns[i](NULL, temp);
    printf("%s(NULL, temp): %d, errno %d\n", names[i], result, meos_errno());
    assert(result == -1);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG);
    meos_errno_reset();

    /* Operands in two spatial reference systems are reported rather than
     * answered */
    result = fns[i](other, temp);
    printf("%s(SRID 3857, SRID 0): %d, errno %d\n", names[i], result,
      meos_errno());
    assert(result == -1);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();

    /* A valid call still answers, and the guard leaves no error behind */
    result = fns[i](geo, temp);
    printf("%s(geo, temp): %d, errno %d\n", names[i], result, meos_errno());
    assert(result == 0 || result == 1);
    assert(meos_errno() == 0);
  }

  /* The sibling taking its operands in the other order already reports a
   * null geometry, and stays as it is */
  int result = econtains_tcbuffer_geo(temp, NULL);
  printf("econtains_tcbuffer_geo(temp, NULL): %d, errno %d\n", result,
    meos_errno());
  assert(result == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  /* A geometry containing the buffer at every instant ever and always
   * contains it */
  GSERIALIZED *big = geom_in("Polygon((-5 -5,-5 10,10 10,10 -5,-5 -5))", -1);
  result = econtains_geo_tcbuffer(big, temp);
  printf("econtains_geo_tcbuffer(big, temp): %d\n", result);
  assert(result == 1);
  assert(acontains_geo_tcbuffer(big, temp) == 1);
  free(big);
  meos_errno_reset();

  /* Two buffers that touch at every instant always touch, two buffers that
   * never meet never touch */
  Temporal *tangent1 = tcbuffer_in("[Cbuffer(Point(0 0),1)@2001-01-01, "
    "Cbuffer(Point(0 1),1)@2001-01-02]");
  Temporal *tangent2 = tcbuffer_in("[Cbuffer(Point(2 0),1)@2001-01-01, "
    "Cbuffer(Point(2 1),1)@2001-01-02]");
  Temporal *apart = tcbuffer_in("[Cbuffer(Point(9 0),1)@2001-01-01, "
    "Cbuffer(Point(9 1),1)@2001-01-02]");
  int ever = etouches_tcbuffer_tcbuffer(tangent1, tangent2);
  int always = atouches_tcbuffer_tcbuffer(tangent1, tangent2);
  printf("etouches/atouches_tcbuffer_tcbuffer(tangent): %d %d\n", ever,
    always);
  assert(ever == 1 && always == 1);
  ever = etouches_tcbuffer_tcbuffer(tangent1, apart);
  always = atouches_tcbuffer_tcbuffer(tangent1, apart);
  printf("etouches/atouches_tcbuffer_tcbuffer(apart): %d %d\n", ever,
    always);
  assert(ever == 0 && always == 0);
  assert(meos_errno() == 0);
  free(tangent1); free(tangent2); free(apart);

  free(temp); free(geo); free(other);
  meos_errno_reset();

  /* A circular buffer set is built from an array of circular buffers the
   * caller fills, whose varlena header the set constructor sets, each radius
   * checked as cbuffer_make checks it */
  Cbuffer cbs[2] = {{0, 3857, 0.5, 1.0, 2.0}, {0, 3857, 1.5, 3.0, 4.0}};
  Set *s = cbufferset_make(cbs, 2);
  printf("cbufferset_make(filled array): %s, errno %d\n",
    s ? "a value" : "NULL", meos_errno());
  assert(s != NULL);
  assert(meos_errno() == 0);
  Cbuffer *start = cbufferset_start_value(s);
  Cbuffer *made = cbuffer_in("SRID=3857;Cbuffer(Point(1 2),0.5)");
  assert(cbuffer_eq(start, made));
  free(start); free(made); free(s);

  cbs[1].radius = -1.0;
  s = cbufferset_make(cbs, 2);
  printf("cbufferset_make(radius -1): %s, errno %d\n",
    s ? "a value" : "NULL", meos_errno());
  assert(s == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();

  /* A commutative relationship of a static value and a temporal circular
   * buffer answers what the same relationship of the temporal circular buffer
   * and the static value answers, over a value apart from the buffer, one on
   * its boundary at the first instant and one overlapping it */
  static const char *swap_names[] = {"edisjoint", "adisjoint", "eintersects",
    "aintersects", "etouches", "atouches"};
  geo_tcbuffer_fn geo_first[] = {&edisjoint_geo_tcbuffer,
    &adisjoint_geo_tcbuffer, &eintersects_geo_tcbuffer,
    &aintersects_geo_tcbuffer, &etouches_geo_tcbuffer, &atouches_geo_tcbuffer};
  tcbuffer_geo_fn geo_second[] = {&edisjoint_tcbuffer_geo,
    &adisjoint_tcbuffer_geo, &eintersects_tcbuffer_geo,
    &aintersects_tcbuffer_geo, &etouches_tcbuffer_geo, &atouches_tcbuffer_geo};
  cbuffer_tcbuffer_fn cb_first[] = {&edisjoint_cbuffer_tcbuffer,
    &adisjoint_cbuffer_tcbuffer, &eintersects_cbuffer_tcbuffer,
    &aintersects_cbuffer_tcbuffer, &etouches_cbuffer_tcbuffer,
    &atouches_cbuffer_tcbuffer};
  tcbuffer_cbuffer_fn cb_second[] = {&edisjoint_tcbuffer_cbuffer,
    &adisjoint_tcbuffer_cbuffer, &eintersects_tcbuffer_cbuffer,
    &aintersects_tcbuffer_cbuffer, &etouches_tcbuffer_cbuffer,
    &atouches_tcbuffer_cbuffer};
  static const char *geo_wkt[] = {"Point(9 9)", "Point(1 1.5)",
    "Polygon((1 1,1 3,3 3,3 1,1 1))"};
  static const char *cb_wkt[] = {"Cbuffer(Point(9 9),0.5)",
    "Cbuffer(Point(1 2),0.5)", "Cbuffer(Point(1.5 1.5),0.5)"};
  Temporal *moving = tcbuffer_in("[Cbuffer(Point(1 1),0.5)@2001-01-01, "
    "Cbuffer(Point(2 2),0.5)@2001-01-02]");
  int ones = 0, zeros = 0;
  for (int c = 0; c < 3; c++)
  {
    GSERIALIZED *g = geom_in(geo_wkt[c], -1);
    Cbuffer *cb = cbuffer_in(cb_wkt[c]);
    for (int i = 0; i < 6; i++)
    {
      int first = geo_first[i](g, moving), second = geo_second[i](moving, g);
      printf("%s_geo_tcbuffer(%s): %d, %s_tcbuffer_geo: %d\n", swap_names[i],
        geo_wkt[c], first, swap_names[i], second);
      assert(first == second);
      ones += (first == 1); zeros += (first == 0);
      first = cb_first[i](cb, moving); second = cb_second[i](moving, cb);
      printf("%s_cbuffer_tcbuffer(%s): %d, %s_tcbuffer_cbuffer: %d\n",
        swap_names[i], cb_wkt[c], first, swap_names[i], second);
      assert(first == second);
      ones += (first == 1); zeros += (first == 0);
    }
    for (int d = 0; d < 2; d++)
    {
      double dist = d ? 8.0 : 0.25;
      assert(edwithin_geo_tcbuffer(g, moving, dist) ==
        edwithin_tcbuffer_geo(moving, g, dist));
      assert(adwithin_geo_tcbuffer(g, moving, dist) ==
        adwithin_tcbuffer_geo(moving, g, dist));
      assert(edwithin_cbuffer_tcbuffer(cb, moving, dist) ==
        edwithin_tcbuffer_cbuffer(moving, cb, dist));
      assert(adwithin_cbuffer_tcbuffer(cb, moving, dist) ==
        adwithin_tcbuffer_cbuffer(moving, cb, dist));
      Temporal *tfirst = tdwithin_cbuffer_tcbuffer(cb, moving, dist);
      Temporal *tsecond = tdwithin_tcbuffer_cbuffer(moving, cb, dist);
      assert(tfirst && tsecond && temporal_eq(tfirst, tsecond));
      free(tfirst); free(tsecond);
    }
    free(g); free(cb);
  }
  /* The cases reach both answers, so the equalities are not those of a
   * single constant */
  printf("swapped relationships: %d answers 1, %d answers 0\n", ones, zeros);
  assert(ones > 0 && zeros > 0);
  free(moving);
  assert(meos_errno() == 0);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
