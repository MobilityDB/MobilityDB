/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
#include <utils/timestamp.h>
#include <utils/varlena.h>
#include <common/hashfn.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/tsequence.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#if CBUFFER
  #include <meos_cbuffer.h>
  #include "cbuffer/cbuffer.h"
#endif
#if H3
  #include <meos_h3.h>
  #include "h3/h3index.h"
#endif
#if JSON
  #include <meos_json.h>
  #include <utils/jsonb.h>
#endif
#if NPOINT
  #include <meos_npoint.h>
  #include "npoint/tnpoint.h"
#endif
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include "pointcloud/pcpoint.h"
  #include "pointcloud/pcpatch.h"
#endif
#if POSE || RGEO
  #include <meos_pose.h>
  #include "pose/pose.h"
#endif
#if POSE
  #include "pose/posechain.h"
#endif
#if QUADBIN
  #include <meos_quadbin.h>
  #include "quadbin/quadbin.h"
#endif
#if RGEO
  #include <meos_rgeo.h>
#endif
#if S2CELL
  #include <meos_s2cell.h>
  #include "s2cell/s2cell.h"
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
 * @brief Return a temporal big integer instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tbigintinst_in(const char *str)
{
  return tinstant_in(str, T_TBIGINT);
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
 * @brief Return a temporal big integer instant from a big integer and a
 * timestamptz
 * @param[in] i Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tbigintinst_make(int64 i, TimestampTz t)
{
  return tinstant_make(Int64GetDatum(i), T_TBIGINT, t);
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

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal boolean instant
 * @param[in] temp Temporal value
 * @errval false
 * @csqlfn #Tinstant_value()
 */
bool
tbool_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, false);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return false;
  return DatumGetBool(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal integer instant
 * @param[in] temp Temporal value
 * @errval INT_MAX
 * @csqlfn #Tinstant_value()
 */
int
tint_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, INT_MAX);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return INT_MAX;
  return DatumGetInt32(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal big integer instant
 * @param[in] temp Temporal value
 * @errval INT64_MAX
 * @csqlfn #Tinstant_value()
 */
int64
tbigint_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBIGINT(temp, INT64_MAX);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return INT64_MAX;
  return DatumGetInt64(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the value of a temporal float instant
 * @param[in] temp Temporal value
 * @errval DBL_MAX
 * @csqlfn #Tinstant_value()
 */
double
tfloat_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, DBL_MAX);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return DBL_MAX;
  return DatumGetFloat8(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the value of a temporal text instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
text *
ttext_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TTEXT(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetTextP(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_geo_accessor
 * @brief Return a copy of the value of a temporal geo instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
GSERIALIZED *
tgeo_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetGserializedP(tinstant_value((const TInstant *) temp));
}

#if CBUFFER
/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the value of a temporal circular buffer instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Cbuffer *
tcbuffer_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetCbufferP(tinstant_value((const TInstant *) temp));
}
#endif /* CBUFFER */

#if H3
/**
 * @ingroup meos_h3_accessor
 * @brief Return the value of a temporal H3 index instant
 * @param[in] temp Temporal value
 * @errval 0
 * @csqlfn #Tinstant_value()
 */
H3Index
th3index_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TH3INDEX(temp, (H3Index) 0);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return (H3Index) 0;
  return DatumGetH3Index(tinstant_value((const TInstant *) temp));
}
#endif /* H3 */

#if JSON
/**
 * @ingroup meos_json_accessor
 * @brief Return a copy of the value of a temporal JSONB instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Jsonb *
tjsonb_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetJsonbP(tinstant_value((const TInstant *) temp));
}
#endif /* JSON */

#if NPOINT
/**
 * @ingroup meos_npoint_accessor
 * @brief Return a copy of the value of a temporal network point instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Npoint *
tnpoint_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetNpointP(tinstant_value((const TInstant *) temp));
}
#endif /* NPOINT */

#if POINTCLOUD
/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return a copy of the value of a temporal pgpointcloud point instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Pcpoint *
tpcpoint_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPOINT(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetPcpointP(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_pointcloud_accessor
 * @brief Return a copy of the value of a temporal pgpointcloud patch instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Pcpatch *
tpcpatch_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPCPATCH(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetPcpatchP(tinstant_value((const TInstant *) temp));
}
#endif /* POINTCLOUD */

#if POSE
/**
 * @ingroup meos_pose_accessor
 * @brief Return a copy of the value of a temporal pose instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Pose *
tpose_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetPoseP(tinstant_value((const TInstant *) temp));
}

/**
 * @ingroup meos_posechain_accessor
 * @brief Return a copy of the value of a temporal pose chain instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
PoseChain *
tposechain_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSECHAIN(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetPoseChainP(tinstant_value((const TInstant *) temp));
}
#endif /* POSE */

#if QUADBIN
/**
 * @ingroup meos_quadbin_accessor
 * @brief Return the value of a temporal quadbin instant
 * @param[in] temp Temporal value
 * @errval 0
 * @csqlfn #Tinstant_value()
 */
Quadbin
tquadbin_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TQUADBIN(temp, (Quadbin) 0);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return (Quadbin) 0;
  return DatumGetQuadbin(tinstant_value((const TInstant *) temp));
}
#endif /* QUADBIN */

#if RGEO
/**
 * @ingroup meos_rgeo_accessor
 * @brief Return a copy of the pose of a temporal rigid geometry instant
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Tinstant_value()
 */
Pose *
trgeometry_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return NULL;
  return DatumGetPoseP(tinstant_value((const TInstant *) temp));
}
#endif /* RGEO */

#if S2CELL
/**
 * @ingroup meos_s2cell_accessor
 * @brief Return the value of a temporal S2 cell instant
 * @param[in] temp Temporal value
 * @errval 0
 * @csqlfn #Tinstant_value()
 */
S2CellId
ts2cell_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TS2CELL(temp, (S2CellId) 0);
  if (! ensure_temporal_isof_subtype(temp, TINSTANT))
    return (S2CellId) 0;
  return DatumGetS2Cell(tinstant_value((const TInstant *) temp));
}
#endif /* S2CELL */

/*****************************************************************************/
