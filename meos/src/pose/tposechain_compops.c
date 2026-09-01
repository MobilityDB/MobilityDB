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
 * documentation for any purposechain, without fee, and without a written
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
 * AND FITNESS FOR A PARTICULAR PURPOSECHAIN. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Ever/always and temporal comparisons for temporal posechains
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/posechain.h"
#include "pose/tposechain.h"
#include "pose/tposechain.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal posechain and a posechain satisfy the ever/always
 * comparison
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tposechain_posechain(const Temporal *temp, const PoseChain *posechain,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_posechain(temp, posechain))
    return -1;
  assert(func);
  return eacomp_temporal_base(temp, PointerGetDatum(posechain), func, ever);
}

/**
 * @brief Return true if two temporal posechains satisfy the ever/always
 * comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_tposechain(temp1, temp2))
    return -1;
  assert(func);
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a posechain is ever equal to a temporal pose chain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_posechain_tposechain()
 */
int
ever_eq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_eq, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever equal to a pose chain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Ever_eq_tposechain_posechain()
 */
int
ever_eq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_eq, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a posechain is ever different from a temporal posechain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_posechain_tposechain()
 */
int
ever_ne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_ne, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a temporal posechain is ever different from a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Ever_ne_tposechain_posechain()
 */
int
ever_ne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_ne, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a posechain is always equal to a temporal posechain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_posechain_tposechain()
 */
int
always_eq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a temporal posechain is always equal to a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Always_eq_tposechain_posechain()
 */
int
always_eq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a posechain is always different from a temporal posechain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_posechain_tposechain()
 */
int
always_ne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if a temporal posechain is always different from a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Always_ne_tposechain_posechain()
 */
int
always_ne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return eacomp_tposechain_posechain(temp, posechain, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if two temporal posechains are ever equal
 * @param[in] temp1,temp2 Temporal posechains
 * @csqlfn #Ever_eq_tposechain_tposechain()
 */
int
ever_eq_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tposechain_tposechain(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if two temporal posechains are ever different
 * @param[in] temp1,temp2 Temporal posechains
 * @csqlfn #Ever_ne_tposechain_tposechain()
 */
int
ever_ne_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tposechain_tposechain(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if two temporal posechains are always equal
 * @param[in] temp1,temp2 Temporal posechains
 * @csqlfn #Always_eq_tposechain_tposechain()
 */
int
always_eq_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tposechain_tposechain(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return true if two temporal posechains are always different
 * @param[in] temp1,temp2 Temporal posechains
 * @csqlfn #Always_ne_tposechain_tposechain()
 */
int
always_ne_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tposechain_tposechain(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a posechain and a temporal posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_posechain_tposechain(const PoseChain *posechain, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSECHAIN(temp, NULL); VALIDATE_NOT_NULL(posechain, NULL);
  assert(func);
  if (! ensure_same_srid(tspatial_srid(temp), posechain_srid(posechain)))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(posechain), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal posechain and a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tposechain_posechain(const Temporal *temp, const PoseChain *posechain,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSECHAIN(temp, NULL); VALIDATE_NOT_NULL(posechain, NULL);
  assert(func);
  if (! ensure_same_srid(tspatial_srid(temp), posechain_srid(posechain)))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(posechain), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_posechain_comp_temp
 * @brief Return the temporal equality of a posechain and a temporal posechain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Teq_posechain_tposechain()
 */
Temporal *
teq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return tcomp_posechain_tposechain(posechain, temp, &datum2_eq);
}

/**
 * @ingroup meos_posechain_comp_temp
 * @brief Return the temporal inequality of a posechain and a temporal posechain
 * @param[in] posechain PoseChain
 * @param[in] temp Temporal value
 * @csqlfn #Tne_posechain_tposechain()
 */
Temporal *
tne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp)
{
  return tcomp_posechain_tposechain(posechain, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_posechain_comp_temp
 * @brief Return the temporal equality of a temporal posechain and a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Teq_tposechain_posechain()
 */
Temporal *
teq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return tcomp_tposechain_posechain(temp, posechain, &datum2_eq);
}

/**
 * @ingroup meos_posechain_comp_temp
 * @brief Return the temporal inequality of a temporal posechain and a
 * posechain
 * @param[in] temp Temporal value
 * @param[in] posechain PoseChain
 * @csqlfn #Tne_tposechain_posechain()
 */
Temporal *
tne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain)
{
  return tcomp_tposechain_posechain(temp, posechain, &datum2_ne);
}

/*****************************************************************************/
