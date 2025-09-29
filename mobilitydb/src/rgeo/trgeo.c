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
 * @brief General functions for temporal rigid geometries
 */


#include "rgeo/trgeo.h"

/* C */
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/array.h"
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_parser.h"
#include "rgeo/trgeo_all.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_in);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Generic input function for temporal rigid geometries
 * @details Examples of input for the various temporal types:
 * - Instant
 * @code
 * Polygon((0 0, 1 0, 0 1, 0 0)); Pose(0, 0, 0) @ 2012-01-01 08:00:00
 * @endcode
 * - Instant set
 * @code
 * Polygon((0 0, 1 0, 0 1, 0 0));{ Pose(0, 0, 0) @ 2012-01-01 08:00:00 ,
 * Pose(1, 1, 0) @ 2012-01-01 08:10:00 }
 * @endcode
 * - Sequence
 * @code
 * Polygon((0 0, 1 0, 0 1, 0 0));[ Pose(0, 0, 0) @ 2012-01-01 08:00:00 ,
 * Pose(1, 1, 0) @ 2012-01-01 08:10:00 )
 * @endcode
 * - Sequence set
 * @code
 * Polygon((0 0, 1 0, 0 1, 0 0));{ [ Pose(0, 0, 0) @ 2012-01-01 08:00:00 ,
 * Pose(1, 1, 0) @ 2012-01-01 08:10:00 ) , [ Pose(1, 1, 0) @ 2012-01-01 08:20:00 ,
 * Pose(0, 0, 0) @ 2012-01-01 08:30:00 ] }
 * @endcode
 */
Datum
Trgeometry_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = trgeo_parse(&input, oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_out);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Generic output function for temporal rigid geometries
 */
Datum
Trgeometry_out(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *result = trgeo_out(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Trgeometry_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_recv);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Return a temporal rigid geometry from its Well-Known Binary (WKB)
 * representation
 * @sqlfn trgeometry_recv()
 */
Datum
Trgeometry_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Temporal *result = temporal_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_send);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal rigid geometry
 * @sqlfn trgeometry_send()
 */
Datum
Trgeometry_send(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(temp);
  /* A temporal geometry always outputs the SRID */
  uint8_t *wkb = temporal_as_wkb(temp, WKB_EXTENDED, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Trgeometry_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_typmod_in);
/**
 * @brief Input typmod information for temporal rigid geometries
 */
Datum
Trgeometry_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, true, false);
  PG_RETURN_INT32(typmod);
}

/*****************************************************************************
 * Input in EWKT representation
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_from_ewkt);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Input a temporal rigid geometry from its Extended Well-Known Text
 * (EWKT) representation
 * @note This just does the same thing as the _in function, except it has to handle
 * a 'text' input. First, unwrap the text into a cstring, then do as tgeometry_in
 * @sqlfn trgeometryFromEWKT()
 */
PGDLLEXPORT Datum
Trgeometry_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  char *wkt = text2cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Temporal *result = trgeo_parse(&wkt_ptr, oid_type(temptypid));
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Output in (E)WKT representation
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal rigid geometry
 */
static Datum
Trgeometry_as_text_common(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = trgeo_wkt_out(temp, dbl_dig_for_wkt, extended);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Trgeometry_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_as_text);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal rigid
 * geometry
 * @sqlfn asText()
 */
Datum
Trgeometry_as_text(PG_FUNCTION_ARGS)
{
  return Trgeometry_as_text_common(fcinfo, false);
}

PGDLLEXPORT Datum Trgeometry_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_as_ewkt);
/**
 * @ingroup mobilitydb_rgeo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal rigid geometry
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Trgeometry_as_ewkt(PG_FUNCTION_ARGS)
{
  return Trgeometry_as_text_common(fcinfo, true);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_inst_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_inst_constructor);
/**
 * @ingroup mobilitydb_rgeo_constructor
 * @brief Construct a temporal rigid geometry instant from a geometry, a pose,
 * and a timestamptz
 * @sqlfn trgeometryInst()
 */
Datum
Trgeometry_inst_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  ensure_not_empty(gs);
  ensure_has_not_M_geo(gs);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(2);
  TInstant *result = trgeoinst_make(gs, pose, t);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_seq_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_seq_constructor);
/**
 * @ingroup mobilitydb_rgeo_constructor
 * @brief Construct a temporal rigid geometry sequence from an array of
 * temporal instants
 * @sqlfn trgeometrySeq()
 */
Datum
Trgeometry_seq_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  bool lower_inc = true, upper_inc = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    lower_inc = PG_GETARG_BOOL(2);
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
    upper_inc = PG_GETARG_BOOL(3);
  ensure_not_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temparr_extract(array, &count);
  Temporal *result = (Temporal *) trgeoseq_make(trgeoinst_geom_p(instants[0]),
    (const TInstant **) instants, count, lower_inc, upper_inc, interp,
    NORMALIZE);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_seqset_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_seqset_constructor);
