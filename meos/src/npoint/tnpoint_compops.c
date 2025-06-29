/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Ever/always and temporal comparisons for temporal network points
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "temporal/temporal_compops.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal network point and a network point satisfy
 * the ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] np Network point
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, -1); VALIDATE_NOT_NULL(np, -1);
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(np), func, ever);
}

/**
 * @brief Return true if two temporal network points satisfy the ever/always comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, -1); VALIDATE_TNPOINT(temp2, -1);
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is ever equal to a temporal network
 * point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Ever_eq_npoint_tnpoint()
 */
inline int
ever_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, EVER);
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is ever equal to a network
 * point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Ever_eq_tnpoint_npoint()
 */
inline int
ever_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, EVER);
}

#if MEOS
/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is ever different from a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Ever_ne_npoint_tnpoint()
 */
inline int
ever_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, EVER);
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is ever different from a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Ever_ne_tnpoint_npoint()
 */
inline int
ever_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, EVER);
}

#if MEOS
/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is always equal to a temporal network
 * point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Always_eq_npoint_tnpoint()
 */
inline int
always_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is always equal to a network
 * point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Always_eq_tnpoint_npoint()
 */
inline int
always_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_eq, ALWAYS);
}

#if MEOS
/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a network point is always different from a temporal
 * network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Always_ne_npoint_tnpoint()
 */
inline int
always_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, ALWAYS);
}
#endif /* MEOS */

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if a temporal network point is always different from a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Always_ne_tnpoint_npoint()
 */
inline int
always_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return eacomp_tnpoint_npoint(temp, np, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are ever equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_eq_tnpoint_tnpoint()
 */
inline int
ever_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are ever different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Ever_ne_tnpoint_tnpoint()
 */
inline int
ever_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are always equal
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_eq_tnpoint_tnpoint()
 */
inline int
always_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_npoint_comp_ever
 * @brief Return true if two temporal network points are always different
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Always_ne_tnpoint_tnpoint()
 */
inline int
always_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tnpoint_tnpoint(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a temporal network point and a
 * network point
 * @param[in] temp Temporal value
 * @param[in] np Network point
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, NULL); VALIDATE_NOT_NULL(np, NULL);
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(np), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_npoint_comp_temp
 * @brief Return the temporal equality of a temporal network point and a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Teq_tnpoint_npoint()
 */
inline Temporal *
teq_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return tcomp_tnpoint_npoint(temp, np, &datum2_eq);
}

/**
 * @ingroup meos_npoint_comp_temp
 * @brief Return the temporal inequality of a temporal network point and a
 * network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Tne_tnpoint_npoint()
 */
inline Temporal *
tne_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  return tcomp_tnpoint_npoint(temp, np, &datum2_ne);
}

/*****************************************************************************/
