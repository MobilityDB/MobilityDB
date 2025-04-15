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
 * @brief Ever/always and temporal comparisons for temporal circular buffers
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/pg_types.h"
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/temporal_compops.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"
// #include "cbuffer/tcbuffer_spatialfuncs.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer and a circular buffer
 * satisfy the ever/always
 * comparison
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
int
eacomp_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cbuf))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(cbuf), func, ever);
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
  Datum (*func)(Datum, Datum, meosType), bool ever)
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
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_cbuffer_tcbuffer()
 */
inline int
ever_eq_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever equal to a circular
 * buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Ever_eq_tcbuffer_cbuffer()
 */
inline int
ever_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_eq, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is ever different from a temporal
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_cbuffer_tcbuffer()
 */
inline int
ever_ne_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is ever different from a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Ever_ne_tcbuffer_cbuffer()
 */
inline int
ever_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_ne, EVER);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is always equal to a temporal
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_cbuffer_tcbuffer()
 */
inline int
always_eq_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always equal to a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Always_eq_tcbuffer_cbuffer()
 */
inline int
always_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a circular buffer is always different from a temporal
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_cbuffer_tcbuffer()
 */
inline int
always_ne_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if a temporal circular buffer is always different from a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Always_ne_tcbuffer_cbuffer()
 */
inline int
always_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return eacomp_tcbuffer_cbuffer(temp, cbuf, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_ever
 * @brief Return true if two temporal circular buffers are ever equal
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Ever_eq_tcbuffer_tcbuffer()
 */
inline int
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
inline int
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
inline int
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
inline int
always_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tcbuffer_tcbuffer(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a circular buffer and a temporal
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cbuf))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(cbuf), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cbuf))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(cbuf), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal equality of a circular buffer and a temporal
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Teq_cbuffer_tcbuffer()
 */
inline Temporal *
teq_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return tcomp_cbuffer_tcbuffer(cbuf, temp, &datum2_eq);
}

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal inequality of a circular buffer and a temporal
 * circular buffer
 * @param[in] cbuf Circular buffer
 * @param[in] temp Temporal value
 * @csqlfn #Tne_cbuffer_tcbuffer()
 */
inline Temporal *
tne_cbuffer_tcbuffer(const Cbuffer *cbuf, const Temporal *temp)
{
  return tcomp_cbuffer_tcbuffer(cbuf, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal equality of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Teq_tcbuffer_cbuffer()
 */
inline Temporal *
teq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return tcomp_tcbuffer_cbuffer(temp, cbuf, &datum2_eq);
}

/**
 * @ingroup meos_cbuffer_comp_temp
 * @brief Return the temporal inequality of a temporal circular buffer and a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cbuf Circular buffer
 * @csqlfn #Tne_tcbuffer_cbuffer()
 */
inline Temporal *
tne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cbuf)
{
  return tcomp_tcbuffer_cbuffer(temp, cbuf, &datum2_ne);
}

/*****************************************************************************/
