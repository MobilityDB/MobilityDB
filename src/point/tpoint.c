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
 * @file tpoint.c
 * @brief General functions for temporal points.
 */

#include "point/tpoint.h"

/* PostgreSQL */
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "point/stbox.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

#define PGC_ERRMSG_MAXLEN 2048

/**
 * Output an error message
 */
static void
pg_error(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN + 1];
  vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  errmsg[PGC_ERRMSG_MAXLEN]='\0';
  ereport(ERROR, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * Output a notice message
 */
static void
pg_notice(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN + 1];
  vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  errmsg[PGC_ERRMSG_MAXLEN]='\0';
  ereport(NOTICE, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * Set the handlers for initializing the liblwgeom library
 */
void
temporalgeom_init()
{
  lwgeom_set_handlers(palloc, repalloc, pfree, pg_error, pg_notice);
}

/**
 * Copy a GSERIALIZED. This function is not available anymore in PostGIS 3
 */
GSERIALIZED *
gserialized_copy(const GSERIALIZED *g)
{
  GSERIALIZED *result = (GSERIALIZED *) palloc(VARSIZE(g));
  memcpy(result, g, VARSIZE(g));
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_accessor
 * @brief Return the bounding box of the temporal point value
 */
STBOX *
tpoint_stbox(const Temporal *temp)
{
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  temporal_bbox(temp, result);
  return result;
}

/*****************************************************************************
 * Expand functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of the temporal point value expanded on the
 * spatial dimension
 */
STBOX *
geo_expand_spatial(const GSERIALIZED *gs, double d)
{
  if (gserialized_is_empty(gs))
    return NULL;
  STBOX box;
  geo_stbox(gs, &box);
  STBOX *result = stbox_expand_spatial(&box, d);
  return result;
}

/**
 * @ingroup libmeos_temporal_spatial_transf
 * @brief Return the bounding box of the temporal point value expanded on the
 * spatial dimension
 */
STBOX *
tpoint_expand_spatial(const Temporal *temp, double d)
{
  STBOX box;
  temporal_bbox(temp, &box);
  STBOX *result = stbox_expand_spatial(&box, d);
  return result;
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of the base value and temporal value
 */
Temporal *
tcomp_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, CachedType, CachedType), bool invert)
{
  if (gserialized_is_empty(gs))
    return NULL;
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  CachedType basetype = temptype_basetype(temp->temptype);
  Temporal *result = tcomp_temporal_base(temp, PointerGetDatum(gs), basetype,
    func, invert);
  return result;
}

/*****************************************************************************/
