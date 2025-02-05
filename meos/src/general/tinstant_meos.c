/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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

#include "general/tinstant.h"

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
#include "general/meos_catalog.h"
#include "general/pg_types.h"
#include "general/tsequence.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo_parser.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include "cbuffer/tcbuffer_parser.h"
#endif

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

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry point instant from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tgeompointinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal instant geography point from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tgeogpointinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tgeometryinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tgeo_parse(&str, T_TGEOMETRY);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tgeographyinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tgeo_parse(&str, T_TGEOGRAPHY);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

#if CBUFFER
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal circular buffer instant from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tcbufferinst_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tcbuffer_parse(&str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}
#endif /* CBUFFER */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt))
    return NULL;
  return tinstant_make(PointerGetDatum(txt), T_TTEXT, t);
}

/*****************************************************************************/
