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
 * @brief Set types over pgpointcloud pcpoint / pcpatch base values.
 *
 * Mirrors the structure of @p geoset_meos.c and @p cbufferset_meos.c.
 * Both set types enforce @c pcid uniformity across their elements —
 * values with different schemas cannot coexist because their byte
 * layouts are incomparable at the MEOS layer (dimension-level access
 * needs the schema XML from @c pointcloud_formats, unavailable here).
 * The constructor-path check lives in @p set_make_exp in
 * @p meos/src/temporal/set.c; the set-operation path uses
 * @p ensure_valid_pcpointset_pcpoint / @p ensure_valid_pcpatchset_pcpatch
 * implemented in @p pcpoint.c / @p pcpatch.c.
 *
 * Phase 8E has no bounding box — @p pointcloudset_type(settype) makes
 * @p set_bbox_size return 0. TPCBox lands in Phase 8F.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/set.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "pointcloud/pcpoint.h"
#include "pointcloud/pcpatch.h"

/*****************************************************************************
 * Validity helpers
 *
 * Declared in pcpoint.h / pcpatch.h; defined here rather than next to
 * @p ensure_same_pcid_* in pcpoint.c / pcpatch.c because @p SET_VAL_N
 * lives in @p meos_internal.h, whose @p <json-c/json.h> include collides
 * with PG's @p utils/builtins.h (both define a symbol @p json_object).
 * The base-type files need @p builtins.h for hex_encode/hex_decode, so
 * the set-side validators live here where json-c can coexist with the
 * rest of the MEOS internal API.
 *****************************************************************************/

/**
 * @brief Ensure that a pcpoint and a pcpoint set share the same schema (pcid)
 * @note Empty sets trivially pass — the incoming value's pcid becomes the
 * set's pcid on first insert.
 */
bool
ensure_valid_pcpointset_pcpoint(const Set *s, const Pcpoint *pt)
{
  VALIDATE_PCPOINTSET(s, false); VALIDATE_NOT_NULL(pt, false);
  if (s->count == 0)
    return true;
  const Pcpoint *first =
    (const Pcpoint *) DatumGetPointer(SET_VAL_N(s, 0));
  return ensure_same_pcid_pcpoint(first, pt);
}

/**
 * @brief Ensure that a pcpatch and a pcpatch set share the same schema (pcid)
 */
bool
ensure_valid_pcpatchset_pcpatch(const Set *s, const Pcpatch *pa)
{
  VALIDATE_PCPATCHSET(s, false); VALIDATE_NOT_NULL(pa, false);
  if (s->count == 0)
    return true;
  const Pcpatch *first =
    (const Pcpatch *) DatumGetPointer(SET_VAL_N(s, 0));
  return ensure_same_pcid_pcpatch(first, pa);
}

/*****************************************************************************
 * Input / output — pcpoint sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_inout
 * @brief Return a pcpoint set from its textual (hex-WKB) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
pcpointset_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_PCPOINTSET);
}

/**
 * @ingroup meos_pointcloud_set_inout
 * @brief Return the textual (hex-WKB) representation of a pcpoint set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits (ignored — pcpoint
 *   serializes as opaque hex at the MEOS layer)
 * @csqlfn #Set_out()
 */
char *
pcpointset_out(const Set *s, int maxdd)
{
  VALIDATE_PCPOINTSET(s, NULL);
  return set_out(s, maxdd);
}

/*****************************************************************************
 * Constructor — pcpoint sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_constructor
 * @brief Return a pcpoint set from an array of pcpoint values
 * @param[in] values Array of values (all must share the same pcid)
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
pcpointset_make(Pcpoint **values, int count)
{
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  /* set_make_free → set_make_exp enforces same-pcid across the array */
  return set_make_free(datums, count, T_PCPOINT, ORDER);
}

/*****************************************************************************
 * Conversion — pcpoint sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_conversion
 * @brief Convert a pcpoint into a singleton pcpoint set
 * @param[in] pt Value
 * @csqlfn #Value_to_set()
 */
Set *
pcpoint_to_set(const Pcpoint *pt)
{
  VALIDATE_NOT_NULL(pt, NULL);
  return value_set(PointerGetDatum(pt), T_PCPOINT);
}

