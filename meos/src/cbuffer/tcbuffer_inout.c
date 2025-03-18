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
 * @brief Output of temporal circular buffers in WKT, EWKT, and MF-JSON format
 */

#include "cbuffer/tcbuffer_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/pg_types.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_parser.h"

/*****************************************************************************
 * Input in WKT and EWKT format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal circular buffer from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tcbuffer_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tspatial_parse(&str, T_TCBUFFER);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal
 * circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tcbuffer_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || 
      ! ensure_temporal_isof_type(temp, T_TCBUFFER))
    return NULL;
  return temporal_out(temp, maxdd);
}
#endif /* MEOS */


/*****************************************************************************/
