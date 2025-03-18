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
 * @brief General functions for temporal pose objects.
 */

#include "pose/tpose.h"

/* C */
#include <assert.h>
/* Postgres */
#include <postgres.h>
#include <common/hashfn.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "pose/pose.h"

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_base_conversion
 * @brief Return a geometry point from a temporal pose
 * @param[in] temp Temporal pose
 */
Temporal *
tpose_tgeompoint(const Temporal *temp)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_point;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal pose with the precision of the values set to a
 * number of decimal places
 */
Temporal *
tpose_round(const Temporal *temp, Datum size)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_round;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.argtype[0]= temp->temptype;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