/*****************************************************************************
 * Accessors — pcpoint sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_accessor
 * @brief Return a copy of the start (smallest) value of a pcpoint set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
Pcpoint *
pcpointset_start_value(const Set *s)
{
  VALIDATE_PCPOINTSET(s, NULL);
  return (Pcpoint *) DatumGetPointer(datum_copy(SET_VAL_N(s, 0),
    s->basetype));
}

/**
 * @ingroup meos_pointcloud_set_accessor
 * @brief Return a copy of the end (largest) value of a pcpoint set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
Pcpoint *
pcpointset_end_value(const Set *s)
{
  VALIDATE_PCPOINTSET(s, NULL);
  return (Pcpoint *) DatumGetPointer(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}

/**
 * @ingroup meos_pointcloud_set_accessor
 * @brief Return in the last argument a copy of the n-th value of a pcpoint set
 * @param[in] s Set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return @p true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
pcpointset_value_n(const Set *s, int n, Pcpoint **result)
{
  VALIDATE_PCPOINTSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = (Pcpoint *) DatumGetPointer(datum_copy(SET_VAL_N(s, n - 1),
    s->basetype));
  return true;
}

/**
 * @ingroup meos_pointcloud_set_accessor
 * @brief Return an array of copies of the values of a pcpoint set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
Pcpoint **
pcpointset_values(const Set *s)
{
  VALIDATE_PCPOINTSET(s, NULL);
  Pcpoint **result = palloc(sizeof(Pcpoint *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = (Pcpoint *) DatumGetPointer(datum_copy(SET_VAL_N(s, i),
      s->basetype));
  return result;
}

/*****************************************************************************
 * Set operations — pcpoint / pcpoint-set
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return @p true if a set contains a pcpoint
 * @csqlfn #Contains_set_value()
 */
bool
contains_set_pcpoint(const Set *s, Pcpoint *pt)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return false;
  return contains_set_value(s, PointerGetDatum(pt));
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return @p true if a pcpoint is contained in a set
 * @csqlfn #Contained_value_set()
 */
bool
contained_pcpoint_set(const Pcpoint *pt, const Set *s)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return false;
  return contained_value_set(PointerGetDatum(pt), s);
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the union of a set and a pcpoint
 * @csqlfn #Union_set_value()
 */
Set *
union_set_pcpoint(const Set *s, const Pcpoint *pt)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return NULL;
  return union_set_value(s, PointerGetDatum(pt));
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the union of a pcpoint and a set
 * @csqlfn #Union_set_value()
 */
Set *
union_pcpoint_set(const Pcpoint *pt, const Set *s)
{
  return union_set_pcpoint(s, pt);
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the intersection of a set and a pcpoint
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_set_pcpoint(const Set *s, const Pcpoint *pt)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(pt));
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the intersection of a pcpoint and a set
 * @csqlfn #Intersection_set_value()
 */
Set *
intersection_pcpoint_set(const Pcpoint *pt, const Set *s)
{
  return intersection_set_pcpoint(s, pt);
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the difference of a pcpoint and a set
 * @csqlfn #Minus_value_set()
 */
Set *
minus_pcpoint_set(const Pcpoint *pt, const Set *s)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return NULL;
  return minus_value_set(PointerGetDatum(pt), s);
}

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Return the difference of a set and a pcpoint
 * @csqlfn #Minus_set_value()
 */
Set *
minus_set_pcpoint(const Set *s, const Pcpoint *pt)
{
  if (! ensure_valid_pcpointset_pcpoint(s, pt))
    return NULL;
  return minus_set_value(s, PointerGetDatum(pt));
}

/*****************************************************************************
 * Aggregate — pcpoint union
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_setops
 * @brief Transition function for set union aggregate of pcpoint values
 * @param[in,out] state Current aggregate state
 * @param[in] pt Value
 */
Set *
pcpoint_union_transfn(Set *state, const Pcpoint *pt)
{
  VALIDATE_NOT_NULL(pt, NULL);
  if (state && ! ensure_set_isof_type(state, T_PCPOINTSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(pt), T_PCPOINT);
}

/*****************************************************************************
 * Input / output — pcpatch sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_inout
 * @brief Return a pcpatch set from its textual (hex-WKB) representation
 * @csqlfn #Set_in()
 */
Set *
pcpatchset_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_PCPATCHSET);
}

