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
 * @brief Return the temporal equality of a boolean and a temporal boolean.
 * @sqlop @p #=
 */
Temporal *
teq_bool_tbool(bool b, const Temporal *temp)
{
  return tcomp_temporal_base(temp, BoolGetDatum(b), T_BOOL, &datum2_eq2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of an integer and a temporal integer.
 * @sqlop @p #=
 */
Temporal *
teq_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_eq2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a float and a temporal float.
 * @sqlop @p #=
 */
Temporal *
teq_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_eq2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a text and a temporal text.
 * @sqlop @p #=
 */
Temporal *
teq_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_eq2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a point and a temporal geometric
 * point.
 * @sqlop @p #=
 */
Temporal *
teq_point_tgeompoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOMETRY, &datum2_eq2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a point and a temporal geographic
 * point.
 * @sqlop @p #=
 */
Temporal *
teq_point_tgeogpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOGRAPHY,
    &datum2_eq2, INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal boolean and a boolean.
 * @sqlop @p #=
 */
Temporal *
teq_tbool_bool(const Temporal *temp, bool b)
{
  return tcomp_temporal_base(temp, BoolGetDatum(b), T_BOOL, &datum2_eq2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal integer and an integer.
 * @sqlop @p #=
 */
Temporal *
teq_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_eq2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal float and a float.
 * @sqlop @p #=
 */
Temporal *
teq_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_eq2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal text and a text.
 * @sqlop @p #=
 */
Temporal *
teq_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_eq2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal geometric point and a
 * point.
 * @sqlop @p #=
 */
Temporal *
teq_tgeompoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOMETRY, &datum2_eq2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal equality of a temporal geographic point and a
 * point.
 * @sqlop @p #=
 */
Temporal *
teq_tgeogpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOGRAPHY, &datum2_eq2,
    INVERT_NO);
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
 * @brief Return the temporal inequality of a boolean and a temporal boolean.
 * @sqlop @p #<>
 */
Temporal *
tne_bool_tbool(bool b, const Temporal *temp)
{
  return tcomp_temporal_base(temp, BoolGetDatum(b), T_BOOL, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of an integer and a temporal integer.
 * @sqlop @p #<>
 */
Temporal *
tne_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a float and a temporal float.
 * @sqlop @p #<>
 */
Temporal *
tne_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a text and a temporal text.
 * @sqlop @p #<>
 */
Temporal *
tne_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a point and a temporal geometric
 * point.
 * @sqlop @p #<>
 */
Temporal *
tne_point_tgeompoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOMETRY, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a point and a temporal geographic
 * point.
 * @sqlop @p #<>
 */
Temporal *
tne_point_tgeogpoint(const GSERIALIZED *gs, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOGRAPHY, &datum2_ne2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal boolean and a boolean.
 * @sqlop @p #<>
 */
Temporal *
tne_tbool_bool(const Temporal *temp, bool b)
{
  return tcomp_temporal_base(temp, BoolGetDatum(b), T_BOOL, &datum2_ne2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal integer and an integer.
 * @sqlop @p #<>
 */
Temporal *
tne_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_ne2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal float and a float.
 * @sqlop @p #<>
 */
Temporal *
tne_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_ne2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal text and a text.
 * @sqlop @p #<>
 */
Temporal *
tne_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_ne2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal geometric point and a
 * point.
 * @sqlop @p #<>
 */
Temporal *
tne_tgeompoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOMETRY, &datum2_ne2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal inequality of a temporal geographic point and a
 * point.
 * @sqlop @p #<>
 */
Temporal *
tne_tgeogpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  return tcomp_temporal_base(temp, PointerGetDatum(gs), T_GEOGRAPHY, &datum2_ne2,
    INVERT_NO);
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
 * @brief Return the temporal less than of an integer and a temporal integer.
 * @sqlop @p #<
 */
Temporal *
tlt_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_lt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a float and a temporal float.
 * @sqlop @p #<
 */
Temporal *
tlt_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_lt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a text and a temporal text.
 * @sqlop @p #<
 */
Temporal *
tlt_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_lt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a temporal integer and an integer.
 * @sqlop @p #<
 */
Temporal *
tlt_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_lt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a temporal float and a float.
 * @sqlop @p #<
 */
Temporal *
tlt_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_lt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than of a temporal text and a text.
 * @sqlop @p #<
 */
Temporal *
tlt_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_lt2,
    INVERT_NO);
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
 * @brief Return the temporal less than or equal to of an integer and a
 * temporal integer.
 * @sqlop @p #<=
 */
Temporal *
tle_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_le2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal to of a float and a temporal
 * float.
 * @sqlop @p #<=
 */
Temporal *
tle_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_le2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less or equal to than of a text and a temporal
 * text.
 * @sqlop @p #<=
 */
Temporal *
tle_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_le2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal to of a temporal integer and
 * an integer.
 * @sqlop @p #<=
 */
Temporal *
tle_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_le2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal to of a temporal float and a
 * float.
 * @sqlop @p #<=
 */
Temporal *
tle_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_le2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal to of a temporal text and a
 * text.
 * @sqlop @p #<=
 */
Temporal *
tle_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_le2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal less than or equal to of the temporal values.
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
 * @brief Return the temporal greater than of an integer and a temporal integer.
 * @sqlop @p #>
 */
Temporal *
tgt_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_gt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a float and a temporal float.
 * @sqlop @p #>
 */
Temporal *
tgt_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_gt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a text and a temporal text.
 * @sqlop @p #>
 */
Temporal *
tgt_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_gt2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal integer and an integer.
 * @sqlop @p #>
 */
Temporal *
tgt_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_gt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal float and a float.
 * @sqlop @p #>
 */
Temporal *
tgt_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_gt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal text and a text.
 * @sqlop @p #>
 */
Temporal *
tgt_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_gt2,
    INVERT_NO);
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
 * @brief Return the temporal greater than or equal to of an integer and a
 * temporal integer.
 * @sqlop @p #>=
 */
Temporal *
tge_int_tint(int i, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_ge2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal to of a float and a
 * temporal float.
 * @sqlop @p #>=
 */
Temporal *
tge_float_tfloat(double d, const Temporal *temp)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_ge2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal to of a text and a temporal
 * text.
 * @sqlop @p #>=
 */
Temporal *
tge_text_ttext(const text *txt, const Temporal *temp)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_ge2,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal to of a temporal integer
 * and an integer.
 * @sqlop @p #>=
 */
Temporal *
tge_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_ge2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal to of a temporal float and
 * a float.
 * @sqlop @p #>=
 */
Temporal *
tge_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_ge2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than or equal to of a temporal text and a
 * text.
 * @sqlop @p #>=
 */
Temporal *
tge_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_ge2,
    INVERT_NO);
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
