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
 * @brief Functions for temporal rigid geometries of instant subtype
 */

#include "rgeo/trgeo_inst.h"

/* MEOS */
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"
#include "general/tinstant.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Returns the reference geometry of the temporal value
 */
const GSERIALIZED *
trgeoinst_geom_p(const TInstant *inst)
{
  if (! ensure_has_geom(inst->flags))
    return NULL;
  size_t value_size = DOUBLE_PAD(VARSIZE(&inst->value));
  return (const GSERIALIZED *) ((char *) &inst->value + value_size);
}

/*****************************************************************************/

/**
 * @brief Returns the size of the trgeometry instant without reference geometry
 */
size_t
trgeoinst_pose_varsize(const TInstant *inst)
{
  size_t size = VARSIZE(inst);
  if (MEOS_FLAGS_GET_GEOM(inst->flags))
    size -= DOUBLE_PAD(VARSIZE(trgeoinst_geom_p(inst)));
  return size;
}

/**
 * @brief Set the size of the trgeometry instant without reference geometry
 */
void
trgeoinst_set_pose(TInstant *inst)
{
  if (MEOS_FLAGS_GET_GEOM(inst->flags))
  {
    SET_VARSIZE(inst, trgeoinst_pose_varsize(inst));
    MEOS_FLAGS_SET_GEOM(inst->flags, NO_GEOM);
  }
  return;
}

/**
 * @brief Returns a new temporal pose instant obtained by removing the 
 * reference geometry of a temporal rigid geometry instant
 */
TInstant *
trgeoinst_tposeinst(const TInstant *inst)
{
  assert(inst->temptype == T_TRGEOMETRY);
  size_t inst_size = trgeoinst_pose_varsize(inst);
  TInstant *result = palloc(inst_size);
  memcpy(((char *)result), ((char *)inst), inst_size);
  MEOS_FLAGS_SET_GEOM(result->flags, false);
  result->temptype = T_TPOSE;
  return result;
}

/*****************************************************************************/

/**
 * @brief Ensure the validity of the arguments when creating a temporal value
 */
static bool
trgeoinst_make_valid(const GSERIALIZED *gs, const Pose *pose)
{
  ensure_not_empty(gs);
  ensure_has_not_M_geo(gs);
  int geomtype = gserialized_get_type(gs);
  bool hasZ = MEOS_FLAGS_GET_Z(pose->flags);
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_pose = pose_srid(pose);
  if (geomtype != POLYGONTYPE && geomtype != POLYHEDRALSURFACETYPE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only polygon or polyhedral surface geometries accepted");
    return false;
  }
  if (hasZ != (bool) FLAGS_GET_Z(gs->gflags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Dimension of geometry and pose must correspond");
    return false;
  }
  if (srid_pose != SRID_UNKNOWN && srid_pose != SRID_UNKNOWN &&
    srid_geom != srid_pose)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "SRID of geometry (%d) and pose (%d) must correspond", srid_geom,
        srid_pose);
    return false;
  }
  return true;
}

/**
 * @brief Creating a temporal value from its arguments
 * @pre The validity of the arguments has been tested before
 */
TInstant *
trgeoinst_make1(const GSERIALIZED *geom, const Pose *pose, TimestampTz t)
{
  size_t value_offset = sizeof(TInstant) - sizeof(Datum);
  size_t size = value_offset;
  /* Create the temporal instant */
  void *value_from = (void *) pose;
  size_t value_size = DOUBLE_PAD(VARSIZE(value_from));
  void *geom_from = (void *) geom;
  size_t geom_size = DOUBLE_PAD(VARSIZE(geom_from));
  size += value_size + geom_size;
  TInstant *result = palloc0(size);
  void *value_to = ((char *) result) + value_offset;
  memcpy(value_to, value_from, value_size);
  void *geom_to = ((char *) result) + value_offset + value_size;
  memcpy(geom_to, geom_from, geom_size);

  /* Initialize fixed-size values */
  result->temptype = T_TRGEOMETRY;
  result->subtype = TINSTANT;
  result->t = t;
  SET_VARSIZE(result, size);
  MEOS_FLAGS_SET_BYVAL(result->flags, false);
  MEOS_FLAGS_SET_CONTINUOUS(result->flags, true);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_T(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, MEOS_FLAGS_GET_Z(pose->flags));
  MEOS_FLAGS_SET_GEODETIC(result->flags, false);
  MEOS_FLAGS_SET_GEOM(result->flags, WITH_GEOM);
  return result;
}

/**
 * @ingroup meos_rgeo_constructor
 * @brief Construct a temporal geometry instant value from the arguments
 * @details The memory structure of a temporal geometry instant value is as
 * follows
 * @code
 * -----------------------------
 * ( TInstant )_X | ( Geom )_X |
 * -----------------------------
 * @endcode
 * where the attribute `value` of type `Datum` in the TInstant struct stores
 * the base value (pose). The `_X` are unused bytes added for double padding
 * @param geom Reference geometry
 * @param pose Pose
 * @param t Timestamp
 */
TInstant *
trgeoinst_make(const GSERIALIZED *geom, const Pose *pose, TimestampTz t)
{
  VALIDATE_NOT_NULL(geom, NULL); VALIDATE_NOT_NULL(pose, NULL);
  if (! trgeoinst_make_valid(geom, pose))
    return NULL;
  return trgeoinst_make1(geom, pose, t);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal sequence transformed into a temporal instant
 * @param[in] seq Temporal sequence
 */
TInstant *
trgeoseq_to_tinstant(const TSequence *seq)
{
  assert(seq);
  if (seq->count != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal instant");
    return NULL;
  }
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  return trgeoinst_make(trgeoseq_geom_p(seq),
    DatumGetPoseP(tinstant_value_p(inst)), inst->t);
}

/**
 * @ingroup meos_internal_rgeo_transf
 * @brief Return a temporal sequence set transformed into a temporal instant
 * @param[in] ss Temporal sequence set
 */
TInstant *
trgeoseqset_to_tinstant(const TSequenceSet *ss)
{
  assert(ss);
  if (ss->totalcount != 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot transform input value to a temporal instant");
    return NULL;
  }
  const TSequence *seq = TSEQUENCESET_SEQ_N(ss, 0);
  const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
  return trgeoinst_make(trgeoseqset_geom_p(ss),
    DatumGetPoseP(tinstant_value_p(inst)), inst->t);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/*****************************************************************************/
