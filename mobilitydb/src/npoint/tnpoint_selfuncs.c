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
 * @brief Functions for selectivity estimation of operators on temporal network
 * points.
 */

#include "pg_npoint/tnpoint_selfuncs.h"

/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal_selfuncs.h"

/*****************************************************************************/

/**
 * @brief Get the enum value associated to the operator
 */
bool
tnpoint_oper_sel(Oid operid __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) ||
        timespan_type(ltype) || timespanset_type(ltype) ||
        spatial_basetype(ltype) || ltype == T_STBOX || tspatial_type(ltype)) &&
      (timespan_basetype(rtype) || timeset_type(rtype) ||
        timespan_type(rtype) || timespanset_type(rtype) ||
        spatial_basetype(rtype) || rtype == T_STBOX || tspatial_type(rtype)))
    return true;
  return false;
}

/*****************************************************************************
 * Restriction selectivity
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_sel);
/**
 * @brief Estimate the restriction selectivity of the operators for temporal
 * network points
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
 * @brief Estimate the join selectivity of the operators for temporal network
 * points
 */
PGDLLEXPORT Datum
Tnpoint_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TNPOINTTYPE);
}

/*****************************************************************************/
