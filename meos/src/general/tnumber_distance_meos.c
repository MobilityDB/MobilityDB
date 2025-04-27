/***********************************************************************
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
 * @brief Distance functions for temporal numbers
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/span.h"
#include "general/tbox.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a temporal integer and an
 * integer
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @return On error return @p NULL
 * @csqlfn #Distance_tnumber_number()
 */
Temporal *
distance_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return distance_tnumber_number(temp, Int32GetDatum(i));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the temporal distance between a temporal float and a float
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @return On error return @p NULL
 * @csqlfn #Distance_tnumber_number()
 */
Temporal *
distance_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);
  return distance_tnumber_number(temp, Int32GetDatum(d));
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] i Value
 * @return On error return -1
 * @csqlfn #NAD_tnumber_number()
 */
int
nad_tint_int(const Temporal *temp, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, -1.0);
  return DatumGetInt32(nad_tnumber_number(temp, Int32GetDatum(i)));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal number
 * and a number
 * @param[in] temp Temporal value
 * @param[in] d Value
 * @return On error return -1
 * @csqlfn #NAD_tnumber_number()
 */
double
nad_tfloat_float(const Temporal *temp, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, -1.0);
  return DatumGetFloat8(nad_tnumber_number(temp, Float8GetDatum(d)));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal integer
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @return On error or if the time frames of the boxes do not overlap return -1
 * @csqlfn #NAD_tnumber_tbox()
 */
int
nad_tint_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, &box->span))
    return -1;
  return DatumGetInt32(nad_tnumber_tbox(temp, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between a temporal float
 * and a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Temporal box
 * @return On error or if the time frames of the boxes do not overlap return -1
 * @csqlfn #NAD_tnumber_tbox()
 */
double
nad_tfloat_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_numspan(temp, &box->span))
    return -1;
  return DatumGetFloat8(nad_tnumber_tbox(temp, box));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between the int temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @return On error return -1
 * @csqlfn #NAD_tbox_tbox()
 */
int
nad_tboxint_tboxint(const TBox *box1, const TBox *box2)
{
  if (! ensure_span_isof_type(&box1->span, T_INTSPAN) ||
      ! ensure_span_isof_type(&box2->span, T_INTSPAN))
    return -1;

  return DatumGetInt32(nad_tbox_tbox(box1, box2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between the float temporal boxes
 * @param[in] box1,box2 Temporal boxes
 * @return On error return -1
 * @csqlfn #NAD_tbox_tbox()
 */
double
nad_tboxfloat_tboxfloat(const TBox *box1, const TBox *box2)
{
  if (! ensure_span_isof_type(&box1->span, T_FLOATSPAN) ||
      ! ensure_span_isof_type(&box2->span, T_FLOATSPAN))
    return -1;

  return DatumGetFloat8(nad_tbox_tbox(box1, box2));
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between two temporal integers
 * @param[in] temp1,temp2 Temporal values
 * @return On error return -1
 * @csqlfn #NAD_tnumber_tnumber()
 */
int
nad_tint_tint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp1, -1); VALIDATE_TINT(temp2, -1);
  Datum result = nad_tnumber_tnumber(temp1, temp2);
  return Int32GetDatum(result);
}

/**
 * @ingroup meos_temporal_dist
 * @brief Return the nearest approach distance between two temporal floats
 * @param[in] temp1,temp2 Temporal values
 * @return On error return -1
 * @csqlfn #NAD_tnumber_tnumber()
 */
double
nad_tfloat_tfloat(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp1, -1.0); VALIDATE_TFLOAT(temp2, -1.0);
  Datum result = nad_tnumber_tnumber(temp1, temp2);
  return Float8GetDatum(result);
}

/*****************************************************************************/
