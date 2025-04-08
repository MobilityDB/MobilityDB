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
 * @brief General functions for temporal sequence sets
 */

#include "general/tsequenceset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/tsequence.h"
#include "general/temporal_boxops.h"
#include "general/type_parser.h"
#include "general/type_util.h"

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean sequence set from a boolean and a
 * timestamptz span set
 * @param[in] b Value
 * @param[in] ss Span set
 */
TSequenceSet *
tboolseqset_from_base_tstzspanset(bool b, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(BoolGetDatum(b), T_TBOOL, ss,
    STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer sequence set from an integer and a
 * timestamptz span set
 * @param[in] i Value
 * @param[in] ss Span set
 */
TSequenceSet *
tintseqset_from_base_tstzspanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(Int32GetDatum(i), T_TINT, ss,
    STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float sequence set from a float and a timestamptz
 * span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tfloatseqset_from_base_tstzspanset(double d, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(Float8GetDatum(d), T_TFLOAT, ss,
    interp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text sequence set from a text and a timestamptz
 * span set
 * @param[in] txt Value
 * @param[in] ss Span set
 */
TSequenceSet *
ttextseqset_from_base_tstzspanset(const text *txt, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(txt, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(txt), T_TTEXT, ss,
    STEP);
}


/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tboolseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TBOOL, true);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tintseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TINT, true);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
tfloatseqset_in(const char *str)
{
  assert(str);
  /* Call the superclass function to read the interpolation at the beginning (if any) */
  Temporal *temp = temporal_parse(&str, T_TFLOAT);
  assert(temp->subtype == TSEQUENCE);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text sequence set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TSequenceSet *
ttextseqset_in(const char *str)
{
  assert(str);
  return tsequenceset_parse(&str, T_TTEXT, true);
}

/*****************************************************************************/
