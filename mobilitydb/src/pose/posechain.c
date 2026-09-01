/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Basic functions for pose chain objects
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include <funcapi.h>
#include <lib/stringinfo.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "temporal/span.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "pose/pose.h"
#include "pose/posechain.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_in);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Input function for pose chain values
 * @details Example of input:
 *    PoseChain(Pose(Point(1 1), 0.5), Pose(Point(2 0), 0.1))
 *    SRID=5676;PoseChain(Pose(Point(1 1 1), 1, 0, 0, 0), Pose(Point(0 0 1), 1, 0, 0, 0))
 * @sqlfn posechain_in()
 */
Datum
Posechain_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(posechain_in(str));
}

PGDLLEXPORT Datum Posechain_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_out);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Output function for pose chain values
 * @sqlfn posechain_out()
 */
Datum
Posechain_out(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_CSTRING(posechain_out(pc, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Posechain_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_recv);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return a pose chain from its Well-Known Binary (WKB) representation
 * @sqlfn posechain_recv()
 */
Datum
Posechain_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PoseChain *result = posechain_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_send);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose chain
 * @sqlfn posechain_send()
 */
Datum
Posechain_send(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(pc);
  /* A pose chain always outputs the SRID */
  uint8_t *wkb = posechain_as_wkb(pc, WKB_EXTENDED, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Input/output in WKT, EWKT, and HexWKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_from_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_from_text);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return a pose chain from its Well-Known Text (WKT) representation
 * @sqlfn posechainFromText()
 */
Datum
Posechain_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = text_to_cstring(txt);
  PoseChain *result = posechain_in(str);
  pfree(str);
  PG_FREE_IF_COPY(txt, 0);
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_from_ewkt);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return a pose chain from its Extended Well-Known Text (EWKT)
 * representation
 * @sqlfn posechainFromEWKT()
 */
Datum
Posechain_from_ewkt(PG_FUNCTION_ARGS)
{
  return Posechain_from_text(fcinfo);
}

PGDLLEXPORT Datum Posechain_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_text);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a pose chain
 * @sqlfn asText()
 */
Datum
Posechain_as_text(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = posechain_as_text(pc, dbl_dig_for_wkt);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Posechain_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_ewkt);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a pose
 * chain
 * @sqlfn asEWKT()
 */
Datum
Posechain_as_ewkt(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = posechain_as_ewkt(pc, dbl_dig_for_wkt);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output in WKB and HexWKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_from_wkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return a pose chain from its Well-Known Binary (WKB) representation
 * @sqlfn posechainFromBinary()
 */
Datum
Posechain_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  PoseChain *result = posechain_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_from_hexwkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return a pose chain from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn posechainFromHexEWKB()
 */
Datum
Posechain_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  PoseChain *result = posechain_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_wkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose chain
 * @sqlfn asBinary()
 */
Datum
Posechain_as_wkb(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(pc), T_POSECHAIN,
    false));
}

PGDLLEXPORT Datum Posechain_as_ewkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_ewkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the Extended Well-Known Binary (EWKB) representation of a
 * pose chain
 * @note It is the WKB representation prefixed with the SRID
 * @sqlfn asEWKB()
 */
Datum
Posechain_as_ewkb(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(pc), T_POSECHAIN,
    true));
}

PGDLLEXPORT Datum Posechain_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_hexwkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a pose chain
 * @sqlfn asHexWKB()
 */
Datum
Posechain_as_hexwkb(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, PointerGetDatum(pc), T_POSECHAIN,
    false));
}

PGDLLEXPORT Datum Posechain_as_hexewkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_as_hexewkb);
/**
 * @ingroup mobilitydb_posechain_base_inout
 * @brief Return the ASCII hex-encoded Extended Well-Known Binary (HexEWKB)
 * representation of a pose chain
 * @note It is the HexWKB representation prefixed with the SRID
 * @sqlfn asHexEWKB()
 */
Datum
Posechain_as_hexewkb(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, PointerGetDatum(pc), T_POSECHAIN,
    true));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_constructor);
/**
 * @ingroup mobilitydb_posechain_base_constructor
 * @brief Return a pose chain from an array of poses ordered from the
 * outermost frame inwards
 * @sqlfn posechain()
 */
