/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Ever/always and temporal comparisons for the temporal pgpointcloud
 *   types
 *
 * Mirrors @c meos/src/cbuffer/tcbuffer_compops.c and
 * @c meos/src/quadbin/tquadbin_compops.c: every flavour is a thin typed
 * wrapper over the generic @c eacomp_* / @c tcomp_* kernels, so the temporal
 * pgpointcloud types carry the same orthogonal comparison grid as the other
 * spatial families.
 *
 * @note Two values with different pgpointcloud schemas (pcid) simply compare
 * unequal here; a mismatch is not an error. That is this family's established
 * convention — its restriction functions return NULL rather than raising on a
 * pcid mismatch — and it keeps the behaviour of the already-deployed
 * @c eEq / @c aEq, which bind the generic @c Ever_eq_temporal_base,
 * unchanged. The pcid is enforced where mixing schemas would build an
 * ill-formed value, namely set construction (@c ensure_same_pcid_pcpoint in
 * @c pcset_meos.c).
 *
 * Ordering comparisons (@c <, @c <=, @c >, @c >=) are not lifted: only
 * equality and inequality are exposed, as in the other spatial families.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal pgpointcloud point and a pcpoint satisfy
 * the ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @param[in] func Comparison function
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eacomp_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(func);
  VALIDATE_TPCPOINT(temp, -1); VALIDATE_NOT_NULL(pt, -1);
  return eacomp_temporal_base(temp, PointerGetDatum(pt), func, ever);
}

/**
 * @brief Return true if two temporal pgpointcloud points satisfy the
 * ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Comparison function
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eacomp_tpcpoint_tpcpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(func);
  VALIDATE_TPCPOINT(temp1, -1); VALIDATE_TPCPOINT(temp2, -1);
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/**
 * @brief Return true if a temporal pgpointcloud patch and a pcpatch satisfy
 * the ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @param[in] func Comparison function
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eacomp_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(func);
  VALIDATE_TPCPATCH(temp, -1); VALIDATE_NOT_NULL(pa, -1);
  return eacomp_temporal_base(temp, PointerGetDatum(pa), func, ever);
}

/**
 * @brief Return true if two temporal pgpointcloud patches satisfy the
 * ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Comparison function
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eacomp_tpcpatch_tpcpatch(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  assert(func);
  VALIDATE_TPCPATCH(temp1, -1); VALIDATE_TPCPATCH(temp2, -1);
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpoint is ever equal to a temporal pgpointcloud
 * point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_pcpoint_tpcpoint()
 */
int
ever_eq_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is ever equal to a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Ever_eq_tpcpoint_pcpoint()
 */
int
ever_eq_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are ever equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_eq_tpcpoint_tpcpoint()
 */
