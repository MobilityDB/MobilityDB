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
 * @brief Basic functions for temporal pose objects.
 */


#include "pose/tpose_static.h"

/* C */
#include <assert.h>
#include <math.h>
/* MobilityDB */
#include "general/pg_types.h"
#include "general/type_out.h"
#include "general/type_util.h"

/*****************************************************************************
 * Input/Output functions for pose
 *****************************************************************************/

PGDLLEXPORT Datum Pose_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_in);
/**
 * Input function for pose values
 * Example of input:
 *    (1, 0.5)
 */
Datum
Pose_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_POINTER(pose_in(str, true));
}

PGDLLEXPORT Datum Pose_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_out);
/**
 * Output function for pose values
 */
Datum
Pose_out(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  PG_RETURN_CSTRING(pose_out(pose, OUT_DEFAULT_DECIMAL_DIGITS));
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

PGDLLEXPORT Datum Pose_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_constructor);
/**
 * Construct a pose value from the arguments
 */
Datum
Pose_constructor(PG_FUNCTION_ARGS)
{
  double x = PG_GETARG_FLOAT8(0);
  double y = PG_GETARG_FLOAT8(1);
  double z = PG_GETARG_FLOAT8(2);
  double theta = z;

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
 * Casting to Point
 *****************************************************************************/

PGDLLEXPORT Datum Pose_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_to_geom);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transforms the pose into a geometry point
 * @sqlfn geometry()
 */
Datum
Pose_to_geom(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  GSERIALIZED *result = pose_geom(pose);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