Datum
Posechain_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  int count;
  Datum *values = datumarr_extract(array, &count);
  Pose **poses = palloc(sizeof(Pose *) * count);
  for (int i = 0; i < count; i++)
    poses[i] = DatumGetPoseP(values[i]);
  PoseChain *result = posechain_make((const Pose **) poses, count);
  pfree(poses); pfree(values);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_append_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_append_pose);
/**
 * @ingroup mobilitydb_posechain_base_constructor
 * @brief Return a pose chain with a pose appended as its innermost link
 * @sqlfn appendPose()
 */
Datum
Posechain_append_pose(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  PG_RETURN_POSECHAIN_P(posechain_append(pc, pose));
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_posechain);
/**
 * @ingroup mobilitydb_posechain_base_conversion
 * @brief Convert a pose into a pose chain of a single link
 * @sqlfn posechain()
 */
Datum
Pose_to_posechain(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_POSECHAIN_P(pose_to_posechain(pose));
}

PGDLLEXPORT Datum Posechain_to_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_to_pose);
/**
 * @ingroup mobilitydb_posechain_base_conversion
 * @brief Convert a pose chain into the pose of its innermost frame, read in
 * the outer frame of the chain
 * @sqlfn pose()
 */
Datum
Posechain_to_pose(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_POSE_P(posechain_to_pose(pc));
}

PGDLLEXPORT Datum Posechain_prefix_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_prefix_pose);
/**
 * @ingroup mobilitydb_posechain_base_conversion
 * @brief Return the pose of the frame the first n links of a pose chain
 * define, read in the outer frame of the chain
 * @sqlfn pose()
 */
Datum
Posechain_prefix_pose(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int n = PG_GETARG_INT32(1);
  Pose *result = posechain_prefix_pose(pc, n);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Posechain_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_to_point);
/**
 * @ingroup mobilitydb_posechain_base_conversion
 * @brief Convert a pose chain into the geometry point of its innermost frame
 * @sqlfn point()
 */
Datum
Posechain_to_point(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_GSERIALIZED_P(posechain_to_point(pc));
}

PGDLLEXPORT Datum Posechain_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_to_stbox);
/**
 * @ingroup mobilitydb_posechain_base_conversion
 * @brief Convert a pose chain into a spatiotemporal box
 * @sqlfn stbox()
 */
Datum
Posechain_to_stbox(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_STBOX_P(posechain_to_stbox(pc));
}

PGDLLEXPORT Datum Posechain_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_posechain_base_box
 * @brief Construct a spatiotemporal box from a pose chain and a timestamptz
 * @sqlfn stbox()
 */
Datum
Posechain_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(posechain_timestamptz_to_stbox(pc, t));
}

PGDLLEXPORT Datum Posechain_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_posechain_base_box
 * @brief Construct a spatiotemporal box from a pose chain and a timestamptz
 * span
 * @sqlfn stbox()
 */
Datum
Posechain_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(posechain_tstzspan_to_stbox(pc, s));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_num_poses(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_num_poses);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the number of links of a pose chain
 * @sqlfn numPoses()
 */
Datum
Posechain_num_poses(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_INT32(posechain_num_poses(pc));
}

PGDLLEXPORT Datum Posechain_start_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_start_pose);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the outermost link of a pose chain
 * @sqlfn startPose()
 */
Datum
Posechain_start_pose(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_POSE_P(posechain_start_pose(pc));
}

PGDLLEXPORT Datum Posechain_end_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_end_pose);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the innermost link of a pose chain
 * @sqlfn endPose()
 */
Datum
Posechain_end_pose(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_POSE_P(posechain_end_pose(pc));
}

PGDLLEXPORT Datum Posechain_pose_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_pose_n);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the n-th link of a pose chain, in the frame the link before
 * it defines
 * @sqlfn poseN()
 */
