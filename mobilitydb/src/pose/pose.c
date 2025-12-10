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
 * @brief Basic functions for pose objects
 */

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include <funcapi.h>
#include <access/heaptoast.h>
#include <lib/stringinfo.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "pose/pose.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_in);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Input function for pose values
 * @details Example of input:
 *    Pose(Point(1 1), 0.5)
 *    Pose(Point(1 1 1), 0.5, 0.5, 0.5, 0.5)
 *    SRID=5676;Pose(Point(1 1 1), 0.5, 0.5, 0.5, 0.5)
 * @sqlfn pose_in()
 */
Datum
Pose_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(pose_parse(&str, true));
}

PGDLLEXPORT Datum Pose_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_out);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Output function for pose values
 * @sqlfn pose_out()
 */
Datum
Pose_out(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_CSTRING(pose_out(pose, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Pose_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_recv);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return a pose from its Well-Known Binary (WKB) representation
 * @sqlfn pose_recv()
 */
Datum
Pose_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Pose *result = pose_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Pose_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_send);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose
 * @sqlfn pose_send()
 */
Datum
Pose_send(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(pose);
  /* A pose always outputs the SRID */
  uint8_t *wkb = pose_as_wkb(pose, WKB_EXTENDED, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Input/output in (E)WKT, (E)WKB, and HexWKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Pose_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_from_ewkt);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return a pose from its Extended Well-Known Text (EWKT) representation
 * @note This just does the same thing as the SQL function pose_in, except it
 * has to handle a 'text' input. First, unwrap the text into a cstring, then
 * do as pose_in
 * @sqlfn poseFromEWKT(), poseFromEWKT()
 */
Datum
Pose_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  char *wkt = text_to_cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Pose *result = pose_parse(&wkt_ptr, true);
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Pose_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_from_wkb);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return a pose from its Well-Known Binary (WKB) representation
 * @sqlfn poseFromBinary()
 */
Datum
Pose_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Pose *result = pose_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Pose_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_from_hexwkb);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return a pose from its ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation
 * @sqlfn poseFromHexWKB()
 */
Datum
Pose_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  Pose *result = pose_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POSE_P(result);
}

/*****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a pose
 * @sqlfn asText(), asEWKT()
 */
static Datum
Pose_as_text_common(FunctionCallInfo fcinfo, bool extended)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? pose_as_ewkt(pose, dbl_dig_for_wkt) : 
    pose_as_text(pose, dbl_dig_for_wkt);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Pose_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_text);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Return the Well-Known Text (WKT) representation of a pose
 * @sqlfn asText()
 */
Datum
Pose_as_text(PG_FUNCTION_ARGS)
{
  return Pose_as_text_common(fcinfo, false);
}

PGDLLEXPORT Datum Pose_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_ewkt);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a pose
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Pose_as_ewkt(PG_FUNCTION_ARGS)
{
  return Pose_as_text_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Pose_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_wkb);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose
 * @sqlfn asBinary()
 */
Datum
Pose_as_wkb(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(pose), T_POSE,
    false));
}

PGDLLEXPORT Datum Pose_as_ewkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_ewkb);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose
 * @sqlfn asBinary()
 */
Datum
Pose_as_ewkb(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(pose), T_POSE, true));
}

PGDLLEXPORT Datum Pose_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_hexwkb);
/**
 * @ingroup mobilitydb_pose_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a pose
 * @sqlfn asHexWKB()
 */
Datum
Pose_as_hexwkb(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, PointerGetDatum(pose), T_POSE));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_constructor);
/**
 * @ingroup mobilitydb_pose_base_constructor
 * @brief Construct a pose from a 2D/3D point and an orientation
 * @sqlfn pose()
 */
Datum
Pose_constructor(PG_FUNCTION_ARGS)
{
  double x = PG_GETARG_FLOAT8(0);
  double y = PG_GETARG_FLOAT8(1);
  double z = PG_GETARG_FLOAT8(2);
  double theta = z;
  int32_t srid;

  assert(PG_NARGS() == 4 || PG_NARGS() == 8);
  Pose *result;
  if (PG_NARGS() == 4)
  {
    srid = PG_GETARG_INT32(3);
    result = pose_make_2d(x, y, theta, srid);
  }
  else /* PG_NARGS() == 8 */
  {
    double W = PG_GETARG_FLOAT8(3);
    double X = PG_GETARG_FLOAT8(4);
    double Y = PG_GETARG_FLOAT8(5);
    double Z = PG_GETARG_FLOAT8(6);
    srid = PG_GETARG_INT32(7);
    result = pose_make_3d(x, y, z, W, X, Y, Z, srid);
  }
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Pose_constructor_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_constructor_point);
/**
 * @ingroup mobilitydb_pose_base_constructor
 * @brief Construct a pose from a 2D/3D point and an orientation
 * @sqlfn pose()
 */
