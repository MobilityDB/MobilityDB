/*****************************************************************************
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
 * @brief Comparison operators for the th3index type.
 *
 * Analogue of `meos/src/cbuffer/tcbuffer_compops.c`. Every ever /
 * always / temporal comparison flavour for th3index is mirrored
 * from the cbuffer pattern, with the bare-cell operand carried as
 * `H3Index` (uint64).
 *
 * For h3 cells, equality / inequality is exactly int64 bit-equality —
 * the `datum2_eq` / `datum2_ne` primitives used by the tbigint
 * machinery already have the right semantics, so the per-instant
 * comparison primitives here are shallow reuses. This matches
 * h3-pg's own `=` / `<>` operator implementation (bit-wise on the
 * underlying uint64 payload).
 *
 * Ordering operators (`<`, `<=`, `>`, `>=`) are NOT lifted — h3
 * cells have no canonical ordering beyond the int64 bit
 * representation, and the h3-pg API deliberately exposes only
 * `<->` (distance) and `=` / `<>`. Users who want raw int64 order
 * can cast to tbigint.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_h3.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
/* th3index */
#include "h3/th3index.h"
#include "h3/h3index.h"

/*****************************************************************************
 * Internal dispatchers
 *****************************************************************************/

/**
 * @brief Return `true` if a temporal H3 cell and a bare H3 cell satisfy
 * the ever/always comparison.
 * @param[in] temp Temporal H3 cell
 * @param[in] cell Bare H3 cell
 * @param[in] func Per-instant comparison primitive
 * @param[in] ever True for ever semantics, false for always
 */
static int
eacomp_th3index_h3index(const Temporal *temp, H3Index cell,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  if (! ensure_valid_th3index_h3index(temp, cell))
    return -1;
  return eacomp_temporal_base(temp, H3IndexGetDatum(cell), func, ever);
}

/**
 * @brief Return `true` if two temporal H3 cells satisfy the ever/always
 * comparison.
 */
static int
eacomp_th3index_th3index(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  if (! ensure_valid_th3index_th3index(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************
 * Ever/always equality
 *****************************************************************************/

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a temporal H3 cell is ever equal to a bare H3 cell.
 */
inline int
ever_eq_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_eq, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a bare H3 cell is ever equal to a temporal H3 cell.
 */
inline int
ever_eq_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_eq, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a temporal H3 cell is ever not equal to a bare
 * H3 cell.
 */
inline int
ever_ne_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_ne, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a bare H3 cell is ever not equal to a temporal
 * H3 cell.
 */
inline int
ever_ne_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_ne, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a temporal H3 cell is always equal to a bare
 * H3 cell.
 */
inline int
always_eq_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a bare H3 cell is always equal to a temporal
 * H3 cell.
 */
inline int
always_eq_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a temporal H3 cell is always not equal to a bare
 * H3 cell.
 */
inline int
always_ne_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if a bare H3 cell is always not equal to a temporal
 * H3 cell.
 */
inline int
always_ne_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return eacomp_th3index_h3index(temp, cell, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Two-temporal ever/always
 *****************************************************************************/

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if two temporal H3 cells are ever equal at some
 * shared instant.
 */
inline int
ever_eq_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_th3index_th3index(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if two temporal H3 cells are ever unequal at some
 * shared instant.
 */
inline int
ever_ne_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_th3index_th3index(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if two temporal H3 cells are always equal across
 * their shared time axis.
 */
inline int
always_eq_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_th3index_th3index(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_h3_comp_ever
 * @brief Return `true` if two temporal H3 cells are always unequal across
 * their shared time axis.
 */
inline int
always_ne_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_th3index_th3index(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal equality / inequality — return a tbool trajectory of per-instant
 * truth values.
 *****************************************************************************/

/**
 * @brief Internal dispatcher for per-instant temporal comparisons against a
 * bare H3 cell. Equality / inequality are commutative, so the (cell, temp)
 * and (temp, cell) entry points share a single implementation.
 */
static Temporal *
tcomp_th3index_h3index(const Temporal *temp, H3Index cell,
  Datum (*func)(Datum, Datum, MeosType))
{
  if (! ensure_valid_th3index_h3index(temp, cell))
    return NULL;
  return tcomp_temporal_base(temp, H3IndexGetDatum(cell), func);
}

static Temporal *
tcomp_h3index_th3index(H3Index cell, const Temporal *temp,
  Datum (*func)(Datum, Datum, MeosType))
{
  if (! ensure_valid_th3index_h3index(temp, cell))
    return NULL;
  return tcomp_temporal_base(temp, H3IndexGetDatum(cell), func);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal equality between a temporal H3 cell and a
 * bare H3 cell.
 */
inline Temporal *
teq_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return tcomp_th3index_h3index(temp, cell, &datum2_eq);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal equality between a bare H3 cell and a
 * temporal H3 cell.
 */
inline Temporal *
teq_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return tcomp_h3index_th3index(cell, temp, &datum2_eq);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal inequality between a temporal H3 cell and a
 * bare H3 cell.
 */
inline Temporal *
tne_th3index_h3index(const Temporal *temp, H3Index cell)
{
  return tcomp_th3index_h3index(temp, cell, &datum2_ne);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal inequality between a bare H3 cell and a
 * temporal H3 cell.
 */
inline Temporal *
tne_h3index_th3index(H3Index cell, const Temporal *temp)
{
  return tcomp_h3index_th3index(cell, temp, &datum2_ne);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal equality of two temporal H3 cells across
 * their shared time axis.
 */
inline Temporal *
teq_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_th3index_th3index(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq);
}

/**
 * @ingroup meos_h3_comp_temp
 * @brief Return the temporal inequality of two temporal H3 cells across
 * their shared time axis.
 */
inline Temporal *
tne_th3index_th3index(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_th3index_th3index(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
}

/*****************************************************************************/