Datum
Posechain_pose_n(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int n = PG_GETARG_INT32(1);
  Pose *result = posechain_pose_n(pc, n);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Posechain_poses(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_poses);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the array of links of a pose chain
 * @sqlfn poses()
 */
Datum
Posechain_poses(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int count;
  Pose **poses = posechain_poses(pc, &count);
  /* The helper frees both the poses and the array holding them */
  ArrayType *result = posearr_to_array(poses, count, true);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_round);
/**
 * @ingroup mobilitydb_posechain_base_transf
 * @brief Return a pose chain with the values of its links rounded to a number
 * of decimal places
 * @sqlfn round()
 */
Datum
Posechain_round(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int maxdd = PG_GETARG_INT32(1);
  PG_RETURN_POSECHAIN_P(posechain_round(pc, maxdd));
}

/*****************************************************************************
 * Spatial reference system functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_srid);
/**
 * @ingroup mobilitydb_posechain_base_srid
 * @brief Return the SRID of the outer frame of a pose chain
 * @sqlfn SRID()
 */
Datum
Posechain_srid(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_INT32(posechain_srid(pc));
}

PGDLLEXPORT Datum Posechain_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_set_srid);
/**
 * @ingroup mobilitydb_posechain_base_srid
 * @brief Return a pose chain with its outer frame set to an SRID
 * @sqlfn setSRID()
 */
Datum
Posechain_set_srid(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  PG_RETURN_POSECHAIN_P(posechain_set_srid(pc, srid));
}

PGDLLEXPORT Datum Posechain_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_transform);
/**
 * @ingroup mobilitydb_posechain_base_srid
 * @brief Return a pose chain transformed to another SRID
 * @sqlfn transform()
 */
Datum
Posechain_transform(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  PoseChain *result = posechain_transform(pc, srid);
  PG_FREE_IF_COPY(pc, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POSECHAIN_P(result);
}

PGDLLEXPORT Datum Posechain_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_transform_pipeline);
/**
 * @ingroup mobilitydb_posechain_base_srid
 * @brief Return a pose chain transformed to another SRID using a
 * transformation pipeline
 * @sqlfn transformPipeline()
 */
Datum
Posechain_transform_pipeline(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipeline = text_to_cstring(pipelinetxt);
  PoseChain *result = posechain_transform_pipeline(pc, pipeline, srid,
    is_forward);
  pfree(pipeline);
  PG_FREE_IF_COPY(pc, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POSECHAIN_P(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_same);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if two pose chains are equal up to the tolerance of the
 * comparison of floating-point values
 * @sqlfn same()
 * @sqlop @p ~=
 */
Datum
Posechain_same(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_same(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_eq);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is equal to the second one
 * @sqlfn eq()
 * @sqlop @p =
 */
Datum
Posechain_eq(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_eq(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_ne);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is not equal to the second one
 * @sqlfn ne()
 * @sqlop @p <>
 */
Datum
Posechain_ne(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_ne(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_cmp);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pose chain is
 * less than, equal to, or greater than the second one
 * @sqlfn cmp()
 */
Datum
Posechain_cmp(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_INT32(posechain_cmp(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_lt);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is less than the second one
 * @sqlfn lt()
 * @sqlop @p <
 */
Datum
Posechain_lt(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_lt(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_le);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is less than or equal to the
 * second one
 * @sqlfn le()
 * @sqlop @p <=
 */
Datum
Posechain_le(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_le(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_ge);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is greater than or equal to the
 * second one
 * @sqlfn ge()
 * @sqlop @p >=
 */
Datum
Posechain_ge(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_ge(pc1, pc2));
}

PGDLLEXPORT Datum Posechain_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_gt);
/**
 * @ingroup mobilitydb_posechain_base_comp
 * @brief Return true if the first pose chain is greater than the second one
 * @sqlfn gt()
 * @sqlop @p >
 */
Datum
Posechain_gt(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  PG_RETURN_BOOL(posechain_gt(pc1, pc2));
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

PGDLLEXPORT Datum Posechain_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_hash);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the 32-bit hash value of a pose chain
 * @sqlfn hash()
 */
Datum
Posechain_hash(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  PG_RETURN_UINT32(posechain_hash(pc));
}

PGDLLEXPORT Datum Posechain_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Posechain_hash_extended);
/**
 * @ingroup mobilitydb_posechain_base_accessor
 * @brief Return the 64-bit hash value of a pose chain using a seed
 * @sqlfn hashExtended()
 */
Datum
Posechain_hash_extended(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  PG_RETURN_UINT64(posechain_hash_extended(pc, seed));
}

/*****************************************************************************/