Datum
Pose_constructor_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  assert(PG_NARGS() == 2 || PG_NARGS() == 5);
  Pose *result;
  if (PG_NARGS() == 2)
  {
    double theta = PG_GETARG_FLOAT8(1);
    result = pose_make_point2d(gs, theta);
  }
  else /* PG_NARGS() == 5 */
  {
    double W = PG_GETARG_FLOAT8(1);
    double X = PG_GETARG_FLOAT8(2);
    double Y = PG_GETARG_FLOAT8(3);
    double Z = PG_GETARG_FLOAT8(4);
    result = pose_make_point3d(gs, W, X, Y, Z);
  }
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_point);
/**
 * @ingroup mobilitydb_pose_base_conversion
 * @brief Convert a pose into a geometry point
 * @sqlfn geometry()
 */
Datum
Pose_to_point(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  GSERIALIZED *result = pose_to_point(pose);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Bounding box functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_stbox);
/**
 * @ingroup mobilitydb_pose_base_box
 * @brief Construct a spatiotemporal box from a pose
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Pose_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  STBox *result = palloc0(sizeof(STBox));
  pose_set_stbox(pose, result);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Pose_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_pose_base_box
 * @brief Construct a spatiotemporal box from a pose and a timestamptz
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Pose_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(pose_timestamptz_to_stbox(pose, t));
}

PGDLLEXPORT Datum Pose_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_pose_base_box
 * @brief Construct a spatiotemporal box from a pose and a timestamptz span
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Pose_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(pose_tstzspan_to_stbox(pose, s));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_point);
/**
 * @ingroup mobilitydb_pose_base_accessor
 * @brief Return the point of a pose
 * @sqlfn point()
 */
Datum
Pose_point(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Datum d = PointerGetDatum(pose_to_point(pose));
  PG_RETURN_DATUM(datum_copy(d, T_GEOMETRY));
}

PGDLLEXPORT Datum Pose_rotation(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_rotation);
/**
 * @ingroup mobilitydb_pose_base_accessor
 * @brief Return the rotation of a 2D pose
 * @sqlfn rotation()
 */
Datum
Pose_rotation(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_FLOAT8(pose_rotation(pose));
}

PGDLLEXPORT Datum Pose_orientation(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_orientation);
/**
 * @ingroup mobilitydb_pose_base_accessor
 * @brief Return the orientation of a 3D pose
 * @sqlfn orientation()
 */
Datum
Pose_orientation(PG_FUNCTION_ARGS)
{
  /* Define the return type properties */
  TupleDesc tupdesc;
  HeapTuple tuple;
  Datum values[4];
  bool nulls[4] = { false, false, false, false }; // Assume no nulls
  /* Define the structure of the returned tuple */
  tupdesc = CreateTemplateTupleDesc(4);
  TupleDescInitEntry(tupdesc, (AttrNumber) 1, "W", FLOAT8OID, -1, 0);
  TupleDescInitEntry(tupdesc, (AttrNumber) 2, "X", FLOAT8OID, -1, 0);
  TupleDescInitEntry(tupdesc, (AttrNumber) 3, "Y", FLOAT8OID, -1, 0);
  TupleDescInitEntry(tupdesc, (AttrNumber) 4, "Z", FLOAT8OID, -1, 0);
  BlessTupleDesc(tupdesc);
  /* Get input pose */
  Pose *pose = PG_GETARG_POSE_P(0);
  /* Get the array of doubles representing the orientation */
  double *quaternion = pose_orientation(pose);
  /* Create values for the tuple */
  values[0] = Float8GetDatum(quaternion[0]);
  values[1] = Float8GetDatum(quaternion[1]);
  values[2] = Float8GetDatum(quaternion[2]);
  values[3] = Float8GetDatum(quaternion[3]);
  /* Create a new tuple */
  tuple = heap_form_tuple(tupdesc, values, nulls);
  /* Return the tuple */
  PG_RETURN_DATUM(HeapTupleGetDatum(tuple));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_round);
/**
 * @ingroup mobilitydb_pose_base_transf
 * @brief Return a pose with the precision of the values set to a number of
 * decimal places
 * @sqlfn round()
 */
