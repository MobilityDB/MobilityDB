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
 * @file temporal.c
 * @brief Basic functions for temporal types of any subtype.
 */

#include "general/temporal.h"

/* PostgreSQL */
#include <assert.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#if POSTGRESQL_VERSION_NUMBER < 130000
#include <access/tuptoaster.h>
#else
#include <access/heaptoast.h>
#include <access/detoast.h>
#endif
#include <catalog/namespace.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/fmgroids.h>
#include <utils/lsyscache.h>
#include <utils/rel.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/doxygen_libmeos_api.h"
#include "general/period.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/temporal_boxops.h"
#include "general/temporal_parser.h"
#include "general/rangetypes_ext.h"
#include "general/tnumber_distance.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the minimum base value
 */
Temporal *
temporal_at_min(const Temporal *temp)
{
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the minimum base value
 */
Temporal *
temporal_minus_min(const Temporal *temp)
{
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_MINUS);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the maximum base value
 */
Temporal *
temporal_at_max(const Temporal *temp)
{
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the maximum base value
 */
Temporal *
temporal_minus_max(const Temporal *temp)
{
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_MINUS);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the timestamp
 */
Temporal *
temporal_at_timestamp(const Temporal *temp, TimestampTz t)
{
  Temporal *result = temporal_restrict_timestamp(temp, t, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the timestamp set
 */
Temporal *
temporal_minus_timestamp(const Temporal *temp, TimestampTz t)
{
  Temporal *result = temporal_restrict_timestamp(temp, t, REST_MINUS);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the timestamp set
 */
Temporal *
temporal_at_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the timestamp set
 */
Temporal *
temporal_minus_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_MINUS);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the period set
 */
Temporal *
temporal_at_periodset(const Temporal *temp, const PeriodSet *ps)
{
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_AT);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict the temporal value to the complement of the period set
 */
Temporal *
temporal_minus_periodset(const Temporal *temp, const PeriodSet *ps)
{
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_MINUS);
  return result;
}


