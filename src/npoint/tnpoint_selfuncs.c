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
 * tnpoint_selfuncs.c
 * Functions for selectivity estimation of operators on temporal network points
 */

#include "npoint/tnpoint_selfuncs.h"

/* MobilityDB */
#include "general/temporal_selfuncs.h"
#include "point/tpoint_selfuncs.h"

/*****************************************************************************/

/**
 * Get the enum value associated to the operator
 */
bool
tnpoint_cachedop(Oid oper, CachedOp *cachedOp)
{
  for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
  {
    if (oper == oper_oid((CachedOp) i, T_GEOMETRY, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_NPOINT, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_PERIOD, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_PERIODSET, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_STBOX, T_TNPOINT) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_GEOMETRY) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_NPOINT) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_TIMESTAMPTZ) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_TIMESTAMPSET) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_PERIOD) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_PERIODSET) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_STBOX) ||
        oper == oper_oid((CachedOp) i, T_TNPOINT, T_TNPOINT))
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

PG_FUNCTION_INFO_V1(tnpoint_sel);
/**
 * Estimate the restriction selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
tnpoint_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_generic(fcinfo, TNPOINTTYPE);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_joinsel);
/**
 * Estimate the join selectivity of the operators for temporal network points
 */
PGDLLEXPORT Datum
tnpoint_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_generic(fcinfo, TNPOINTTYPE);
}

/*****************************************************************************/