Datum
Pose_round(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Pose *result = pose_round(pose, size);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Posearr_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posearr_round);
/**
 * @ingroup mobilitydb_pose_base_transf
 * @brief Return an array of poses with the precision of the values set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Posearr_round(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int maxdd = PG_GETARG_INT32(1);

  Pose **posearr = posearr_extract(array, &count);
  Pose **resarr = posearr_round((const Pose **) posearr, count, maxdd);
  ArrayType *result = posearr_to_array((const Pose **) resarr, count);
  pfree(posearr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_srid);
/**
 * @ingroup mobilitydb_pose_base_srid
 * @brief Return the SRID of a pose
 * @sqlfn SRID()
 */
Datum
Pose_srid(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int result = pose_srid(pose);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Pose_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_set_srid);
/**
 * @ingroup mobilitydb_pose_base_srid
 * @brief Return a pose with the coordinates of the point set to an SRID
 * @sqlfn setSRID()
 */
Datum
Pose_set_srid(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Pose *result = pose_copy(pose);
  pose_set_srid(result, srid);
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Pose_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_transform);
/**
 * @ingroup mobilitydb_pose_base_srid
 * @brief Return a pose transformed to an SRID
 * @sqlfn transform()
 */
Datum
Pose_transform(PG_FUNCTION_ARGS)
{
  Pose *cb = PG_GETARG_POSE_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Pose *result = pose_transform(cb, srid);
  PG_FREE_IF_COPY(cb, 0);
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Pose_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_transform_pipeline);
/**
 * @ingroup mobilitydb_pose_base_srid
 * @brief Return a pose transformed to an SRID using a pipeline
 * @sqlfn transformPipeline()
 */
Datum
Pose_transform_pipeline(PG_FUNCTION_ARGS)
{
  Pose *cb = PG_GETARG_POSE_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text_to_cstring(pipelinetxt);
  Pose *result = pose_transform_pipeline(cb, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(cb, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_POSE_P(result);
}

/*****************************************************************************
 * Approximate equality for poses
 *****************************************************************************/

PGDLLEXPORT Datum Pose_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_same);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if two poses are approximately equal with respect to an
 * epsilon value
 * @sqlfn same()
 */
Datum
Pose_same(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_same(pose1, pose2));
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_eq);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is equal to the second one
 * @sqlfn pose_eq()
 * @sqlop @p =
 */
Datum
Pose_eq(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_eq(pose1, pose2));
}

PGDLLEXPORT Datum Pose_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_ne);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is not equal to the second one
 * @sqlfn pose_ne()
 * @sqlop @p <>
 */
Datum
Pose_ne(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_ne(pose1, pose2));
}

PGDLLEXPORT Datum Pose_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_cmp);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pose is less than,
 * equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn pose_cmp()
 */
Datum
Pose_cmp(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_INT32(pose_cmp(pose1, pose2));
}

PGDLLEXPORT Datum Pose_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_lt);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is less than the second one
 * @sqlfn pose_lt()
 * @sqlop @p <
 */
Datum
Pose_lt(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_lt(pose1, pose2));
}

PGDLLEXPORT Datum Pose_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_le);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is less than or equal to the second one
 * @sqlfn pose_le()
 * @sqlop @p <=
 */
Datum
Pose_le(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_le(pose1, pose2));
}

PGDLLEXPORT Datum Pose_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_ge);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is greater than or equal to the second
 * one
 * @sqlfn pose_ge()
 * @sqlop @p >=
 */
Datum
Pose_ge(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_ge(pose1, pose2));
}

PGDLLEXPORT Datum Pose_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_gt);
/**
 * @ingroup mobilitydb_pose_base_comp
 * @brief Return true if the first pose is greater than the second one
 * @sqlfn pose_gt()
 * @sqlop @p >
 */
Datum
Pose_gt(PG_FUNCTION_ARGS)
{
  Pose *pose1 = PG_GETARG_POSE_P(0);
  Pose *pose2 = PG_GETARG_POSE_P(1);
  PG_RETURN_BOOL(pose_gt(pose1, pose2));
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

PGDLLEXPORT Datum Pose_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_hash);
/**
 * @ingroup mobilitydb_pose_base_accessor
 * @brief Return the 32-bit hash value of a pose
 * @sqlfn hash()
 */
Datum
Pose_hash(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  uint32 result = pose_hash(pose);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Pose_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_hash_extended);
/**
 * @ingroup mobilitydb_pose_base_accessor
 * @brief Return the 64-bit hash value of a pose using a seed
 * @sqlfn hash_extended()
 */
Datum
Pose_hash_extended(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = pose_hash_extended(pose, seed);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
