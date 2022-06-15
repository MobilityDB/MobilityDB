/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file temporal_compops_meos.c
 * @brief Temporal comparison operators: #=, #<>, #<, #>, #<=, #>=.
 */

#include "general/temporal_compops.h"

/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a base value and a temporal value.
 * @sqlop @p #=
 */
Temporal *
teq_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_eq2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal value and a base value.
 * @sqlop @p #=
 */
Temporal *
teq_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_eq2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of the temporal values.
 * @sqlop @p #=
 */
Temporal *
teq_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_eq2);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a base value and a temporal value.
 * @sqlop @p #<>
 */
Temporal *
tne_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_ne2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal value and a base value.
 * @sqlop @p #<>
 */
Temporal *
tne_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_ne2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of the temporal values.
 * @sqlop @p #<>
 */
Temporal *
tne_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ne2);
}

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a base value and a temporal value.
 * @sqlop @p #<
 */
Temporal *
tlt_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_lt2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a temporal value and a base value.
 * @sqlop @p #<
 */
Temporal *
tlt_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_lt2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of the temporal values.
 * @sqlop @p #<
 */
Temporal *
tlt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_lt2);
}

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal of a base value and a
 * temporal value.
 * @sqlop @p #<=
 */
Temporal *
tle_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_le2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal of a temporal value and a
 * base value.
 * @sqlop @p #<=
 */
Temporal *
tle_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_le2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal of the temporal values.
 * @sqlop @p #<=
 */
Temporal *
tle_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_le2);
}

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a base value and a temporal value.
 * @sqlop @p #>
 */
Temporal *
tgt_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_gt2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal value and a base value.
 * @sqlop @p #>
 */
Temporal *
tgt_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_gt2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of the temporal values.
 * @sqlop @p #>
 */
Temporal *
tgt_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_gt2);
}

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal of a base value and a
 * temporal value.
 * @sqlop @p #>=
 */
Temporal *
tge_base_temporal(Datum base, MDB_Type basetype, const Temporal *temp)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_ge2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal of a temporal value and a
 * base value.
 * @sqlop @p #>=
 */
Temporal *
tge_temporal_base(const Temporal *temp, Datum base, MDB_Type basetype)
{
  return tcomp_temporal_base(temp, base, basetype, &datum2_ge2, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal of the temporal values.
 * @sqlop @p #>=
 */
Temporal *
tge_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return tcomp_temporal_temporal(temp1, temp2, &datum2_ge2);
}

/*****************************************************************************/
