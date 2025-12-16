/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Internal types used in particular for computing the average and
 * centroid temporal aggregates
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
#include "temporal/doublen.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

#include <utils/numeric.h>
#include <utils/jsonb.h>
#include <pgtypes.h>

/*****************************************************************************
 * Double2
 *****************************************************************************/

#if MEOS || DEBUG_BUILD
/**
 * @brief Create a double2 value from the double values
 */
double2 *
double2_make(double a, double b)
{
  /* Note: zero-fill is done in function double2_set */
  double2 *result = palloc(sizeof(double2));
  double2_set(a, b, result);
  return result;
}

/**
 * @brief Output function for double2 values
 */
char *
double2_out(const double2 *d, int maxdd)
{
  assert(d); assert(maxdd >= 0);
  char *astr = float8_out(d->a, maxdd);
  char *bstr = float8_out(d->b, maxdd);
  size_t size = strlen(astr) + strlen(bstr) + 4;
  char *result = palloc(size);
  snprintf(result, size, "(%s,%s)", astr, bstr);
  pfree(astr); pfree(bstr);
  return result;
}
#endif /* MEOS */

/**
 * @brief Set a double2 value from the double values
 */
void
double2_set(double a, double b, double2 *result)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(result, 0, sizeof(double2));
  result->a = a;
  result->b = b;
  return;
}

/**
 * @brief Return the addition of the double2 values
 */
double2 *
double2_add(const double2 *d1, const double2 *d2)
{
  double2 *result = palloc0(sizeof(double2));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  return result;
}

/**
 * @brief Return true if the double2 values are equal
 */
bool
double2_eq(const double2 *d1, const double2 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b);
}

/*****************************************************************************
 * Double3
 *****************************************************************************/

#if MEOS || DEBUG_BUILD
/**
 * @brief Create a double3 value from the double values
 */
double3 *
double3_make(double a, double b, double c)
{
  /* Note: zero-fill is done in function double3_set */
  double3 *result = palloc(sizeof(double3));
  double3_set(a, b, c, result);
  return result;
}

/**
 * @brief Output function for double3 values
 */
char *
double3_out(const double3 *d, int maxdd)
{
  assert(d); assert(maxdd >= 0);
  char *astr = float8_out(d->a, maxdd);
  char *bstr = float8_out(d->b, maxdd);
  char *cstr = float8_out(d->c, maxdd);
  size_t size = strlen(astr) + strlen(bstr) + strlen(cstr) + 5;
  char *result = palloc(size);
  snprintf(result, size, "(%s,%s,%s)", astr, bstr, cstr);
  pfree(astr); pfree(bstr); pfree(cstr);
  return result;
}
#endif /* MEOS */

/**
 * @brief Set a double3 value from the double values
 */
void
double3_set(double a, double b, double c, double3 *result)
{
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(result, 0, sizeof(double3));
  result->a = a;
  result->b = b;
  result->c = c;
  return;
}

/**
 * @brief Return the addition of the double3 values
 */
double3 *
double3_add(const double3 *d1, const double3 *d2)
{
  double3 *result = palloc0(sizeof(double3));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  result->c = d1->c + d2->c;
  return result;
}

/**
 * @brief Return true if the double3 values are equal
 */
bool
double3_eq(const double3 *d1, const double3 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c);
}

/*****************************************************************************
 * Double4
 *****************************************************************************/

#if MEOS || DEBUG_BUILD
/**
 * @brief Create a double2 value from the double values
 */
double4 *
double4_make(double a, double b, double c, double d)
{
  /* Note: zero-fill is done in function double4_set */
  double4 *result = palloc(sizeof(double4));
  double4_set(a, b, c, d, result);
  return result;
}

/**
 * @brief Output function for double4 values
 */
char *
double4_out(const double4 *d, int maxdd)
{
  assert(d); assert(maxdd >= 0);
  char *astr = float8_out(d->a, maxdd);
  char *bstr = float8_out(d->b, maxdd);
  char *cstr = float8_out(d->c, maxdd);
  char *dstr = float8_out(d->d, maxdd);
  size_t size = strlen(astr) + strlen(bstr) + strlen(cstr) + strlen(dstr) + 6;
  char *result = palloc(size);
  snprintf(result, size, "(%s,%s,%s,%s)", astr, bstr, cstr, dstr);
  pfree(astr); pfree(bstr); pfree(cstr); pfree(dstr);
  return result;
}
#endif /* MEOS */

/**
 * @brief Set a double4 value from the double values
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
  return;
}

/**
 * @brief Return the addition of the double4 values
 */
