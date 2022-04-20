/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file doublen.c
 * @brief Internal types used in particular for computing the average and
 * centroid temporal aggregates.
 *
 * The `double2`, `double3`, and `double4` types are base types composed,
 * respectively, of two, three, and four `double` values. The `tdouble2`,
 * `tdouble3`, and `tdouble4` types are the corresponding temporal types.
 * The in/out functions of all these types are stubs since all access should
 * be internal. These types are used for the transition function of the
 * aggregates, where the first components of the `doubleN` values store the
 * sum and the last one stores the count of the values. The final function
 * computes the average from the `doubleN` values.
*/
#include "general/doublen.h"

/* PostgreSQL */
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double2 value from the double values
 */
void
double2_set(double a, double b, double2 *result)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(result, 0, sizeof(double2));
  result->a = a;
  result->b = b;
}

/**
 * Return the addition of the double2 values
 */
double2 *
double2_add(const double2 *d1, const double2 *d2)
{
  double2 *result = (double2 *) palloc(sizeof(double2));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  return result;
}

/**
 * Return true if the double2 values are equal
 */
bool
double2_eq(const double2 *d1, const double2 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b);
}

#if 0 /* Not used */
/**
 * Return -1, 0, or 1 depending on whether the first double2
 * is less than, equal, or greater than the second one
 */
int
double2_cmp(double2 *d1, double2 *d2)
{
  int cmp = float8_cmp_internal(d1->a, d2->a);
  if (cmp == 0)
    cmp = float8_cmp_internal(d1->b, d2->b);
  return cmp;
}
#endif

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double3 value from the double values
 */
void
double3_set(double a, double b, double c, double3 *result)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(result, 0, sizeof(double3));
  result->a = a;
  result->b = b;
  result->c = c;
}

/**
 * Return the addition of the double3 values
 */
double3 *
double3_add(const double3 *d1, const double3 *d2)
{
  double3 *result = (double3 *) palloc(sizeof(double3));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  result->c = d1->c + d2->c;
  return result;
}

/**
 * Return true if the double3 values are equal
 */
bool
double3_eq(const double3 *d1, const double3 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c);
}

#if 0 /* Not used */
/**
 * Return -1, 0, or 1 depending on whether the first double2
 * is less than, equal, or greater than the second one
 */
int
double3_cmp(double3 *d1, double3 *d2)
{
  int cmp = float8_cmp_internal(d1->a, d2->a);
  if (cmp == 0)
  {
    cmp = float8_cmp_internal(d1->b, d2->b);
    if (cmp == 0)
      cmp = float8_cmp_internal(d1->c, d2->c);
  }
  return cmp;
}
#endif

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double4 value from the double values
 */
void
double4_set(double a, double b, double c, double d, double4 *result)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(result, 0, sizeof(double4));
  result->a = a;
  result->b = b;
  result->c = c;
  result->d = d;
}

/**
 * Return the addition of the double4 values
 */
double4 *
double4_add(const double4 *d1, const double4 *d2)
{
  double4 *result = (double4 *) palloc(sizeof(double4));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  result->c = d1->c + d2->c;
  result->d = d1->d + d2->d;
  return result;
}

/**
 * Return true if the double4 values are equal
 */
bool
double4_eq(const double4 *d1, const double4 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c &&
    d1->d == d2->d);
}

/*****************************************************************************/
