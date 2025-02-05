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
 * @brief General functions for temporal sequences
 */

#include "general/tsequence.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
#include <utils/timestamp.h>
#include <common/hashfn.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/pg_types.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/tinstant.h"
#include "general/temporal_boxops.h"
#include "general/type_util.h"
#include "general/type_parser.h"
#include "geo/tgeo_parser.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include "cbuffer/tcbuffer.h"
  #include "cbuffer/tcbuffer_parser.h"
#endif
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif
#if POSE
  #include "pose/pose.h"
  #include "pose/tpose_spatialfuncs.h"
#endif

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal boolean sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tboolseq_in(const char *str, interpType interp)
{
  return tsequence_in(str, T_TBOOL, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal integer sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tintseq_in(const char *str, interpType interp)
{
  return tsequence_in(str, T_TINT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal float sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tfloatseq_in(const char *str, interpType interp)
{
  return tsequence_in(str, T_TFLOAT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal text sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
ttextseq_in(const char *str, interpType interp)
{
  return tsequence_in(str, T_TTEXT, interp);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry point sequence from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeompointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOMPOINT);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography point sequence from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeogpointseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tpoint_parse(&str, T_TGEOGPOINT);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geometry sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeometryseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tgeo_parse(&str, T_TGEOMETRY);
  if (! temp)
    return NULL;
  assert(temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal geography sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tgeographyseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tgeo_parse(&str, T_TGEOGRAPHY);
  if (! temp)
    return NULL;
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

#if CBUFFER
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal circular buffer sequence from its Well-Known Text 
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tcbufferseq_in(const char *str, interpType interp __attribute__((unused)))
{
  assert(str);
  /* Call the superclass function to read the SRID at the beginning (if any) */
  Temporal *temp = tcbuffer_parse(&str);
  if (! temp)
    return NULL;
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}
#endif /* CBUFFER */

/*****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean discrete sequence from a boolean and a
 * timestamptz set
 * @param[in] b Value
 * @param[in] s Set
 */
TSequence *
tboolseq_from_base_tstzset(bool b, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return tsequence_from_base_tstzset(BoolGetDatum(b), T_TBOOL, s);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer discrete sequence from an integer and a
 * timestamptz set
 * @param[in] i Value
 * @param[in] s Set
 */
TSequence *
tintseq_from_base_tstzset(int i, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return tsequence_from_base_tstzset(Int32GetDatum(i), T_TINT, s);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float discrete sequence from a float and a
 * timestamptz set
 * @param[in] d Value
 * @param[in] s Set
 */
TSequence *
tfloatseq_from_base_tstzset(double d, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return tsequence_from_base_tstzset(Float8GetDatum(d), T_TFLOAT, s);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text discrete sequence from a text and a
 * timestamptz set
 * @param[in] txt Value
 * @param[in] s Set
 */
TSequence *
ttextseq_from_base_tstzset(const text *txt, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt) || ! ensure_not_null((void *) s) || 
      ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return tsequence_from_base_tstzset(PointerGetDatum(txt), T_TTEXT, s);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal geometry point discrete sequence from a point
 * and a timestamptz set
 * @param[in] gs Value
 * @param[in] s Set
 */
TSequence *
tpointseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) s) ||
      ! ensure_not_empty(gs) || ! ensure_point_type(gs) || 
      ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequence_from_base_tstzset(PointerGetDatum(gs), temptype, s);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal geo discrete sequence from a geometry/geography
 * and a timestamptz set
 * @param[in] gs Value
 * @param[in] s Set
 */
TSequence *
tgeoseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) s) ||
      ! ensure_not_empty(gs) || ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tsequence_from_base_tstzset(PointerGetDatum(gs), temptype, s);
}

#if CBUFFER
/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal circular buffer discrete sequence from a circular
 * buffer and a timestamptz set
 * @param[in] gs Value
 * @param[in] s Set
 */
TSequence *
tcbufferseq_from_base_tstzset(const Cbuffer *cbuf, const Set *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) s) || 
      ! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;
  return tsequence_from_base_tstzset(PointerGetDatum(cbuf), T_TCBUFFER, s);
}
#endif /* CBUFFER */

/*****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean sequence from a boolean and a timestamptz
 * span
 * @param[in] b Value
 * @param[in] s Span
 */
TSequence *
tboolseq_from_base_tstzspan(bool b, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tsequence_from_base_tstzspan(BoolGetDatum(b), T_TBOOL, s, STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer sequence from an integer and a timestamptz
 * span
 * @param[in] i Value
 * @param[in] s Span
 */
TSequence *
tintseq_from_base_tstzspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if ( ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tsequence_from_base_tstzspan(Int32GetDatum(i), T_TINT, s, STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float sequence from a float and a timestamptz
 * span
 * @param[in] d Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tfloatseq_from_base_tstzspan(double d, const Span *s, interpType interp)
{
  /* Ensure validity of the arguments */
  if ( ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tsequence_from_base_tstzspan(Float8GetDatum(d), T_TFLOAT, s, interp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text sequence from a text and a timestamptz span
 * @param[in] txt Value
 * @param[in] s Span
 */
TSequence *
ttextseq_from_base_tstzspan(const text *txt, const Span *s)
{
  /* Ensure validity of the arguments */
  if ( ! ensure_not_null((void *) txt) || ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tsequence_from_base_tstzspan(PointerGetDatum(txt), T_TTEXT, s, STEP);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal point sequence from a point and a timestamptz span
 * @param[in] gs Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tpointseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s,
  interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) s) ||
      gserialized_is_empty(gs) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  return tsequence_from_base_tstzspan(PointerGetDatum(gs), temptype, s,
    interp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal geo sequence from a geometry/geography and a
 * timestamptz span
 * @param[in] gs Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tgeoseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s,
  interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) gs) || ! ensure_not_null((void *) s) ||
      gserialized_is_empty(gs) || ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  meosType temptype = FLAGS_GET_GEODETIC(gs->gflags) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tsequence_from_base_tstzspan(PointerGetDatum(gs), temptype, s,
    interp);
}

#if CBUFFER
/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal circular buffer sequence from a circular buffer and
 * a timestamptz span
 * @param[in] gs Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tcbufferseq_from_base_tstzspan(const Cbuffer *cbuf, const Span *s,
  interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) cbuf) || ! ensure_not_null((void *) s) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;
  return tsequence_from_base_tstzspan(PointerGetDatum(cbuf), T_TCBUFFER, s,
    interp);
}
#endif /* CBUFFER */

/*****************************************************************************/
