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
 * @brief Basic functions for pose objects
 */

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/heaptoast.h>
#include <lib/stringinfo.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos_pose.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/type_inout.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/temporal.h"

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_in);
/**
 * @ingroup mobilitydb_base_inout
 * @brief Input function for pose values
 * @details Example of input:
 *    (1, 0.5)
 * @sqlfn pose_in()
 */
Datum
Pose_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(pose_in(str));
}

PGDLLEXPORT Datum Pose_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_out);
/**
 * @ingroup mobilitydb_base_inout
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
 * @ingroup mobilitydb_base_inout
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
 * @ingroup mobilitydb_base_inout
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
 * Output in WKT and EWKT representation
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a pose
 * @sqlfn asText()
 */
static Datum
Pose_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? pose_as_ewkt(pose, dbl_dig_for_wkt) : 
    pose_as_text(pose, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Pose_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a pose
 * @sqlfn asText()
 */
Datum
Pose_as_text(PG_FUNCTION_ARGS)
{
  return Pose_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Pose_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a pose
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Pose_as_ewkt(PG_FUNCTION_ARGS)
{
  return Pose_as_text_ext(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Pose_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_from_wkb);
/**
 * @ingroup mobilitydb_base_inout
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
 * @ingroup mobilitydb_base_inout
 * @brief Return a pose from its hex-encoded ASCII Well-Known Binary
 * (HexWKB) representation
 * @sqlfn poseFromHexWKB()
 */
Datum
Pose_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Pose *result = pose_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POSE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Pose_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_wkb);
/**
 * @ingroup mobilitydb_base_inout
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

PGDLLEXPORT Datum Pose_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_as_hexwkb);
/**
 * @ingroup mobilitydb_base_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
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
 * Constructors
 *****************************************************************************/

PGDLLEXPORT Datum Pose_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_constructor);
/**
 * @ingroup mobilitydb_base_constructor
 * @brief Construct a pose value from the arguments
 * @sqlfn pose()
 */
Datum
Pose_constructor(PG_FUNCTION_ARGS)
{
  double x = PG_GETARG_FLOAT8(0);
  double y = PG_GETARG_FLOAT8(1);
  double z = PG_GETARG_FLOAT8(2);
  double theta = z;

  assert(PG_NARGS() == 3 || PG_NARGS() == 7);
  Pose *result;
  if (PG_NARGS() == 3)
    result = pose_make_2d(x, y, theta);
  else /* PG_NARGS() == 7 */
  {
    double W = PG_GETARG_FLOAT8(3);
    double X = PG_GETARG_FLOAT8(4);
    double Y = PG_GETARG_FLOAT8(5);
    double Z = PG_GETARG_FLOAT8(6);
    result = pose_make_3d(x, y, z, W, X, Y, Z);
  }

  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_geom);
/**
 * @ingroup mobilitydb_base_conversion
 * @brief Transforms a pose into a geometry point
 * @sqlfn geometry()
 */
Datum
Pose_to_geom(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  GSERIALIZED *result = pose_point(pose);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Pose_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_round);
/**
 * @ingroup mobilitydb_base_transf
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

PGDLLEXPORT Datum Poseset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Poseset_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return an array of poses with the precision of the values set to a
 * number of decimal values
 * @sqlfn round()
 */
Datum
Poseset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int maxdd = PG_GETARG_INT32(1);
  Set *result = poseset_round(s, maxdd);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Approximate equality for poses
 *****************************************************************************/

PGDLLEXPORT Datum Pose_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_same);
/**
 * @ingroup mobilitydb_base_spatial
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_comp
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
 * @ingroup mobilitydb_base_accessor
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
 * @ingroup mobilitydb_base_accessor
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
