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
 * @brief Internal types used in particular for computing the average and
 * centroid temporal aggregates
 * @details The `double2`, `double3`, and `double4` types are base types
 * composed,
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
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/tsequence.h"

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
 * @details A value holds the sum of the values aggregated and their count, or,
 * where that sum would exceed the largest double, their average and the
 * count negated. Two sums are added while their sum is finite; otherwise the
 * two averages are weighted by their shares of the count, which never
 * exceeds the largest of the values aggregated.
 */
double2 *
double2_add(const double2 *d1, const double2 *d2)
{
  double2 *result = palloc0(sizeof(double2));
  if (d1->b > 0 && d2->b > 0)
  {
    result->a = d1->a + d2->a;
    result->b = d1->b + d2->b;
    if (isfinite(result->a))
      return result;
  }
  double n1 = fabs(d1->b), n2 = fabs(d2->b), n = n1 + n2;
  double w1 = n1 / n, w2 = n2 / n;
  double m1 = d1->b > 0 ? d1->a / n1 : d1->a;
  double m2 = d2->b > 0 ? d2->a / n2 : d2->a;
  result->a = m1 * w1 + m2 * w2;
  result->b = -n;
  return result;
}

/**
 * @brief Return the average held by a double2 value of #double2_add
 */
double
double2_avg(const double2 *d)
{
  return d->b > 0 ? d->a / d->b : d->a;
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
 * @details As #double2_add, with the count in @p c
 */
double3 *
double3_add(const double3 *d1, const double3 *d2)
{
  double3 *result = palloc0(sizeof(double3));
  if (d1->c > 0 && d2->c > 0)
  {
    result->a = d1->a + d2->a;
    result->b = d1->b + d2->b;
    result->c = d1->c + d2->c;
    if (isfinite(result->a) && isfinite(result->b))
      return result;
  }
  double n1 = fabs(d1->c), n2 = fabs(d2->c), n = n1 + n2;
  double w1 = n1 / n, w2 = n2 / n;
  double s1 = d1->c > 0 ? n1 : 1.0, s2 = d2->c > 0 ? n2 : 1.0;
  result->a = (d1->a / s1) * w1 + (d2->a / s2) * w2;
  result->b = (d1->b / s1) * w1 + (d2->b / s2) * w2;
  result->c = -n;
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
 * @details As #double2_add, with the count in @p d
 */
double4 *
double4_add(const double4 *d1, const double4 *d2)
{
  double4 *result = palloc0(sizeof(double4));
  if (d1->d > 0 && d2->d > 0)
  {
    result->a = d1->a + d2->a;
    result->b = d1->b + d2->b;
    result->c = d1->c + d2->c;
    result->d = d1->d + d2->d;
    if (isfinite(result->a) && isfinite(result->b) && isfinite(result->c))
      return result;
  }
  double n1 = fabs(d1->d), n2 = fabs(d2->d), n = n1 + n2;
  double w1 = n1 / n, w2 = n2 / n;
  double s1 = d1->d > 0 ? n1 : 1.0, s2 = d2->d > 0 ? n2 : 1.0;
  result->a = (d1->a / s1) * w1 + (d2->a / s2) * w2;
  result->b = (d1->b / s1) * w1 + (d2->b / s2) * w2;
  result->c = (d1->c / s1) * w1 + (d2->c / s2) * w2;
  result->d = -n;
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
 * @details Every component is collinear, as #float_collinear decides it
 * @param[in] x1,x2,x3 Values
 * @param[in] t1,t2,t3 Timestamps of the values, in increasing order
 * @param[in] constructed True when the middle value is constructed, false
 * when the three values are input values
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double2_collinear(const double2 *x1, const double2 *x2, const double2 *x3,
  TimestampTz t1, TimestampTz t2, TimestampTz t3, bool constructed)
{
  return float_collinear(x1->a, x2->a, x3->a, t1, t2, t3, constructed) &&
    float_collinear(x1->b, x2->b, x3->b, t1, t2, t3, constructed);
}

/**
 * @brief Return true if the three double3 values are collinear
 * @details Every component is collinear, as #float_collinear decides it
 * @param[in] x1,x2,x3 Values
 * @param[in] t1,t2,t3 Timestamps of the values, in increasing order
 * @param[in] constructed True when the middle value is constructed, false
 * when the three values are input values
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double3_collinear(const double3 *x1, const double3 *x2, const double3 *x3,
  TimestampTz t1, TimestampTz t2, TimestampTz t3, bool constructed)
{
  return float_collinear(x1->a, x2->a, x3->a, t1, t2, t3, constructed) &&
    float_collinear(x1->b, x2->b, x3->b, t1, t2, t3, constructed) &&
    float_collinear(x1->c, x2->c, x3->c, t1, t2, t3, constructed);
}

/**
 * @brief Return true if the three double4 values are collinear
 * @details Every component is collinear, as #float_collinear decides it
 * @param[in] x1,x2,x3 Values
 * @param[in] t1,t2,t3 Timestamps of the values, in increasing order
 * @param[in] constructed True when the middle value is constructed, false
 * when the three values are input values
 * @note Function used for normalizing temporal values by removing redundant
 * instants
 */
bool
double4_collinear(const double4 *x1, const double4 *x2, const double4 *x3,
  TimestampTz t1, TimestampTz t2, TimestampTz t3, bool constructed)
{
  return float_collinear(x1->a, x2->a, x3->a, t1, t2, t3, constructed) &&
    float_collinear(x1->b, x2->b, x3->b, t1, t2, t3, constructed) &&
    float_collinear(x1->c, x2->c, x3->c, t1, t2, t3, constructed) &&
    float_collinear(x1->d, x2->d, x3->d, t1, t2, t3, constructed);
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
 * @note Function used for determining the value of a segment at a timestamp.
 * The count is the same along a segment, so where one end holds a sum and the
 * other an average (#double2_add), both are interpolated as averages.
 */
double2 *
double2segm_interpolate(const double2 *start, const double2 *end,
  long double ratio)
{
  assert(ratio >= 0.0 && ratio <= 1.0);
  double2 *result = palloc(sizeof(double2));
  if ((start->b > 0) == (end->b > 0))
  {
    result->a = floatsegm_interpolate(start->a, end->a, ratio);
    result->b = floatsegm_interpolate(start->b, end->b, ratio);
    return result;
  }
  result->a = floatsegm_interpolate(double2_avg(start), double2_avg(end),
    ratio);
  result->b = - fabs(start->b);
  return result;
}

/**
 * @brief Return a double3 interpolated from a double3 segment with respect to
 * a fraction of its total length
 * @param[in] start,end Values defining the segment
 * @param[in] ratio Value between 0 and 1 representing the fraction of the
 * total length of the segment where the value must be located
 * @note Function used for determining the value of a segment at a timestamp,
 * as #double2segm_interpolate
 */
double3 *
double3segm_interpolate(const double3 *start, const double3 *end,
  long double ratio)
{
  assert(ratio >= 0.0 && ratio <= 1.0);
  double3 *result = palloc(sizeof(double3));
  if ((start->c > 0) == (end->c > 0))
  {
    result->a = floatsegm_interpolate(start->a, end->a, ratio);
    result->b = floatsegm_interpolate(start->b, end->b, ratio);
    result->c = floatsegm_interpolate(start->c, end->c, ratio);
    return result;
  }
  double s1 = start->c > 0 ? start->c : 1.0, s2 = end->c > 0 ? end->c : 1.0;
  result->a = floatsegm_interpolate(start->a / s1, end->a / s2, ratio);
  result->b = floatsegm_interpolate(start->b / s1, end->b / s2, ratio);
  result->c = - fabs(start->c);
  return result;
}

/**
 * @brief Return a double4 interpolated from a double4 segment with respect to
 * a fraction of its total length
 * @param[in] start,end Values defining the segment
 * @param[in] ratio Value between 0 and 1 representing the fraction of the
 * total length of the segment where the value must be located
 * @note Function used for determining the value of a segment at a timestamp,
 * as #double2segm_interpolate
 */
double4 *
double4segm_interpolate(const double4 *start, const double4 *end,
  long double ratio)
{
  assert(ratio >= 0.0 && ratio <= 1.0);
  double4 *result = palloc(sizeof(double4));
  if ((start->d > 0) == (end->d > 0))
  {
    result->a = floatsegm_interpolate(start->a, end->a, ratio);
    result->b = floatsegm_interpolate(start->b, end->b, ratio);
    result->c = floatsegm_interpolate(start->c, end->c, ratio);
    result->d = floatsegm_interpolate(start->d, end->d, ratio);
    return result;
  }
  double s1 = start->d > 0 ? start->d : 1.0, s2 = end->d > 0 ? end->d : 1.0;
  result->a = floatsegm_interpolate(start->a / s1, end->a / s2, ratio);
  result->b = floatsegm_interpolate(start->b / s1, end->b / s2, ratio);
  result->c = floatsegm_interpolate(start->c / s1, end->c / s2, ratio);
  result->d = - fabs(start->d);
  return result;
}

/*****************************************************************************/

