/*****************************************************************************
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
 * @brief Ever, always and temporal comparisons for the temporal S2 cell type.
 *
 * Equality of S2 cells is bit equality of the identifier, so every entry point
 * here is a thin type-correct dispatcher over the generic comparison machinery,
 * as the quadbin and h3index twins are. Each comparison stands in three forms:
 * the ever form, true when it holds at some instant; the always form, true when
 * it holds at every instant; and the temporal form, a tbool of the result at
 * each instant.
 */

#include "s2cell/ts2cell.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_s2cell.h>
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "s2cell/s2cell.h"

/*****************************************************************************
 * Internal dispatchers
 *****************************************************************************/

/**
 * @brief Return true if a temporal S2 cell and a bare S2 cell satisfy the
 * ever or always comparison
 * @param[in] temp Temporal S2 cell
 * @param[in] cell Bare S2 cell
 * @param[in] func Per-instant comparison primitive
 * @param[in] ever True for ever semantics, false for always
 */
static int
eacomp_ts2cell_s2cell(const Temporal *temp, S2CellId cell,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_ts2cell_s2cell(temp, cell))
    return -1;
  return eacomp_temporal_base(temp, S2CellGetDatum(cell), func, ever);
}

/**
 * @brief Return true if two temporal S2 cells satisfy the ever or always
 * comparison
 */
static int
eacomp_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_ts2cell_ts2cell(temp1, temp2))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************
 * Ever and always comparisons
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a temporal S2 cell is ever equal to a bare S2 cell
 * @csqlfn #Ever_eq_ts2cell_s2cell()
 */
int
ever_eq_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_eq, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a bare S2 cell is ever equal to a temporal S2 cell
 * @csqlfn #Ever_eq_s2cell_ts2cell()
 */
int
ever_eq_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_eq, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a temporal S2 cell is always equal to a bare S2 cell
 * @csqlfn #Always_eq_ts2cell_s2cell()
 */
int
always_eq_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_eq, false);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a bare S2 cell is always equal to a temporal S2 cell
 * @csqlfn #Always_eq_s2cell_ts2cell()
 */
int
always_eq_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_eq, false);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if two temporal S2 cells are ever equal to one another
 * @csqlfn #Ever_eq_ts2cell_ts2cell()
 */
int
ever_eq_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_ts2cell_ts2cell(temp1, temp2, &datum2_eq, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if two temporal S2 cells are always equal to one another
 * @csqlfn #Always_eq_ts2cell_ts2cell()
 */
int
always_eq_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_ts2cell_ts2cell(temp1, temp2, &datum2_eq, false);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a temporal S2 cell is ever different from a bare S2 cell
 * @csqlfn #Ever_ne_ts2cell_s2cell()
 */
int
ever_ne_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_ne, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a bare S2 cell is ever different from a temporal S2 cell
 * @csqlfn #Ever_ne_s2cell_ts2cell()
 */
int
ever_ne_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_ne, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a temporal S2 cell is always different from a bare S2 cell
 * @csqlfn #Always_ne_ts2cell_s2cell()
 */
int
always_ne_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_ne, false);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if a bare S2 cell is always different from a temporal S2 cell
 * @csqlfn #Always_ne_s2cell_ts2cell()
 */
int
always_ne_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return eacomp_ts2cell_s2cell(temp, cell, &datum2_ne, false);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if two temporal S2 cells are ever different from one another
 * @csqlfn #Ever_ne_ts2cell_ts2cell()
 */
int
ever_ne_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_ts2cell_ts2cell(temp1, temp2, &datum2_ne, true);
}

/**
 * @ingroup meos_s2cell_comp_ever
 * @brief Return true if two temporal S2 cells are always different from one another
 * @csqlfn #Always_ne_ts2cell_ts2cell()
 */
int
always_ne_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_ts2cell_ts2cell(temp1, temp2, &datum2_ne, false);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the per-instant comparison of a temporal S2 cell against a
 * bare S2 cell
 * @details Equality and inequality are commutative, so the two argument orders
 * share one implementation.
 */
static Temporal *
tcomp_ts2cell_s2cell(const Temporal *temp, S2CellId cell,
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  assert(func);
  if (! ensure_valid_ts2cell_s2cell(temp, cell))
    return NULL;
  return tcomp_temporal_base(temp, S2CellGetDatum(cell), func);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal equality of a temporal S2 cell and a bare S2 cell
 * @csqlfn #Teq_ts2cell_s2cell()
 */
Temporal *
teq_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return tcomp_ts2cell_s2cell(temp, cell, &datum2_eq);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal equality of a bare S2 cell and a temporal S2 cell
 * @csqlfn #Teq_s2cell_ts2cell()
 */
Temporal *
teq_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return tcomp_ts2cell_s2cell(temp, cell, &datum2_eq);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal equality of two temporal S2 cells across the time
 * they share
 * @csqlfn #Teq_ts2cell_ts2cell()
 */
Temporal *
teq_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_ts2cell_ts2cell(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal inequality of a temporal S2 cell and a bare S2 cell
 * @csqlfn #Tne_ts2cell_s2cell()
 */
Temporal *
tne_ts2cell_s2cell(const Temporal *temp, S2CellId cell)
{
  return tcomp_ts2cell_s2cell(temp, cell, &datum2_ne);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal inequality of a bare S2 cell and a temporal S2 cell
 * @csqlfn #Tne_s2cell_ts2cell()
 */
Temporal *
tne_s2cell_ts2cell(S2CellId cell, const Temporal *temp)
{
  return tcomp_ts2cell_s2cell(temp, cell, &datum2_ne);
}

/**
 * @ingroup meos_s2cell_comp_temp
 * @brief Return the temporal inequality of two temporal S2 cells across the time
 * they share
 * @csqlfn #Tne_ts2cell_ts2cell()
 */
Temporal *
tne_ts2cell_ts2cell(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_ts2cell_ts2cell(temp1, temp2))
    return NULL;
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne);
}

/*****************************************************************************/
