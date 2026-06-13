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
 * @brief Ever, always and temporal comparisons for temporal JSONB
 */

/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include "utils/jsonb.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "json/tjsonb.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a JSONB is ever equal to a temporal JSONB
 * @param[in] jb JSONB value
 * @param[in] temp Temporal JSONB
 * @csqlfn #Ever_eq_base_temporal()
 */
int
ever_eq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return ever_eq_base_temporal(PointerGetDatum(jb), temp);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a temporal JSONB is ever equal to a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB value
 * @csqlfn #Ever_eq_temporal_base()
 */
int
ever_eq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return ever_eq_temporal_base(temp, PointerGetDatum(jb));
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a JSONB is always equal to a temporal JSONB
 * @param[in] jb JSONB
 * @param[in] temp Temporal JSONB
 * @csqlfn #Always_eq_base_temporal()
 */
int
always_eq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return always_eq_base_temporal(PointerGetDatum(jb), temp);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a temporal JSONB is always equal to a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB
 * @csqlfn #Always_eq_temporal_base()
 */
int
always_eq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return always_eq_temporal_base(temp, PointerGetDatum(jb));
}

/*****************************************************************************/

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if two temporal JSONB are ever equal
 * @param[in] temp1,temp2 Temporal JSONB
 * @csqlfn #Ever_eq_tjsonb_tjsonb()
 */
inline int
ever_eq_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_temporal_temporal(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if two temporal JSONB are ever different
 * @param[in] temp1,temp2 Temporal JSONB
 * @csqlfn #Ever_ne_tjsonb_tjsonb()
 */
inline int
ever_ne_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if two temporal JSONB are always equal
 * @param[in] temp1,temp2 Temporal JSONB
 * @csqlfn #Always_eq_tjsonb_tjsonb()
 */
inline int
always_eq_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_temporal_temporal(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if two temporal JSONB are always different
 * @param[in] temp1,temp2 Temporal JSONB
 * @csqlfn #Always_ne_tjsonb_tjsonb()
 */
inline int
always_ne_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_temporal_temporal(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a JSONB is ever different from a temporal JSONB
 * @param[in] jb JSONB
 * @param[in] temp Temporal JSONB
 * @csqlfn #Ever_ne_base_temporal()
 */
int
ever_ne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return ever_ne_base_temporal(PointerGetDatum(jb), temp);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a temporal JSONB is ever different from a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB
 * @csqlfn #Ever_ne_temporal_base()
 */
int
ever_ne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return ever_ne_temporal_base(temp, PointerGetDatum(jb));
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a JSONB is always different from a temporal JSONB
 * @param[in] jb JSONB
 * @param[in] temp Temporal JSONB
 * @csqlfn #Always_ne_base_temporal()
 */
int
always_ne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return always_ne_base_temporal(PointerGetDatum(jb), temp);
}

/**
 * @ingroup meos_json_comp_ever
 * @brief Return true if a temporal JSONB is always different from a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB
 * @csqlfn #Always_ne_temporal_base()
 */
int
always_ne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, -1); VALIDATE_NOT_NULL(jb, -1);
  return always_ne_temporal_base(temp, PointerGetDatum(jb));
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @ingroup meos_json_comp_temp
 * @brief Return the temporal equality of a JSONB and a temporal JSONB
 * @param[in] jb JSONB
 * @param[in] temp Temporal JSONB
 * @csqlfn #Teq_base_temporal()
 */
Temporal *
teq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return tcomp_base_temporal(PointerGetDatum(jb), temp, &datum2_eq);
}

/**
 * @ingroup meos_json_comp_temp
 * @brief Return the temporal equality of a temporal JSONB and a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB
 * @csqlfn #Teq_temporal_base()
 */
Temporal *
teq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(jb), &datum2_eq);
}

/**
 * @ingroup meos_json_comp_temp
 * @brief Return the temporal difference of a JSONB and a temporal JSONB
 * @param[in] jb JSONB
 * @param[in] temp Temporal JSONB
 * @csqlfn #Tne_base_temporal()
 */
Temporal *
tne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return tcomp_base_temporal(PointerGetDatum(jb), temp, &datum2_ne);
}

/**
 * @ingroup meos_json_comp_temp
 * @brief Return the temporal difference of a temporal JSONB and a JSONB
 * @param[in] temp Temporal JSONB
 * @param[in] jb JSONB
 * @csqlfn #Tne_temporal_base()
 */
Temporal *
tne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TJSONB(temp, NULL); VALIDATE_NOT_NULL(jb, NULL);
  return tcomp_temporal_base(temp, PointerGetDatum(jb), &datum2_ne);
}

/*****************************************************************************/
