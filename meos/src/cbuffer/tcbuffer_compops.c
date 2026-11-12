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
 * @brief Ever/always and temporal comparisons for temporal circular buffers
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer and a circular buffer
 * satisfy the ever/always comparison
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(cb), func, ever);
}

/**
 * @brief Return true if two temporal circular buffers satisfy the ever/always
 * comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is ever equal to a temporal circular
 * buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_cbuffer_tcbuffer()
 */
int
ever_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever equal to a circular
 * buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Ever_eq_tcbuffer_cbuffer()
 */
int
ever_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is ever different from a temporal
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_cbuffer_tcbuffer()
 */
int
ever_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever different from a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Ever_ne_tcbuffer_cbuffer()
 */
int
ever_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is always equal to a temporal
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_cbuffer_tcbuffer()
 */
int
always_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always equal to a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Always_eq_tcbuffer_cbuffer()
 */
int
always_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is always different from a temporal
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_cbuffer_tcbuffer()
 */
int
always_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always different from a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Always_ne_tcbuffer_cbuffer()
 */
int
always_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return eacomp_tcbuffer_cbuffer(temp, cb, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are ever equal
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Ever_eq_tcbuffer_tcbuffer()
 */
int
ever_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tcbuffer_tcbuffer(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are ever different
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Ever_ne_tcbuffer_tcbuffer()
 */
int
ever_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tcbuffer_tcbuffer(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are always equal
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Always_eq_tcbuffer_tcbuffer()
 */
int
always_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tcbuffer_tcbuffer(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are always different
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Always_ne_tcbuffer_tcbuffer()
 */
int
always_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tcbuffer_tcbuffer(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Ever/always overlaps of a temporal circular buffer and a spatiotemporal box
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal circular buffer ever (or always) lies in a
 * spatiotemporal box, 0 if not, and -1 on error, as #eaoverlaps_tgeo_stbox
 * answers it for a temporal geo
 * @details The value ever lies in the box when its restriction to the box by
 * #tcbuffer_restrict_stbox is not empty, and always lies in it when its
 * restriction to the complement of the box is empty; a box with only the T
 * dimension restricts the value to its period
 * @param[in] temp Temporal circular buffer
 * @param[in] box Spatiotemporal box
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tcbuffer_stbox(const Temporal *temp, const STBox *box, bool ever)
{
  /* Ensure the validity of the arguments: a box with the X dimension shares
   * the SRID of the value, as #tcbuffer_restrict_stbox reads it */
  VALIDATE_TCBUFFER(temp, -1); VALIDATE_NOT_NULL(box, -1);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  if (hasx && ! ensure_valid_tcbuffer_stbox(temp, box))
    return -1;

  bool atfunc = ever ? REST_AT : REST_MINUS;
  Temporal *res = hasx ? tcbuffer_restrict_stbox(temp, box, true, atfunc) :
    temporal_restrict_tstzspan(temp, &box->period, atfunc);
  int result = ever ? (res != NULL) : (res == NULL);
  if (res)
    pfree(res);
  return result;
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return 1 if a temporal circular buffer ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] box Spatiotemporal box
 * @csqlfn #Ever_overlaps_tcbuffer_stbox()
 */
int
ever_overlaps_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tcbuffer_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return 1 if a temporal circular buffer ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Ever_overlaps_stbox_tcbuffer()
 */
int
ever_overlaps_stbox_tcbuffer(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tcbuffer_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return 1 if a temporal circular buffer always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal circular buffer
 * @param[in] box Spatiotemporal box
 * @csqlfn #Always_overlaps_tcbuffer_stbox()
 */
int
always_overlaps_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tcbuffer_stbox(temp, box, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return 1 if a temporal circular buffer always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal circular buffer
 * @csqlfn #Always_overlaps_stbox_tcbuffer()
 */
int
always_overlaps_stbox_tcbuffer(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tcbuffer_stbox(temp, box, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a circular buffer and a temporal
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(cb), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(cb), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal equality of a circular buffer and a temporal
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Teq_cbuffer_tcbuffer()
 */
Temporal *
teq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tcomp_cbuffer_tcbuffer(cb, temp, &datum2_eq);
}

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal inequality of a circular buffer and a temporal
 * circular buffer
 * @param[in] cb Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Tne_cbuffer_tcbuffer()
 */
Temporal *
tne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp)
{
  return tcomp_cbuffer_tcbuffer(cb, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal equality of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Teq_tcbuffer_cbuffer()
 */
Temporal *
teq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcomp_tcbuffer_cbuffer(temp, cb, &datum2_eq);
}

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal inequality of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Circular buffer
 * @csqlfn #Tne_tcbuffer_cbuffer()
 */
Temporal *
tne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcomp_tcbuffer_cbuffer(temp, cb, &datum2_ne);
}

/*****************************************************************************/
