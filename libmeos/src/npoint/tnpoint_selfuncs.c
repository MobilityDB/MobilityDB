/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tnpoint_selfuncs.c
 * @brief Functions for selectivity estimation of operators on temporal network
 * points.
 */

#include "npoint/tnpoint_selfuncs.h"

/* MobilityDB */
#include "general/temporal.h"
#include "general/temporal_selfuncs.h"
#include "point/tpoint_selfuncs.h"

/*****************************************************************************/

/**
 * Get the enum value associated to the operator
 */
bool
tnpoint_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
  {
    if (operid == oper_oid((CachedOp) i, T_GEOMETRY, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_NPOINT, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_STBOX, T_TNPOINT) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_GEOMETRY) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_NPOINT) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_STBOX) ||
        operid == oper_oid((CachedOp) i, T_TNPOINT, T_TNPOINT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/*****************************************************************************
 * Restriction selectivity
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_sel);
/**
 * Estimate the restriction selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
Tnpoint_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_ext(fcinfo, TNPOINTTYPE);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_joinsel);
/**
 * Estimate the join selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
Tnpoint_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TNPOINTTYPE);
}

/*****************************************************************************/
