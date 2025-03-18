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
 * @brief Output of temporal poses in WKT and EWKT format
 */

// #include "pose/tpose_out.h"

/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "geo/tspatial_parser.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "pose/tpose_parser.h"
#include "pose/tpose_spatialfuncs.h"

/*****************************************************************************
 * Input in WKT and EWKT format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal pose from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
tpose_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return tspatial_parse(&str, T_TPOSE);
}
#endif /* MEOS */

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tpose_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_negative(maxdd) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
  return temporal_out(temp, maxdd);
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpose_as_text()
 */
char *
tpose_as_text(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_string((TInstant *) temp, maxdd, &pose_wkt_out);
    case TSEQUENCE:
      return tsequence_to_string((TSequence *) temp, maxdd, false, &pose_wkt_out);
    default: /* TSEQUENCESET */
      return tsequenceset_to_string((TSequenceSet *) temp, maxdd, &pose_wkt_out);
  }
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal pose
 * @param[in] temp Temporal pose
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Tpose_as_ewkt()
 */
char *
tpose_as_ewkt(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_not_negative(maxdd))
    return NULL;

  int32_t srid = tspatial_srid(temp);
  char str1[18];
  if (srid > 0)
    /* SRID_MAXIMUM is defined by PostGIS as 999999 */
    snprintf(str1, sizeof(str1), "SRID=%d%c", srid,
      (MEOS_FLAGS_GET_INTERP(temp->flags) == STEP) ? ',' : ';');
  else
    str1[0] = '\0';
  char *str2 = tpose_as_text(temp, maxdd);
  char *result = palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************/
