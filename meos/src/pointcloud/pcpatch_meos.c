/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Functions on pcpatch that MEOS alone states
 * @details The PostgreSQL extension calls these functions by the name with
 * the prefix pg_, since pgPointCloud gives their names to C functions of its
 * own, which the extension would otherwise call in their place.
 */

/* C */
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "pointcloud/pcpatch.h"

/**
 * @ingroup meos_pointcloud_base_srid
 * @brief Return a pcpatch transformed into the schema of another pcid, which
 * states the target SRID
 * @param[in] pa Patch
 * @param[in] pcid Target pcid
 * @csqlfn #Pcpatch_transform()
 */
Pcpatch *
pcpatch_transform(const Pcpatch *pa, uint32_t pcid)
{
  return pg_pcpatch_transform(pa, pcid);
}

/*****************************************************************************/