/**
 * @ingroup meos_pointcloud_set_inout
 * @brief Return the textual (hex-WKB) representation of a pcpatch set
 * @csqlfn #Set_out()
 */
char *
pcpatchset_out(const Set *s, int maxdd)
{
  VALIDATE_PCPATCHSET(s, NULL);
  return set_out(s, maxdd);
}

/*****************************************************************************
 * Constructor — pcpatch sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_constructor
 * @brief Return a pcpatch set from an array of pcpatch values
 * @csqlfn #Set_constructor()
 */
Set *
pcpatchset_make(Pcpatch **values, int count)
{
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_PCPATCH, ORDER);
}

/*****************************************************************************
 * Conversion — pcpatch sets
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_set_conversion
 * @brief Convert a pcpatch into a singleton pcpatch set
 * @csqlfn #Value_to_set()
 */
Set *
pcpatch_to_set(const Pcpatch *pa)
{
  VALIDATE_NOT_NULL(pa, NULL);
  return value_set(PointerGetDatum(pa), T_PCPATCH);
}

/*****************************************************************************
 * Accessors — pcpatch sets
 *****************************************************************************/

Pcpatch *
pcpatchset_start_value(const Set *s)
{
  VALIDATE_PCPATCHSET(s, NULL);
  return (Pcpatch *) DatumGetPointer(datum_copy(SET_VAL_N(s, 0),
    s->basetype));
}

Pcpatch *
pcpatchset_end_value(const Set *s)
{
  VALIDATE_PCPATCHSET(s, NULL);
  return (Pcpatch *) DatumGetPointer(datum_copy(SET_VAL_N(s, s->count - 1),
    s->basetype));
}

bool
pcpatchset_value_n(const Set *s, int n, Pcpatch **result)
{
  VALIDATE_PCPATCHSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = (Pcpatch *) DatumGetPointer(datum_copy(SET_VAL_N(s, n - 1),
    s->basetype));
  return true;
}

Pcpatch **
pcpatchset_values(const Set *s)
{
  VALIDATE_PCPATCHSET(s, NULL);
  Pcpatch **result = palloc(sizeof(Pcpatch *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = (Pcpatch *) DatumGetPointer(datum_copy(SET_VAL_N(s, i),
      s->basetype));
  return result;
}

/*****************************************************************************
 * Set operations — pcpatch / pcpatch-set
 *****************************************************************************/

bool
contains_set_pcpatch(const Set *s, Pcpatch *pa)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return false;
  return contains_set_value(s, PointerGetDatum(pa));
}

bool
contained_pcpatch_set(const Pcpatch *pa, const Set *s)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return false;
  return contained_value_set(PointerGetDatum(pa), s);
}

Set *
union_set_pcpatch(const Set *s, const Pcpatch *pa)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return NULL;
  return union_set_value(s, PointerGetDatum(pa));
}

Set *
union_pcpatch_set(const Pcpatch *pa, const Set *s)
{
  return union_set_pcpatch(s, pa);
}

Set *
intersection_set_pcpatch(const Set *s, const Pcpatch *pa)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return NULL;
  return intersection_set_value(s, PointerGetDatum(pa));
}

Set *
intersection_pcpatch_set(const Pcpatch *pa, const Set *s)
{
  return intersection_set_pcpatch(s, pa);
}

Set *
minus_pcpatch_set(const Pcpatch *pa, const Set *s)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return NULL;
  return minus_value_set(PointerGetDatum(pa), s);
}

Set *
minus_set_pcpatch(const Set *s, const Pcpatch *pa)
{
  if (! ensure_valid_pcpatchset_pcpatch(s, pa))
    return NULL;
  return minus_set_value(s, PointerGetDatum(pa));
}

/*****************************************************************************
 * Aggregate — pcpatch union
 *****************************************************************************/

Set *
pcpatch_union_transfn(Set *state, const Pcpatch *pa)
{
  VALIDATE_NOT_NULL(pa, NULL);
  if (state && ! ensure_set_isof_type(state, T_PCPATCHSET))
    return NULL;
  return value_union_transfn(state, PointerGetDatum(pa), T_PCPATCH);
}

/*****************************************************************************/
