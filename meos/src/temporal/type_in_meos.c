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
 * @brief Input of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB
 * representation
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/tbox.h"

/*****************************************************************************
 * Input in MF-JSON representation
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tboolinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TBOOL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tintinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TINT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
tfloatinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TFLOAT);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TInstant *
ttextinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TTEXT);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tboolseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TBOOL, STEP);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tintseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TINT, STEP);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
tfloatseq_from_mfjson(json_object *mfjson, interpType interp)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TFLOAT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequence *
ttextseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TTEXT, STEP);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tboolseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TBOOL, STEP);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer sequence set from its MF-JSON
 * representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tintseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TINT, STEP);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float sequence set from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
tfloatseqset_from_mfjson(json_object *mfjson, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TFLOAT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text sequence set from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @csqlfn #Temporal_from_mfjson()
 */
inline TSequenceSet *
ttextseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TTEXT, STEP);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal boolean from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tbool_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TBOOL);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal integer from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
inline Temporal *
tint_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal float from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tfloat_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TFLOAT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal text from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
ttext_from_mfjson(const char *mfjson)
{
  return temporal_from_mfjson(mfjson, T_TTEXT);
}

/*****************************************************************************/