double4 *
double4_add(const double4 *d1, const double4 *d2)
{
  double4 *result = palloc0(sizeof(double4));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  result->c = d1->c + d2->c;
  result->d = d1->d + d2->d;
  return result;
}

/**
 * @brief Return true if the double4 values are equal
 */
bool
double4_eq(const double4 *d1, const double4 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c && d1->d == d2->d);
}

/*****************************************************************************
 * Collinear functions
 *****************************************************************************/

/**
 * @brief Return true if the three double2 values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the timestamps
 * associated to `x1` and `x2` divided by the duration of the timestamps
 * associated to `x1` and `x3`
 * @pre The function supposes that the segments are not constant
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double2_collinear(const double2 *x1, const double2 *x2, const double2 *x3,
  double ratio)
{
  double2 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio;
  return (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON);
}

/**
 * @brief Return true if the three values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the timestamps
 * associated to `x1` and `x2` divided by the duration of the timestamps
 * associated to `x1` and `x3`
 * @pre The function supposes that the segments are not constant
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double3_collinear(const double3 *x1, const double3 *x2, const double3 *x3,
  double ratio)
{
  double3 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio,
  x.c = x1->c + (x3->c - x1->c) * ratio;
  return (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON && fabs(x2->c - x.c) <= MEOS_EPSILON);
}

/**
 * @brief Return true if the three values are collinear
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the timestamps
 * associated to `x1` and `x2` divided by the duration of the timestamps
 * associated to `x1` and `x3`
 * @pre The function supposes that the segments are not constant
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double4_collinear(const double4 *x1, const double4 *x2, const double4 *x3,
  double ratio)
{
  double4 x;
  x.a = x1->a + (x3->a - x1->a) * ratio;
  x.b = x1->b + (x3->b - x1->b) * ratio;
  x.c = x1->c + (x3->c - x1->c) * ratio;
  x.d = x1->d + (x3->d - x1->d) * ratio;
  return (fabs(x2->a - x.a) <= MEOS_EPSILON &&
    fabs(x2->b - x.b) <= MEOS_EPSILON && fabs(x2->c - x.c) <= MEOS_EPSILON &&
    fabs(x2->d - x.d) <= MEOS_EPSILON);
}

/*****************************************************************************
 * Interpolate functions
 *****************************************************************************/

/**
 * @brief Return a double2 interpolated from a double2 segment with respect to
 * a fraction of its total length
 * @param[in] start,end Values defining the segment
 * @param[in] ratio Value between 0 and 1 representing the fraction of the
 * total length of the segment where the value must be located
 * @note Function used for determining the value of a segment at a timestamp
 */
double2 *
double2segm_interpolate(const double2 *start, const double2 *end,
  long double ratio)
{
  assert(ratio >= 0.0 || ratio <= 1.0);
  double2 *result = palloc(sizeof(double2));
  result->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
  result->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
  return result;
}

/**
 * @brief Return a double3 interpolated from a double3 segment with respect to
 * a fraction of its total length
 * @param[in] start,end Values defining the segment
 * @param[in] ratio Value between 0 and 1 representing the fraction of the
 * total length of the segment where the value must be located
 * @note Function used for determining the value of a segment at a timestamp
 */
double3 *
double3segm_interpolate(const double3 *start, const double3 *end,
  long double ratio)
{
  assert(ratio >= 0.0 || ratio <= 1.0);
  double3 *result = palloc(sizeof(double3));
  result->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
  result->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
  result->c = start->c + (double) ((long double)(end->c - start->c) * ratio);
  return result;
}

/**
 * @brief Return a double4 interpolated from a double4 segment with respect to
 * a fraction of its total length
 * @param[in] start,end Values defining the segment
 * @param[in] ratio Value between 0 and 1 representing the fraction of the
 * total length of the segment where the value must be located
 * @note Function used for determining the value of a segment at a timestamp
 */
double4 *
double4segm_interpolate(const double4 *start, const double4 *end,
  long double ratio)
{
  assert(ratio >= 0.0 || ratio <= 1.0);
  double4 *result = palloc(sizeof(double4));
  result->a = start->a + (double) ((long double)(end->a - start->a) * ratio);
  result->b = start->b + (double) ((long double)(end->b - start->b) * ratio);
  result->c = start->c + (double) ((long double)(end->c - start->c) * ratio);
  result->d = start->d + (double) ((long double)(end->d - start->d) * ratio);
  return result;
}

/*****************************************************************************/