int
ever_eq_tpcpoint_tpcpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpoint_tpcpoint(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpoint is ever different from a temporal
 * pgpointcloud point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_pcpoint_tpcpoint()
 */
int
ever_ne_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is ever different from a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Ever_ne_tpcpoint_pcpoint()
 */
int
ever_ne_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are ever different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_ne_tpcpoint_tpcpoint()
 */
int
ever_ne_tpcpoint_tpcpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpoint_tpcpoint(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpoint is always equal to a temporal pgpointcloud
 * point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_pcpoint_tpcpoint()
 */
int
always_eq_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is always equal to a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Always_eq_tpcpoint_pcpoint()
 */
int
always_eq_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are always equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_eq_tpcpoint_tpcpoint()
 */
int
always_eq_tpcpoint_tpcpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpoint_tpcpoint(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpoint is always different from a temporal
 * pgpointcloud point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_pcpoint_tpcpoint()
 */
int
always_ne_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud point is always different from
 * a pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Always_ne_tpcpoint_pcpoint()
 */
int
always_ne_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return eacomp_tpcpoint_pcpoint(temp, pt, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud points are always different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_ne_tpcpoint_tpcpoint()
 */
int
always_ne_tpcpoint_tpcpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpoint_tpcpoint(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpatch is ever equal to a temporal pgpointcloud
 * patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_pcpatch_tpcpatch()
 */
int
ever_eq_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is ever equal to a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Ever_eq_tpcpatch_pcpatch()
 */
int
ever_eq_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are ever equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_eq_tpcpatch_tpcpatch()
 */
int
ever_eq_tpcpatch_tpcpatch(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpatch_tpcpatch(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpatch is ever different from a temporal
 * pgpointcloud patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_pcpatch_tpcpatch()
 */
int
ever_ne_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is ever different from a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Ever_ne_tpcpatch_pcpatch()
 */
int
ever_ne_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are ever different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Ever_ne_tpcpatch_tpcpatch()
 */
int
ever_ne_tpcpatch_tpcpatch(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpatch_tpcpatch(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpatch is always equal to a temporal pgpointcloud
 * patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_pcpatch_tpcpatch()
 */
int
always_eq_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is always equal to a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Always_eq_tpcpatch_pcpatch()
 */
int
always_eq_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are always equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_eq_tpcpatch_tpcpatch()
 */
int
always_eq_tpcpatch_tpcpatch(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpatch_tpcpatch(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a pcpatch is always different from a temporal
 * pgpointcloud patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_pcpatch_tpcpatch()
 */
int
always_ne_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if a temporal pgpointcloud patch is always different from
 * a pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Always_ne_tpcpatch_pcpatch()
 */
int
always_ne_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return eacomp_tpcpatch_pcpatch(temp, pa, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_pointcloud_comp_ever
 * @brief Return true if two temporal pgpointcloud patches are always different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Always_ne_tpcpatch_tpcpatch()
 */
int
always_ne_tpcpatch_tpcpatch(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpcpatch_tpcpatch(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a pcpoint and a temporal
 * pgpointcloud point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(func);
  VALIDATE_TPCPOINT(temp, NULL); VALIDATE_NOT_NULL(pt, NULL);
  return tcomp_base_temporal(PointerGetDatum(pt), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal pgpointcloud point and a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(func);
  VALIDATE_TPCPOINT(temp, NULL); VALIDATE_NOT_NULL(pt, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(pt), func);
}

/**
 * @brief Return the temporal comparison of a pcpatch and a temporal
 * pgpointcloud patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(func);
  VALIDATE_TPCPATCH(temp, NULL); VALIDATE_NOT_NULL(pa, NULL);
  return tcomp_base_temporal(PointerGetDatum(pa), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal pgpointcloud patch and a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa,
  Datum (*func)(Datum, Datum, MeosType))
{
  assert(func);
  VALIDATE_TPCPATCH(temp, NULL); VALIDATE_NOT_NULL(pa, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(pa), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal equality of a pcpoint and a temporal
 * pgpointcloud point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Teq_pcpoint_tpcpoint()
 */
Temporal *
teq_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return tcomp_pcpoint_tpcpoint(pt, temp, &datum2_eq);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal inequality of a pcpoint and a temporal
 * pgpointcloud point
 * @param[in] pt Point cloud point
 * @param[in] temp Temporal value
 * @csqlfn #Tne_pcpoint_tpcpoint()
 */
Temporal *
tne_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp)
{
  return tcomp_pcpoint_tpcpoint(pt, temp, &datum2_ne);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal equality of a temporal pgpointcloud point and a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Teq_tpcpoint_pcpoint()
 */
Temporal *
teq_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return tcomp_tpcpoint_pcpoint(temp, pt, &datum2_eq);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal inequality of a temporal pgpointcloud point and a
 * pcpoint
 * @param[in] temp Temporal value
 * @param[in] pt Point cloud point
 * @csqlfn #Tne_tpcpoint_pcpoint()
 */
Temporal *
tne_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt)
{
  return tcomp_tpcpoint_pcpoint(temp, pt, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal equality of a pcpatch and a temporal
 * pgpointcloud patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Teq_pcpatch_tpcpatch()
 */
Temporal *
teq_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return tcomp_pcpatch_tpcpatch(pa, temp, &datum2_eq);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal inequality of a pcpatch and a temporal
 * pgpointcloud patch
 * @param[in] pa Point cloud patch
 * @param[in] temp Temporal value
 * @csqlfn #Tne_pcpatch_tpcpatch()
 */
Temporal *
tne_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp)
{
  return tcomp_pcpatch_tpcpatch(pa, temp, &datum2_ne);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal equality of a temporal pgpointcloud patch and a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Teq_tpcpatch_pcpatch()
 */
Temporal *
teq_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return tcomp_tpcpatch_pcpatch(temp, pa, &datum2_eq);
}

/**
 * @ingroup meos_pointcloud_comp_temp
 * @brief Return the temporal inequality of a temporal pgpointcloud patch and a
 * pcpatch
 * @param[in] temp Temporal value
 * @param[in] pa Point cloud patch
 * @csqlfn #Tne_tpcpatch_pcpatch()
 */
Temporal *
tne_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa)
{
  return tcomp_tpcpatch_pcpatch(temp, pa, &datum2_ne);
}

/*****************************************************************************/
