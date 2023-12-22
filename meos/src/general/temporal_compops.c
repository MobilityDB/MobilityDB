/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Temporal comparison operators: #=, #<>, #<, #>, #<=, #>=.
 */

#include "general/temporal_compops.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_comp_temp
 * @brief Return the temporal comparison of a temporal value and a base value.
 */
Temporal *
tcomp_temporal_base(const Temporal *temp, Datum value, meosType basetype,
  Datum (*func)(Datum, Datum, meosType), bool invert)
{
  /* Ensure validity of the arguments */
  if (tgeo_type(temp->temptype))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value);
    if (gserialized_is_empty(gs) ||
        ! ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs)) ||
        ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;
  }

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @ingroup libmeos_internal_temporal_comp_temp
 * @brief Return the temporal comparison of the temporal values.
 */
Temporal *
tcomp_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure validity of the arguments */
  if (tgeo_type(temp1->temptype))
  {
    if(! ensure_same_srid(tpoint_srid(temp1), tpoint_srid(temp2)) ||
       ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return NULL;
  }

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp1->temptype);
  lfinfo.argtype[1] = temptype_basetype(temp2->temptype);
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************/
