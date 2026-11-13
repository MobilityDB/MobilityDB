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

/*****************************************************************************/

/**
 * @brief Return 1 if a temporal pose chain ever (or always) lies in a
 * spatiotemporal box, 0 if not, and -1 on error, as #eaoverlaps_tpose_stbox
 * answers it for a temporal pose
 * @details The value ever lies in the box when its restriction to the box by
 * #tposechain_restrict_stbox is not empty, which keeps the times at which one
 * of its joints lies in the box, and always lies in it when its restriction to
 * the complement of the box is empty; a box with only the T dimension
 * restricts the value to its period
 * @param[in] temp Temporal pose chain
 * @param[in] box Spatiotemporal box
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tposechain_stbox(const Temporal *temp, const STBox *box,
  bool ever)
{
  /* Ensure the validity of the arguments: a box with the X dimension shares
   * the SRID of the value, as #tposechain_restrict_stbox reads it */
  VALIDATE_TPOSECHAIN(temp, -1); VALIDATE_NOT_NULL(box, -1);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  if (hasx && ! ensure_valid_tposechain_stbox(temp, box))
    return -1;

  bool atfunc = ever ? REST_AT : REST_MINUS;
  Temporal *res = hasx ? tposechain_restrict_stbox(temp, box, true, atfunc) :
    temporal_restrict_tstzspan(temp, &box->period, atfunc);
  int result = ever ? (res != NULL) : (res == NULL);
  if (res)
    pfree(res);
  return result;
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return 1 if a temporal pose chain ever lies in a spatiotemporal
 * box, one of its joints lying in it, 0 if not, and -1 on error
 * @param[in] temp Temporal pose chain
 * @param[in] box Spatiotemporal box
 * @errval -1
 * @csqlfn #Ever_overlaps_tposechain_stbox()
 */
int
ever_overlaps_tposechain_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tposechain_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return 1 if a temporal pose chain ever lies in a spatiotemporal
 * box, one of its joints lying in it, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal pose chain
 * @errval -1
 * @csqlfn #Ever_overlaps_stbox_tposechain()
 */
int
ever_overlaps_stbox_tposechain(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tposechain_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return 1 if a temporal pose chain always lies in a spatiotemporal
 * box, one of its joints lying in it at every time, 0 if not, and -1 on error
 * @param[in] temp Temporal pose chain
 * @param[in] box Spatiotemporal box
 * @errval -1
 * @csqlfn #Always_overlaps_tposechain_stbox()
 */
int
always_overlaps_tposechain_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tposechain_stbox(temp, box, ALWAYS);
}

/**
 * @ingroup meos_posechain_comp_ever
 * @brief Return 1 if a temporal pose chain always lies in a spatiotemporal
 * box, one of its joints lying in it at every time, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal pose chain
 * @errval -1
 * @csqlfn #Always_overlaps_stbox_tposechain()
 */
int
always_overlaps_stbox_tposechain(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tposechain_stbox(temp, box, ALWAYS);
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