/**
 * @ingroup mobilitydb_rgeo_constructor
 * @brief Construct a temporal rigid geometry sequence set from an array of
 * temporal sequences
 * @sqlfn trgeometrySeqSet()
 */
Datum
Trgeometry_seqset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  int count;
  TSequence **sequences = (TSequence **) temparr_extract(array, &count);
  Temporal *result = (Temporal *) trgeoseqset_make(
    trgeoseq_geom_p(sequences[0]), (const TSequence **) sequences, count,
    NORMALIZE);
  pfree(sequences);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_seqset_constructor_gaps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_seqset_constructor_gaps);
/**
 * @ingroup mobilitydb_rgeo_constructor
 * @brief Construct a temporal rigid geometry sequence set from an array of
 * temporal instants accounting for potential gaps
 * @note The SQL function is not strict
 * @sqlfn trgeoSeqsetGaps()
 */
Datum
Trgeometry_seqset_constructor_gaps(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  double maxdist = -1.0;
  Interval *maxt = NULL;
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    maxt = PG_GETARG_INTERVAL_P(1);
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    maxdist = PG_GETARG_FLOAT8(2);
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    text *interp_txt = PG_GETARG_TEXT_P(3);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  /* Store fcinfo into a global variable */
  /* Needed for the distance function for temporal geographic points */
  store_fcinfo(fcinfo);
  /* Extract the array of instants */
  int count;
  TInstant **instants = (TInstant **) temparr_extract(array, &count);
  TSequenceSet *result = trgeoseqset_make_gaps(trgeoinst_geom_p(instants[0]),
    (const TInstant **) instants, count, interp, maxt, maxdist);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Trgeometry_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_constructor);
/**
 * @ingroup mobilitydb_rgeo_constructor
 * @brief Construct a temporal rigid geometry from a reference geometry and a
 * temporal pose
 * @sqlfn trgeometry()
 */
Datum
Trgeometry_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *tpose = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = geo_tpose_to_trgeo(gs, tpose);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_to_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_tpose);
/**
 * @ingroup mobilitydb_rgeo_conversion
 * @brief Convert a temporal rigid geometry into a temporal pose
 * @sqlfn tpose()
 */
Datum
Trgeometry_to_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeo_to_tpose(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_to_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_tpoint);
/**
 * @ingroup mobilitydb_rgeo_conversion
 * @brief Convert a temporal rigid geometry into a temporal point 
 * @sqlfn tgeompoint()
 */
Datum
Trgeometry_to_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeo_to_tpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_geom);
/**
 * @ingroup mobilitydb_rgeo_conversion
 * @brief Return the reference geometry of a temporal rigid geometry
 * @sqlfn geometry()
 */
Datum
Trgeometry_to_geom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeo_geom(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_start_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_start_value);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the start value of a temporal rigid geometry
 * @sqlfn startValue()
 */
Datum
Trgeometry_start_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeo_start_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_end_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_end_value);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the end value of a temporal rigid geometry
 * @sqlfn endValue()
 */
Datum
Trgeometry_end_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeo_end_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeometry_value_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_value_n);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the n-th value of a temporal rigid geometry
 * @sqlfn valueN()
 */
Datum
Trgeometry_value_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  GSERIALIZED *result;
  bool found = trgeo_value_n(temp, n, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Trgeometry_value_at_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_value_at_timestamptz);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the value of a temporal rigid geometry at a timestamptz
 * @sqlfn valueAtTimestamp()
 */
Datum
Trgeometry_value_at_timestamptz(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Datum result;
  bool found = trgeo_value_at_timestamptz(temp, t, true, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Transformation Functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_to_tinstant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_tinstant);
/**
 * @ingroup mobilitydb_rgeo_transf
 * @brief Return a temporal rigid geometry transformed into a temporal instant
 * @sqlfn trgeometryInst()
 */
Datum
Trgeometry_to_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = trgeo_to_tinstant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Trgeometry_to_tsequence(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_tsequence);
/**
 * @ingroup mobilitydb_rgeo_transf
 * @brief Return a temporal rigid geometry transformed into a temporal sequence
 * @note The SQL function is not strict
 * @sqlfn trgeometrySeq()
 */
Datum
Trgeometry_to_tsequence(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *interp_str = NULL;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    interp_str = text2cstring(interp_txt);
  }
  TSequence *result = trgeo_to_tsequence(temp, interp_str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Trgeometry_to_tsequenceset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_to_tsequenceset);
/**
 * @ingroup mobilitydb_rgeo_transf
 * @brief Return a temporal rigid geometry transformed into a temporal sequence
 * set
 * @note The SQL function is not strict
 * @sqlfn trgeoSeqSet()
 */
Datum
Trgeometry_to_tsequenceset(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *interp_str = NULL;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    interp_str = text2cstring(interp_txt);
  }
  TSequenceSet *result = trgeo_to_tsequenceset(temp, interp_str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCESET_P(result);
}

/*****************************************************************************
 * Modification Functions
 *****************************************************************************/


/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/


/*****************************************************************************/


