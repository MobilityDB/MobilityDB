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
 * @brief General functions for temporal instants
 */

#include "temporal/tinstant.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#include <common/hashfn.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/meos_catalog.h"
#include "temporal/tsequence.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tboolinst_in(const char *str)
{
  return tinstant_in(str, T_TBOOL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tintinst_in(const char *str)
{
  return tinstant_in(str, T_TINT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tfloatinst_in(const char *str)
{
  return tinstant_in(str, T_TFLOAT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
ttextinst_in(const char *str)
{
  return tinstant_in(str, T_TTEXT);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean instant from a boolean and a timestamptz
 * @param[in] b Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tboolinst_make(bool b, TimestampTz t)
{
  return tinstant_make(BoolGetDatum(b), T_TBOOL, t);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer instant from an integer and a timestamptz
 * @param[in] i Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tintinst_make(int i, TimestampTz t)
{
  return tinstant_make(Int32GetDatum(i), T_TINT, t);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float instant from a float and a timestamptz
 * @param[in] d Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tfloatinst_make(double d, TimestampTz t)
{
  return tinstant_make(Float8GetDatum(d), T_TFLOAT, t);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text instant from a text and a timestamptz
 * @param[in] txt Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
ttextinst_make(const text *txt, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(txt, NULL);
  return tinstant_make(PointerGetDatum(txt), T_TTEXT, t);
}

/*****************************************************************************/
