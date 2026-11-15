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
 * @brief A program that tests how the functions answering the values of a
 * set of the MEOS API report a null count under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. The count a function answers through its
 * last argument is such a pointer: it is written once the values are copied.
 *
 * The program verifies that each of #intset_values, #bigintset_values,
 * #floatset_values, #textset_values, #dateset_values, #tstzset_values,
 * #h3indexset_values, #npointset_values, #poseset_values,
 * #posechainset_values, #quadbinset_values and #s2cellset_values reports a
 * null count by answering NULL and setting #meos_errno, and that a valid call
 * answers the values of the set with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o set_values_validity_test set_values_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_npoint.h>
#include <meos_pose.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/* Check that an accessor reports a null count and answers a valid call; an
 * accessor of a base type passed by reference answers a copy of each value,
 * which the check frees with the array and the set */
#define CHECK_VALUES(func, str, size, byref) \
  do { \
    Set *set = set_in_##func(str); \
    assert(set != NULL); \
    void *none = (void *) func(set, NULL); \
    printf("%s(s, NULL): %s, errno %d\n", #func, none ? "values" : "NULL", \
      meos_errno()); \
    assert(none == NULL); \
    assert(meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    int count = -1; \
    void *values = (void *) func(set, &count); \
    printf("%s(s, &count): %d values, errno %d\n", #func, count, \
      meos_errno()); \
    assert(values != NULL); \
    assert(count == (size)); \
    assert(meos_errno() == 0); \
    if (byref) \
      for (int i = 0; i < count; i++) \
        free(((void **) values)[i]); \
    free(values); \
    free(set); \
  } while (0)

/* The input function of the set each accessor reads */
#define set_in_intset_values intset_in
#define set_in_bigintset_values bigintset_in
#define set_in_floatset_values floatset_in
#define set_in_textset_values textset_in
#define set_in_dateset_values dateset_in
#define set_in_tstzset_values tstzset_in
#define set_in_h3indexset_values h3indexset_in
#define set_in_npointset_values npointset_in
#define set_in_poseset_values poseset_in
#define set_in_posechainset_values posechainset_in
#define set_in_quadbinset_values quadbinset_in
#define set_in_s2cellset_values s2cellset_in

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  CHECK_VALUES(intset_values, "{1, 2, 3}", 3, false);
  CHECK_VALUES(bigintset_values, "{1, 5000000000}", 2, false);
  CHECK_VALUES(floatset_values, "{1.5, 2.5}", 2, false);
  CHECK_VALUES(textset_values, "{\"AAA\", \"BBB\"}", 2, true);
  CHECK_VALUES(dateset_values, "{2001-01-01, 2001-01-02}", 2, false);
  CHECK_VALUES(tstzset_values, "{2001-01-01, 2001-01-02}", 2, false);
  CHECK_VALUES(h3indexset_values, "{880326b885fffff, 880326b88dfffff}", 2,
    false);
  CHECK_VALUES(npointset_values, "{\"Npoint(1,0.5)\", \"Npoint(2,0.5)\"}", 2,
    true);
  CHECK_VALUES(poseset_values,
    "{\"Pose(Point(1 1),0.5)\", \"Pose(Point(2 2),0.5)\"}", 2, true);
  CHECK_VALUES(posechainset_values,
    "{\"PoseChain(Pose(Point(0 0), 0))\", "
      "\"PoseChain(Pose(Point(1 0), 0))\"}", 2, true);
  CHECK_VALUES(quadbinset_values, "{480fffffffffffff, 48427fffffffffff}", 2,
    false);
  CHECK_VALUES(s2cellset_values, "{47c3c3, 54b5c9}", 2, false);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
