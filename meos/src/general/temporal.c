/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Basic functions for temporal types of any subtype
 */

#include "general/temporal.h"

/* C */
#include <assert.h>
#include <float.h>
#include <geos_c.h>
#include <limits.h>
/* POSTGRESQL */
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_meos.h"
#include "general/lifting.h"
#include "general/temporal_boxops.h"
#include "general/temporal_tile.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that the pointer is not null
 */
bool
ensure_not_null(void *ptr)
{
  if (ptr == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "Null pointer not allowed");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that at least one of the pointers is not null
 */
bool
ensure_one_not_null(void *ptr1, void *ptr2)
{
  if (ptr1 == NULL && ptr2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "At least one pointer must be not null");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that at least one of the pointers is not null
 */
bool
ensure_one_true(bool hasshift, bool haswidth)
{
  if (! hasshift && ! haswidth)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "At least one of the arguments shift or width must be given");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the interpolation is valid
 * @note Used for the constructor functions
 */
bool
ensure_valid_interp(meosType temptype, interpType interp)
{
  if (interp == LINEAR && ! temptype_continuous(temptype))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal type cannot have linear interpolation");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the subtype of temporal type is a sequence (set)
 */
bool
ensure_continuous(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Input must be a temporal continuous sequence (set)");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two temporal values have the same interpolation
 * @param[in] temp1,temp2 Temporal values
 */
bool
ensure_same_interp(const Temporal *temp1, const Temporal *temp2)
{
  interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
  if (interp1 != interp2)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal values must have the same interpolation");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the temporal values have the same continuous
 * interpolation
 */
bool
ensure_same_continuous_interp(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_STEP_LINEAR_INTERP(flags1) &&
      MEOS_FLAGS_STEP_LINEAR_INTERP(flags2) &&
      MEOS_FLAGS_GET_INTERP(flags1) != MEOS_FLAGS_GET_INTERP(flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal values must have the same continuous interpolation");
    return false;
  }
  return true;
}

#if 0 /* not used */
/**
 * @brief Ensure that a temporal value does not have discrete interpolation
 */
bool
ensure_not_discrete_interp(int16 flags)
{
  if (MEOS_FLAGS_DISCRETE_INTERP(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal value cannot have discrete interpolation");
    return false;
  }
  return true;
}
#endif /* not used */

/**
 * @brief Ensure that a temporal value has linear interpolation
 */
bool
ensure_linear_interp(int16 flags)
{
  if (! MEOS_FLAGS_LINEAR_INTERP(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal value must have linear interpolation");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal value does not have linear interpolation
 */
bool
ensure_nonlinear_interp(int16 flags)
{
  if (MEOS_FLAGS_LINEAR_INTERP(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal value cannot have linear interpolation");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two temporal values have at least one common dimension
 * based on their flags
 */
bool
ensure_common_dimension(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) &&
      MEOS_FLAGS_GET_T(flags1) != MEOS_FLAGS_GET_T(flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal values must have at least one common dimension");
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal value has the same base type as the given one
 * @param[in] temp Input value
 * @param[in] basetype Input base type
 */
bool
ensure_temporal_isof_basetype(const Temporal *temp, meosType basetype)
{
  if (temptype_basetype(temp->temptype) != basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal type and base type: %s, %s",
      meostype_name(temp->temptype), meostype_name(basetype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal value is of a temporal type
 */
bool
ensure_temporal_isof_type(const Temporal *temp, meosType temptype)
{
  if (temp->temptype != temptype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be of type %s", meostype_name(temptype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two temporal values have the same temporal type
 * @param[in] temp1,temp2 Temporal values
 */
bool
ensure_same_temporal_type(const Temporal *temp1, const Temporal *temp2)
{
  if (temp1->temptype != temp2->temptype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal types: %s and %s",
      meostype_name(temp1->temptype), meostype_name(temp2->temptype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal value is of a temporal type
 */
bool
ensure_temporal_isof_subtype(const Temporal *temp, tempSubtype subtype)
{
  if (temp->subtype != subtype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be of subtype %s", tempsubtype_name(subtype));
    return false;
  }
  return true;
}

#if MEOS
/**
 * @brief Ensure that a temporal number and a span have the same span type
 * @param[in] temp Temporal value
 * @param[in] s Span value
 */
bool
ensure_valid_tnumber_span(const Temporal *temp, const Span *s)
{
  if (temptype_basetype(temp->temptype) != s->basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal number type and span type: %s, %s",
      meostype_name(temp->temptype), meostype_name(s->spantype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal number and a span have the same span type
 * @param[in] temp Temporal value
 * @param[in] ss Span set value
 */
bool
ensure_valid_tnumber_spanset(const Temporal *temp, const SpanSet *ss)
{
  if (temptype_basetype(temp->temptype) != ss->basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal number type and span type: %s, %s",
      meostype_name(temp->temptype), meostype_name(ss->spantype));
    return false;
  }
  return true;
}
#endif /* MEOS */

/**
 * @brief Ensure that a temporal number and a temporal box have the same span
 * type
 * @param[in] temp Temporal value
 * @param[in] box Box value
 */
bool
ensure_valid_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  if (MEOS_FLAGS_GET_X(box->flags) &&
      temptype_basetype(temp->temptype) != box->span.basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal number type and box span type: %s, %s",
      meostype_name(temp->temptype), meostype_name(box->span.spantype));
    return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Ensure that the number is positive or zero
 */
bool
ensure_not_negative(int i)
{
  if (i < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value cannot be negative: %d", i);
    return false;
  }
  return true;
}

/**
 * @brief Ensure that the number is positive
 */
bool
ensure_positive(int i)
{
  if (i <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value must be strictly positive: %d", i);
    return false;
  }
  return true;
}

#if 0 /* not used */
/**
 * @brief Ensure that the first value is less or equal than the second one
 */
bool
ensure_less_equal(int i, int j)
{
  if (i > j)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The first value must be less or equal than the second one: %d, %d",
      i, j);
    return false;
  }
  return true;
}
#endif /* not used */

/**
 * @brief Return true if the number is not negative
 */
bool
not_negative_datum(Datum size, meosType basetype)
{
  assert(span_basetype(basetype));
  if (basetype == T_INT4 && DatumGetInt32(size) < 0)
    return false;
  else if (basetype == T_FLOAT8 && DatumGetFloat8(size) < 0.0)
    return false;
  /* basetype == T_TIMESTAMPTZ */
  else if (DatumGetInt64(size) < 0)
    return false;
  return true;
}

/**
 * @brief Ensure that the number is not negative
 */
bool
ensure_not_negative_datum(Datum size, meosType basetype)
{
  if (! not_negative_datum(size, basetype))
  {
    char str[256];
    assert(basetype == T_INT4 || basetype == T_FLOAT8 ||
      basetype == T_TIMESTAMPTZ);
    if (basetype == T_INT4)
      sprintf(str, "%d", DatumGetInt32(size));
    else if (basetype == T_FLOAT8)
      sprintf(str, "%f", DatumGetFloat8(size));
    else /* basetype == T_TIMESTAMPTZ */
      sprintf(str, INT64_FORMAT, DatumGetInt64(size));
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value cannot be negative: %s", str);
    return false;
  }
  return true;
}

/**
 * @brief Return true if the number is strictly positive
 */
bool
positive_datum(Datum size, meosType basetype)
{
  assert(basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8 ||
    basetype == T_DATE || basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4 && DatumGetInt32(size) <= 0)
    return false;
  if (basetype == T_INT8 && DatumGetInt64(size) <= 0)
    return false;
  if (basetype == T_FLOAT8 && DatumGetFloat8(size) <= 0.0)
    return false;
  /* For dates the value expected are integers */
  if (basetype == T_DATE && DatumGetInt32(size) <= 0.0)
    return false;
  /* basetype == T_TIMESTAMPTZ */
  if (DatumGetInt64(size) <= 0)
    return false;
  return true;
}

/**
 * @brief Ensure that the number is strictly positive
 */
bool
ensure_positive_datum(Datum size, meosType basetype)
{
  if (! positive_datum(size, basetype))
  {
    char str[256];
    if (basetype == T_INT4)
      sprintf(str, "%d", DatumGetInt32(size));
    else if (basetype == T_FLOAT8)
      sprintf(str, "%f", DatumGetFloat8(size));
#if 0 /* not used */
    else if (basetype == T_INT8)
      sprintf(str, "%ld", DatumGetInt64(size));
    else /* basetype == T_TIMESTAMPTZ */
      sprintf(str, INT64_FORMAT, DatumGetInt64(size));
#endif /* not used */
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The value must be strictly positive: %s", str);
    return false;
  }
  return true;
}

/**
 * @brief Return true if the interval is a positive and absolute duration
 */
bool
valid_duration(const Interval *duration)
{
  if (duration->month != 0)
    return false;
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  if (pg_interval_cmp(duration, &intervalzero) <= 0)
    return false;
  return true;
}

/**
 * @brief Ensure that the interval is a positive and absolute duration
 */
bool
ensure_valid_duration(const Interval *duration)
{
  if (valid_duration(duration))
    return true;
  char *str = pg_interval_out(duration);
  if (duration->month != 0)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Interval defined in terms of month, year, century, etc. not supported: %s", str);
  else
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The interval must be positive: %s", str);
  pfree(str);
  return false;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return a pointer to the precomputed bounding box of a temporal value
 * @note Return NULL for temporal instants since they do not have bounding box
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  /* Values of TINSTANT subtype have not bounding box */
  if (temp->subtype == TSEQUENCE)
    return TSEQUENCE_BBOX_PTR((TSequence *) temp);
  else if (temp->subtype == TSEQUENCESET)
    return TSEQUENCESET_BBOX_PTR((TSequenceSet *) temp);
  return NULL;
}

/**
 * @brief Temporally intersect the two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[in] mode Either intersection or synchronization
 * @param[out] inter1,inter2 Output values
 * @result Return false if the values do not overlap on time
 */
bool
intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  SyncMode mode, Temporal **inter1, Temporal **inter2)
{
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  switch (temp1->subtype)
  {
    case TINSTANT:
      switch (temp2->subtype)
      {
        case TINSTANT:
          return intersection_tinstant_tinstant(
            (TInstant *) temp1, (TInstant *) temp2,
            (TInstant **) inter1, (TInstant **) inter2);
        case TSEQUENCE:
          return intersection_tinstant_tsequence(
            (TInstant *) temp1, (TSequence *) temp2,
            (TInstant **) inter1, (TInstant **) inter2);
        default: /* TSEQUENCESET */
          return intersection_tinstant_tsequenceset(
            (TInstant *) temp1, (TSequenceSet *) temp2,
            (TInstant **) inter1, (TInstant **) inter2);
      }
    case TSEQUENCE:
      switch (temp2->subtype)
      {
        case TINSTANT:
          return intersection_tsequence_tinstant(
            (TSequence *) temp1, (TInstant *) temp2,
            (TInstant **) inter1, (TInstant **) inter2);
        case TSEQUENCE:
          {
            bool disc1 = MEOS_FLAGS_DISCRETE_INTERP(temp1->flags);
            bool disc2 = MEOS_FLAGS_DISCRETE_INTERP(temp2->flags);
            if (disc1 && disc2)
              return intersection_tdiscseq_tdiscseq(
                  (TSequence *) temp1, (TSequence *) temp2,
                  (TSequence **) inter1, (TSequence **) inter2);
            else if (disc1 && ! disc2)
              return intersection_tdiscseq_tcontseq(
                  (TSequence *) temp1, (TSequence *) temp2,
                  (TSequence **) inter1, (TSequence **) inter2);
            else if (! disc1 && disc2)
              return intersection_tcontseq_tdiscseq(
                  (TSequence *) temp1, (TSequence *) temp2,
                  (TSequence **) inter1, (TSequence **) inter2);
            else /* !disc1 && ! disc2 */
              return synchronize_tsequence_tsequence(
                  (TSequence *) temp1, (TSequence *) temp2,
                  (TSequence **) inter1, (TSequence **) inter2,
                    mode == SYNCHRONIZE_CROSS);
          }
        default: /* TSEQUENCESET */
          return MEOS_FLAGS_DISCRETE_INTERP(temp1->flags) ?
            intersection_tdiscseq_tsequenceset(
              (TSequence *) temp1, (TSequenceSet *) temp2,
              (TSequence **) inter1, (TSequence **) inter2) :
            intersection_tsequence_tsequenceset(
                (TSequence *) temp1, (TSequenceSet *) temp2, mode,
                (TSequenceSet **) inter1, (TSequenceSet **) inter2);
      }
    default: /* TSEQUENCESET */
      switch (temp2->subtype)
      {
        case TINSTANT:
          return intersection_tsequenceset_tinstant(
            (TSequenceSet *) temp1, (TInstant *) temp2,
            (TInstant **) inter1, (TInstant **) inter2);
        case TSEQUENCE:
          return MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
            intersection_tsequenceset_tdiscseq(
              (TSequenceSet *) temp1, (TSequence *) temp2,
              (TSequence **) inter1, (TSequence **) inter2) :
            synchronize_tsequenceset_tsequence(
              (TSequenceSet *) temp1, (TSequence *) temp2, mode,
              (TSequenceSet **) inter1, (TSequenceSet **) inter2);
        default: /* TSEQUENCESET */
          return synchronize_tsequenceset_tsequenceset(
            (TSequenceSet *) temp1, (TSequenceSet *) temp2, mode,
            (TSequenceSet **) inter1, (TSequenceSet **) inter2);
      }
  }
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

#define MOBDB_VERSION_STR_MAX_LEN 256
/**
 * @ingroup meos_misc
 * @brief Return the version of the MobilityDB extension
 */
char *
mobilitydb_version(void)
{
  return MOBILITYDB_VERSION_STRING;
}

/**
 * @ingroup meos_misc
 * @brief Return the versions of the MobilityDB extension and its dependencies
 */
char *
mobilitydb_full_version(void)
{
  const char *proj_vers;
#if POSTGIS_PROJ_VERSION < 61
  proj_vers = pj_get_release();
#else
  PJ_INFO pji = proj_info();
  proj_vers = pji.version;
#endif
  const char* geos_vers = GEOSversion();
  const char* json_c_vers = json_c_version();

  char *result = palloc(sizeof(char) * MOBDB_VERSION_STR_MAX_LEN);
  int len = snprintf(result, MOBDB_VERSION_STR_MAX_LEN,
    "%s, %s, %s, GEOS %s, PROJ %s, JSON-C %s, GSL %s",
    MOBILITYDB_VERSION_STRING, POSTGRESQL_VERSION_STRING,
    POSTGIS_VERSION_STRING, geos_vers, proj_vers, json_c_vers,
    GSL_VERSION_STRING);
  result[len] = '\0';
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal value from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @param[in] temptype Temporal type
 */
Temporal *
temporal_in(const char *str, meosType temptype)
{
  assert(str);
  return temporal_parse(&str, temptype);
}

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tbool_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TBOOL);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal integer from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
Temporal *
tint_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TINT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
tfloat_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TFLOAT);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation
 * @param[in] str String
 */
Temporal *
ttext_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;
  return temporal_parse(&str, T_TTEXT);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
temporal_out(const Temporal *temp, int maxdd)
{
  assert(temp);
  if (! ensure_not_negative(maxdd))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_out((TInstant *) temp, maxdd);
    case TSEQUENCE:
      return tsequence_out((TSequence *) temp, maxdd);
    default: /* TSEQUENCESET */
      return tsequenceset_out((TSequenceSet *) temp, maxdd);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation
 * @param[in] temp Temporal value
 */
char *
tbool_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal integer from its Well-Known Text (WKT)
 * representation
 * @param[in] temp Temporal value
 */
char *
tint_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tfloat_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_out(temp, maxdd);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation
 * @param[in] temp Temporal value
 */
char *
ttext_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal geometry/geography point from its Well-Known Text
 * (WKT) representation
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
tpoint_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;
  return temporal_out(temp, maxdd);
}
#endif /* MEOS */

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a copy of a temporal value
 * @param[in] temp Temporal value
 */
Temporal *
temporal_cp(const Temporal *temp)
{
  assert(temp);
  Temporal *result = palloc(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a copy of a temporal value
 * @param[in] temp Temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_cp(temp);
}

/**
 * @brief Return a temporal discrete sequence from a base value and time frame
 * of another temporal discrete sequence
 */
static TSequence *
tdiscseq_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  assert(seq); assert(MEOS_FLAGS_DISCRETE_INTERP(seq->flags));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype, TSEQUENCE_INST_N(seq, i)->t);
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Return a temporal sequence from a base value and the time frame of
 * another temporal sequence
 */
TSequence *
tsequence_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  assert(seq);
  return MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
    tdiscseq_from_base_temp(value, temptype, seq) :
    tsequence_from_base_tstzspan(value, temptype, &seq->period,
      temptype_continuous(temptype) ? LINEAR : STEP);
}

/**
 * @brief Return a temporal sequence set from a base value and the time
 * frame of another temporal sequence set
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] ss Sequence set
 * @note The interpolation of the result is step or linear depending on whether
 * the base type is continous or not.
 */
static TSequenceSet *
tsequenceset_from_base_temp(Datum value, meosType temptype,
  const TSequenceSet *ss)
{
  assert(ss);
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tsequence_from_base_tstzspan(value, temptype, &seq->period,
      interp);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_temporal_constructor
 * @brief Return a temporal value from a base value and the time frame of
 * another temporal value
 * @note For continuous sequence (sets), the resulting interpolation is linear
 * or step depending on whether the temporal type is, respectively, continuous
 * or not.
 * @param[in] value Value
 * @param[in] temptype Type of the resulting temporal value
 * @param[in] temp Temporal value
 */
Temporal *
temporal_from_base_temp(Datum value, meosType temptype, const Temporal *temp)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_make(value, temptype,
        ((TInstant *) temp)->t);
    case TSEQUENCE:
      return (Temporal *) tsequence_from_base_temp(value, temptype,
        (TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_from_base_temp(value, temptype,
        (TSequenceSet *) temp);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal boolean from a boolean and the time frame of
 * another temporal value
 * @param[in] b Value
 * @param[in] temp Temporal value
 */
Temporal *
tbool_from_base_temp(bool b, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_from_base_temp(BoolGetDatum(b), T_TBOOL, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal integer from an integer and the time frame of
 * another temporal value
 * @param[in] i Value
 * @param[in] temp Temporal value
 */
Temporal *
tint_from_base_temp(int i, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_from_base_temp(Int32GetDatum(i), T_TINT, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal float from a float and the time frame of
 * another temporal value
 * @param[in] d Value
 * @param[in] temp Temporal value
 */
Temporal *
tfloat_from_base_temp(double d, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  return temporal_from_base_temp(Float8GetDatum(d), T_TFLOAT, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal text from a text and the time frame of
 * another temporal value
 * @param[in] txt Value
 * @param[in] temp Temporal value
 */
Temporal *
ttext_from_base_temp(const text *txt, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt))
    return NULL;
  return temporal_from_base_temp(PointerGetDatum(txt), T_TTEXT, temp);
}

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a temporal geometry point from a point and the time frame
 * of another temporal value
 * @param[in] gs Value
 * @param[in] temp Temporal value
 */
Temporal *
tpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  meosType geotype = FLAGS_GET_GEODETIC(gs->gflags) ? T_TGEOGPOINT :
    T_TGEOMPOINT;
  return temporal_from_base_temp(PointerGetDatum(gs), geotype, temp);
}
#endif /* MEOS */

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return an integer converted to a float
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_int_to_float(Datum d)
{
  return Float8GetDatum((double) DatumGetInt32(d));
}

/**
 * @brief Return a float converted to an integer
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_float_to_int(Datum d)
{
  return Int32GetDatum((int) DatumGetFloat8(d));
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Return a temporal integer converted to a temporal float
 * @param[in] temp Temporal value
 * @csqlfn #Tint_to_tfloat()
 */
Temporal *
tint_to_tfloat(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_int_to_float;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Return a temporal float converted to a temporal integer
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_to_tint()
 */
Temporal *
tfloat_to_tint(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot cast temporal float with linear interpolation to temporal integer");
    return NULL;
  }

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_float_to_int;
  lfinfo.numparam = 0;
  lfinfo.restype = T_TINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the time span of a temporal
 * value
 * @param[in] temp Temporal value
 * @param[out] s Span
 */
void
temporal_set_tstzspan(const Temporal *temp, Span *s)
{
  assert(temp); assert(s);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tinstant_set_tstzspan((TInstant *) temp, s);
      break;
    case TSEQUENCE:
      tsequence_set_tstzspan((TSequence *) temp, s);
      break;
    default: /* TSEQUENCESET */
      tsequenceset_set_tstzspan((TSequenceSet *) temp, s);
  }
  return;
}

#if MEOS
/**
 * @ingroup meos_temporal_conversion
 * @brief Return the bounding period of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_to_tstzspan()
 */
Span *
temporal_to_tstzspan(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  Span *result = palloc(sizeof(Span));
  temporal_set_tstzspan(temp, result);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the value span of a
 * temporal number
 * @param[in] temp Temporal value
 * @param[out] s Span
 */
void
tnumber_set_span(const Temporal *temp, Span *s)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  meosType basetype = temptype_basetype(temp->temptype);
  meosType spantype = basetype_spantype(basetype);
  if (temp->subtype == TINSTANT)
  {
    Datum value = tinstant_val((TInstant *) temp);
    span_set(value, value, true, true, basetype, spantype, s);
  }
  else
  {
    TBox *box = (TBox *) temporal_bbox_ptr(temp);
    memcpy(s, &box->span, sizeof(Span));
  }
  return;
}

/**
 * @ingroup meos_internal_temporal_conversion
 * @brief Return the value span of a temporal number
 * @param[in] temp Temporal value
 */
Span *
tnumber_span(const Temporal *temp)
{
  assert(temp); assert(tnumber_type(temp->temptype));
  Span *result = palloc(sizeof(Span));
  tnumber_set_span(temp, result);
  return result;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Return the value span of a temporal number
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_to_span()
 */
Span *
tnumber_to_span(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tnumber_type(temp->temptype))
    return NULL;
  return tnumber_span(temp);
}

#if MEOS
/**
 * @ingroup meos_box_conversion
 * @brief Return a temporal number converted to a temporal box
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_to_tbox()
 */
TBox *
tnumber_to_tbox(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tnumber_type(temp->temptype))
    return NULL;

  TBox *result = palloc(sizeof(TBox));
  temporal_set_bbox(temp, result);
  return result;
}
#endif

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a float value converted from radians to degrees
 * @param[in] value Value
 * @param[in] normalize True when the result is normalized
 * @csqlfn #Float_degrees()
 */
double
float_degrees(double value, bool normalize)
{
  double result = float8_div(value, RADIANS_PER_DEGREE);
  if (normalize)
  {
    /* The value would be in the range (-360, 360) */
    result = fmod(result, 360.0);
    if (result < 0)
      result += 360.0; /* The value would be in the range [0, 360) */
  }
  return result;
}

/**
 * @brief Return a number converted from radians to degrees
 */
Datum
datum_degrees(Datum value, Datum normalize)
{
  return Float8GetDatum(float_degrees(DatumGetFloat8(value),
    DatumGetBool(normalize)));
}

/**
 * @brief Return a number converted from degrees to radians
 */
Datum
datum_radians(Datum value)
{
  return Float8GetDatum(float8_mul(DatumGetFloat8(value), RADIANS_PER_DEGREE));
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number transformed from radians to degrees
 * @param[in] temp Temporal value
 * @param[in] normalize True when the result is normalized
 * @csqlfn #Tfloat_degrees()
 */
Temporal *
tfloat_degrees(const Temporal *temp, bool normalize)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_degrees;
  lfinfo.numparam = 1;
  lfinfo.param[0] = BoolGetDatum(normalize);
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number transformed from degrees to radians
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_radians()
 */
Temporal *
tfloat_radians(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;

  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_radians;
  lfinfo.numparam = 0;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a copy of the temporal value without any extra storage space
 * @param[in] temp Temporal value
 */
Temporal *
temporal_compact(const Temporal *temp)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tsequence_compact((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_compact((TSequenceSet *) temp);
  }
}

#if MEOS
/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal sequence (set) restarted by keeping only the last n
 * instants
 * or sequences
 * @param[in] temp Temporal value
 * @param[out] count Number of instants or sequences kept
 */
void
temporal_restart(Temporal *temp, int count)
{
  assert(temp); assert(count > 0);
  assert(temptype_subtype(temp->subtype));
  assert(temp->subtype != TINSTANT);

  if (temp->subtype == TSEQUENCE)
    tsequence_restart((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_restart((TSequenceSet *) temp, count);
  return;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value transformed into a temporal instant
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_to_tinstant()
 */
TInstant *
temporal_to_tinstant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_to_tinstant((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_to_tinstant((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence
 * @param[in] temp Temporal value
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_to_tsequence()
 */
TSequence *
temporal_tsequence(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interp(temp->temptype, interp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_tsequence((TInstant *) temp, interp);
    case TSEQUENCE:
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(temp->flags);
      if (interp1 == DISCRETE && interp != DISCRETE &&
        ((TSequence *) temp)->count > 1)
      {
        /* The first character should be transformed to lowercase */
        char *str = pstrdup(interptype_name(interp));
        str[0] = tolower(str[0]);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
          "Cannot transform input value to a temporal sequence with %s interpolation",
          str);
        return NULL;
      }
      /* Given the above test, the result subtype is TSequence */
      return (TSequence *) tsequence_set_interp((TSequence *) temp, interp);
    }
    default: /* TSEQUENCESET */
      return tsequenceset_to_tsequence((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string, may be NULL
 * @csqlfn #Temporal_to_tsequence()
 */
TSequence *
temporal_to_tsequence(const Temporal *temp, char *interp_str)
{
  interpType interp;
  /* If the interpolation is not NULL */
  if (interp_str)
    interp = interptype_from_string(interp_str);
  else
  {
    if (temp->subtype == TSEQUENCE)
      interp = MEOS_FLAGS_GET_INTERP(temp->flags);
    else
      interp = MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP;
  }
  return temporal_tsequence(temp, interp);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence set
 * @param[in] temp Temporal value
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
temporal_tsequenceset(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interp(temp->temptype, interp))
    return NULL;
  /* Discrete interpolation is only valid for TSequence */
  if (interp == DISCRETE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal sequence set cannot have discrete interpolation");
    return NULL;
  }

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_tsequenceset((TInstant *) temp, interp);
    case TSEQUENCE:
      return tsequence_to_tsequenceset_interp((TSequence *) temp, interp);
    default: /* TSEQUENCESET */
      /* Since interp != DISCRETE the result subtype is TSequenceSet */
      return (TSequenceSet *) tsequenceset_set_interp((TSequenceSet *) temp,
        interp);
  }
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence set
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
temporal_to_tsequenceset(const Temporal *temp, char *interp_str)
{
  interpType interp;
  /* If the interpolation is not NULL */
  if (interp_str)
    interp = interptype_from_string(interp_str);
  else
  {
    interp = MEOS_FLAGS_GET_INTERP(temp->flags);
    if (interp == INTERP_NONE || interp == DISCRETE)
      interp = MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP;
  }
  return temporal_tsequenceset(temp, interp);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value transformed to a given interpolation
 * @param[in] temp Temporal value
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_set_interp()
 */
Temporal *
temporal_set_interp(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interp(temp->temptype, interp))
    return NULL;

  Temporal *result;
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequence((TInstant *) temp, interp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_set_interp((TSequence *) temp, interp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_set_interp((TSequenceSet *) temp,
      interp);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal number with the value dimension shifted and/or
 * scaled by two values
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Tnumber_shift_value(), #Tnumber_scale_value(),
 *   #Tnumber_shift_scale_value()
 */
Temporal *
tnumber_shift_scale_value(const Temporal *temp, Datum shift, Datum width,
  bool hasshift, bool haswidth)
{
  assert(temp);
  meosType basetype = temptype_basetype(temp->temptype);
  /* Ensure validity of the arguments */
  if (! ensure_one_true(hasshift, haswidth) ||
      (width && ! ensure_positive_datum(width, basetype)))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (hasshift) ?
        (Temporal *) tnumberinst_shift_value((TInstant *) temp, shift) :
        (Temporal *) tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tnumberseq_shift_scale_value((TSequence *) temp,
        shift, width, hasshift, haswidth);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_shift_scale_value((TSequenceSet *) temp,
        shift, width, hasshift, haswidth);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value
 * @csqlfn #Tnumber_shift_value()
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 */
Temporal *
tint_shift_value(const Temporal *temp, int shift)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tnumber_shift_scale_value(temp, Int32GetDatum(shift), 0, true, false);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value
 * @csqlfn #Tnumber_shift_value()
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 */
Temporal *
tfloat_shift_value(const Temporal *temp, double shift)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tnumber_shift_scale_value(temp, Float8GetDatum(shift), 0, true, false);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_scale_value()
 */
Temporal *
tint_scale_value(const Temporal *temp, int width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tnumber_shift_scale_value(temp, 0, Int32GetDatum(width), false, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_scale_value()
 */
Temporal *
tfloat_scale_value(const Temporal *temp, double width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tnumber_shift_scale_value(temp, 0, Float8GetDatum(width), false, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_shift_scale_value()
 */
Temporal *
tint_shift_scale_value(const Temporal *temp, int shift, int width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return tnumber_shift_scale_value(temp, Int32GetDatum(shift),
    Int32GetDatum(width), true, true);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value
 * @param[in] temp Temporal value
 * @param[in] shift Value for shifting the temporal value
 * @param[in] width Width of the result
 * @csqlfn #Tnumber_shift_scale_value()
 */
Temporal *
tfloat_shift_scale_value(const Temporal *temp, double shift, double width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return tnumber_shift_scale_value(temp, Float8GetDatum(shift),
    Float8GetDatum(width), true, true);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value shifted and/or scaled by two intervals
 * @param[in] temp Temporal value
 * @param[in] shift Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 * @csqlfn #Temporal_shift_time(), #Temporal_scale_time(),
 *   #Temporal_shift_scale_time()
 */
Temporal *
temporal_shift_scale_time(const Temporal *temp, const Interval *shift,
  const Interval *duration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (shift != NULL) ?
        (Temporal *) tinstant_shift_time((TInstant *) temp, shift) :
        (Temporal *) tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tsequence_shift_scale_time((TSequence *) temp,
        shift, duration);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_shift_scale_time((TSequenceSet *) temp,
        shift, duration);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value shifted by an interval
 * @param[in] temp Temporal value
 * @param[in] shift Interval for shifting the temporal value
 * @csqlfn #Temporal_shift_time()
 */
Temporal *
temporal_shift_time(const Temporal *temp, const Interval *shift)
{
  return temporal_shift_scale_time(temp, shift, NULL);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal value scaled by an interval
 * @param[in] temp Temporal value
 * @param[in] duration Duration of the result
 * @csqlfn #Temporal_scale_time()
 */
Temporal *
temporal_scale_time(const Temporal *temp, const Interval *duration)
{
  return temporal_shift_scale_time(temp, NULL, duration);
}
#endif /* MEOS */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the size in bytes of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_mem_size()
 */
size_t
temporal_mem_size(const Temporal *temp)
{
  assert(temp);
  return VARSIZE(temp);
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_subtype()
 */
const char *
temporal_subtype(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  assert(temptype_subtype(temp->subtype));
  return tempsubtype_name(temp->subtype);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the string representation of the interpolation of a temporal
 * value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_interp()
 */
const char *
temporal_interp(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;
  assert(temptype_subtype(temp->subtype));
  return interptype_name(MEOS_FLAGS_GET_INTERP(temp->flags));
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the last argument initialized with the bounding box of a
 * temporal value
 * @param[in] temp Temporal value
 * @param[out] box Boundind box
 * @note For temporal instants the bounding box must be computed. For the
 * other subtypes, a copy of the precomputed bounding box is made.
 */
void
temporal_set_bbox(const Temporal *temp, void *box)
{
  assert(temp); assert(box);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tinstant_set_bbox((TInstant *) temp, box);
      return;
    case TSEQUENCE:
      tsequence_set_bbox((TSequence *) temp, box);
      return;
    default: /* TSEQUENCESET */
      tsequenceset_set_bbox((TSequenceSet *) temp, box);
      return;
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of (pointers to the) distinct base values of a
 * temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Datum *
temporal_vals(const Temporal *temp, int *count)
{
  assert(temp); assert(count);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_vals((TInstant *) temp, count);
    case TSEQUENCE:
      return tsequence_vals((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_vals((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of (copies of the) distinct base values of a
 * temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
  assert(temp); assert(count);
  Datum *result = temporal_vals(temp, count);
  if (MEOS_FLAGS_GET_BYVAL(temp->flags))
    return result;
  meosType basetype =  temptype_basetype(temp->temptype);
  for (int i = 0; i < *count; i++)
    result[i] = datum_copy(result[i], basetype);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal boolean
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
bool *
tbool_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return NULL;

  Datum *datumarr = temporal_vals(temp, count);
  bool *result = palloc(sizeof(bool) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetBool(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal integer
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
int *
tint_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;

  Datum *datumarr = temporal_vals(temp, count);
  int *result = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetInt32(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal float
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
double *
tfloat_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;

  Datum *datumarr = temporal_vals(temp, count);
  double *result = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetFloat8(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of copies of base values of a temporal text
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
text **
ttext_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;

  Datum *datumarr = temporal_vals(temp, count);
  text **result = palloc(sizeof(text *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = text_copy(DatumGetTextP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of base values of a temporal geometry point
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
GSERIALIZED **
tpoint_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  Datum *datumarr = temporal_vals(temp, count);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = geo_copy(DatumGetGserializedP(datumarr[i]));
  pfree(datumarr);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the base values of a temporal number as a span set
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_valuespans()
 */
SpanSet *
tnumber_valuespans(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tnumber_type(temp->temptype))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnumberinst_valuespans((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_valuespans((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnumberseqset_valuespans((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the time frame of a temporal value as a span set
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_time()
 */
SpanSet *
temporal_time(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_time((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_time((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_time((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a copy of) the start base value of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_start_value()
 */
Datum
temporal_start_value(const Temporal *temp)
{
  assert(temp); assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_value((TInstant *) temp);
    case TSEQUENCE:
      return tinstant_value(TSEQUENCE_INST_N((TSequence *) temp, 0));
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
      return tinstant_value(TSEQUENCE_INST_N(seq, 0));
    }
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal boolean
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_start_value()
 */
bool
tbool_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return DatumGetBool(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_start_value()
 */
int
tint_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_start_value()
 */
double
tfloat_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
text *
ttext_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_start_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start value of a temporal point
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
GSERIALIZED *
tpoint_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;
  return DatumGetGserializedP(temporal_start_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a copy of) the end base value of a temporal value
 * @param[in] temp Temporal value
 */
Datum
temporal_end_value(const Temporal *temp)
{
  assert(temp); assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_value((TInstant *) temp);
    case TSEQUENCE:
      return tinstant_value(TSEQUENCE_INST_N((TSequence *) temp,
        ((TSequence *) temp)->count - 1));
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
        ((TSequenceSet *) temp)->count - 1);
      return tinstant_value(TSEQUENCE_INST_N(seq, seq->count - 1));
    }
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal boolean
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_end_value()
 */
bool
tbool_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TBOOL))
    return false;
  return DatumGetBool(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_end_value()
 */
int
tint_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_end_value()
 */
double
tfloat_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
text *
ttext_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_end_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end value of a temporal point
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
GSERIALIZED *
tpoint_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;
  return DatumGetGserializedP(temporal_end_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a copy of) the minimum base value of a temporal value
 * @param[in] temp Temporal value
 */
Datum
temporal_min_value(const Temporal *temp)
{
  assert(temp);
  meosType basetype = temptype_basetype(temp->temptype);
  assert(temptype_subtype(temp->subtype));
  Datum result;
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinstant_val((TInstant *) temp);
      break;
    case TSEQUENCE:
      result = tsequence_min_val((TSequence *) temp);
      break;
    default: /* TSEQUENCESET */
      result = tsequenceset_min_val((TSequenceSet *) temp);
  }
  return MEOS_FLAGS_GET_BYVAL(temp->flags) ?
    result : datum_copy(result, basetype);
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the minimum value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_min_value()
 */
int
tint_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_min_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the minimum value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_min_value()
 */
double
tfloat_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_min_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the minimum value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_min_value()
 */
text *
ttext_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_min_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return (a copy of) the maximum base value of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_max_value()
 */
Datum
temporal_max_value(const Temporal *temp)
{
  assert(temp);
  meosType basetype = temptype_basetype(temp->temptype);
  assert(temptype_subtype(temp->subtype));
  Datum result;
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinstant_val((TInstant *) temp);
      break;
    case TSEQUENCE:
      result = tsequence_max_val((TSequence *) temp);
      break;
    default: /* TSEQUENCESET */
      result = tsequenceset_max_val((TSequenceSet *) temp);
  }
  return MEOS_FLAGS_GET_BYVAL(temp->flags) ?
    result : datum_copy(result, basetype);
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return the maximum value of a temporal integer
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_max_value()
 */
int
tint_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_max_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the maximum value of a temporal float
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Temporal_max_value()
 */
double
tfloat_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_max_value(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the maximum value of a temporal text
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_max_value()
 */
text *
ttext_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_max_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * value
 * @details Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 */
const TInstant *
temporal_min_inst(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (TInstant *) temp;
    case TSEQUENCE:
      return tsequence_min_inst((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_min_inst((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the instant with minimum base value of a temporal
 * value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @csqlfn #Temporal_min_instant()
 */
TInstant *
temporal_min_instant(const Temporal *temp)
{
  return tinstant_copy(temporal_min_inst(temp));
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the instant with maximum base value of a temporal
 * value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_max_instant()
 */
TInstant *
temporal_max_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return tinstant_copy(tsequence_max_inst((TSequence *) temp));
    default: /* TSEQUENCESET */
      return tinstant_copy(tsequenceset_max_inst((TSequenceSet *) temp));
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the duration of a temporal value
 * @param[in] temp Temporal value
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return @p NULL
 * @csqlfn #Temporal_duration()
 */
Interval *
temporal_duration(const Temporal *temp, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return palloc0(sizeof(Interval));
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags) && ! boundspan)
        return palloc0(sizeof(Interval));
      else
        return tsequence_duration((TSequence *) temp);
    }
    default: /* TSEQUENCESET */
      return tsequenceset_duration((TSequenceSet *) temp, boundspan);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the number of sequences of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @return On error return -1
 * @csqlfn #Temporal_num_sequences()
 */
int
temporal_num_sequences(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return -1;
  return (temp->subtype == TSEQUENCE) ? 1 : ((TSequenceSet *) temp)->count;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_sequence()
 */
TSequence *
temporal_start_sequence(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
    return tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    return tsequence_copy(TSEQUENCESET_SEQ_N(ss, 0));
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_sequence()
 */
TSequence *
temporal_end_sequence(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
    return tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    return tsequence_copy(TSEQUENCESET_SEQ_N(ss, ss->count - 1));
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the n-th sequence of a temporal sequence (set)
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @return On error return @p NULL
 * @note n is assumed to be 1-based.
 * @csqlfn #Temporal_sequence_n()
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int n)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
  {
    if (n == 1)
      return tsequence_copy((TSequence *) temp);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    if (n >= 1 && n <= ss->count)
      return tsequence_copy(TSEQUENCESET_SEQ_N(ss, n - 1));
  }
  return NULL;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return an array of pointers to the sequences of a temporal sequence
 * (set)
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequences()
 */
const TSequence **
temporal_seqs(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
  {
    *count = 1;
    return tsequence_seqs((TSequence *) temp, count);
  }
  else /* temp->subtype == TSEQUENCE */
  {
    *count = ((TSequenceSet *) temp)->count;
    return tsequenceset_seqs((TSequenceSet *) temp);
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return an array of copies of the sequences of a temporal sequence
 * (set)
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequences()
 */
TSequence **
temporal_sequences(const Temporal *temp, int *count)
{
  /* We do a casting to avoid allocating a new array of sequences */
  TSequence **sequences = (TSequence **) temporal_seqs(temp, count);
  for (int i = 0; i < *count; i ++)
    sequences[i] = tsequence_copy(sequences[i]);
  return sequences;
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of segments of a temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_segments()
 */
TSequence **
temporal_segments(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count))
    return NULL;
  if (temp->subtype == TINSTANT)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be of subtype sequence (set)");
    return NULL;
  }

  return (temp->subtype == TSEQUENCE) ?
    tsequence_segments((TSequence *) temp, count) :
    /* temp->subtype == TSEQUENCESET */
    tsequenceset_segments((TSequenceSet *) temp, count);
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return 1 if the start instant of a temporal value is inclusive
 * @param[in] temp Temporal value
 * @return On error return -1
 * @csqlfn #Temporal_lower_inc()
 */
int
temporal_lower_inc(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return 1;
    case TSEQUENCE:
      return ((TSequence *) temp)->period.lower_inc;
    default: /* TSEQUENCESET */
      return ((TSequenceSet *) temp)->period.lower_inc;
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return 1 if the end instant of a temporal value is inclusive
 * @param[in] temp Temporal value
 * @return On error return -1
 * @csqlfn #Temporal_upper_inc()
 */
int
temporal_upper_inc(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return 1;
    case TSEQUENCE:
      return ((TSequence *) temp)->period.upper_inc;
    default: /* TSEQUENCESET */
      return ((TSequenceSet *) temp)->period.upper_inc;
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the number of distinct instants of a temporal value
 * @param[in] temp Temporal value
 * @return On error return -1
 * @csqlfn #Temporal_num_instants()
 */
int
temporal_num_instants(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return 1;
    case TSEQUENCE:
      return ((TSequence *) temp)->count;
    default: /* TSEQUENCESET */
      return tsequenceset_num_instants((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the start instant of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_instant()
 */
TInstant *
temporal_start_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return tinstant_copy(TSEQUENCE_INST_N((TSequence *) temp, 0));
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
      return tinstant_copy(TSEQUENCE_INST_N(seq, 0));
    }
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the end instant of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @note This function is used for validity testing.
 * @csqlfn #Temporal_end_instant()
 */
TInstant *
temporal_end_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_copy((TInstant *) temp);
    case TSEQUENCE:
      return tinstant_copy(TSEQUENCE_INST_N((TSequence *) temp,
        ((TSequence *) temp)->count - 1));
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
        ((TSequenceSet *) temp)->count - 1);
      return tinstant_copy(TSEQUENCE_INST_N(seq, seq->count - 1));
    }
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the n-th instant of a temporal value
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @return On error return @p NULL
 * @note n is assumed 1-based
 * @csqlfn #Temporal_instant_n()
 */
TInstant *
temporal_instant_n(const Temporal *temp, int n)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      if (n == 1)
        return tinstant_copy((const TInstant *) temp);
      return NULL;
    }
    case TSEQUENCE:
    {
      if (n >= 1 && n <= ((TSequence *) temp)->count)
        return tinstant_copy(TSEQUENCE_INST_N((TSequence *) temp, n - 1));
      return NULL;
    }
    default: /* TSEQUENCESET */
    {
      /* This test is necessary since the n-th DISTINCT instant is requested */
      if (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount)
      {
        const TInstant *inst = tsequenceset_inst_n((TSequenceSet *) temp, n);
        return inst ? tinstant_copy(inst) : NULL;
      }
      return NULL;
    }
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return an array of pointers to the distinct instants of a temporal
 * value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 */
const TInstant **
temporal_insts(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_insts((TInstant *) temp, count);
    case TSEQUENCE:
      *count = ((TSequence *) temp)->count;
      return tsequence_insts((TSequence *) temp);
    default: /* TSEQUENCESET */
    {
      const TInstant **result = tsequenceset_insts((TSequenceSet *) temp);
      *count = tinstarr_remove_duplicates(result,
        ((TSequenceSet *) temp)->totalcount);
      return result;
    }
  }
}

#if MEOS
/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the distinct instants of a temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_instants()
 */
TInstant **
temporal_instants(const Temporal *temp, int *count)
{
  /* We do a casting to avoid allocating a new array of instants */
  TInstant **instants = (TInstant **) temporal_insts(temp, count);
  for (int i = 0; i < *count; i ++)
    instants[i] = tinstant_copy(instants[i]);
  return instants;
}
#endif /* MEOS */

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the number of distinct timestamps of a temporal value
 * @param[in] temp Temporal value
 * @return On error return -1
 * @csqlfn #Temporal_num_timestamps()
 */
int
temporal_num_timestamps(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return 1;
    case TSEQUENCE:
      return ((TSequence *) temp)->count;
    default: /* TSEQUENCESET */
      return tsequenceset_num_timestamps((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the start timestamp of a temporal value
 * @param[in] temp Temporal value
 * @return On error return DT_NOEND
 * @csqlfn #Temporal_start_timestamptz()
 */
TimestampTz
temporal_start_timestamptz(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return DT_NOEND;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return ((TInstant *) temp)->t;
    case TSEQUENCE:
      return tsequence_start_timestamptz((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_start_timestamptz((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the end timestamptz of a temporal value
 * @param[in] temp Temporal value
 * @return On error return DT_NOEND
 * @csqlfn #Temporal_end_timestamptz()
 */
TimestampTz
temporal_end_timestamptz(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return DT_NOEND;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return ((TInstant *) temp)->t;
    case TSEQUENCE:
      return tsequence_end_timestamptz((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_end_timestamptz((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the n-th distinct timestamp of a temporal value in the last
 * argument
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Resulting timestamp
 * @return On error return false
 * @note n is assumed 1-based
 * @csqlfn #Temporal_timestamptz_n()
 */
bool
temporal_timestamptz_n(const Temporal *temp, int n, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) result))
    return false;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      if (n == 1)
      {
        *result = ((TInstant *) temp)->t;
        return true;
      }
      return false;
    }
    case TSEQUENCE:
    {
      if (n >= 1 && n <= ((TSequence *) temp)->count)
      {
        *result = TSEQUENCE_INST_N((TSequence *) temp, n - 1)->t;
        return true;
      }
      return false;
    }
    default: /* TSEQUENCESET */
      return tsequenceset_timestamptz_n((TSequenceSet *) temp, n, result);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the array of distinct timestamps of a temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @return On error return @p NULL
 * @csqlfn #Temporal_timestamps()
 */
TimestampTz *
temporal_timestamps(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_timestamps((TInstant *) temp, count);
    case TSEQUENCE:
      return tsequence_timestamps((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_timestamps((TSequenceSet *) temp, count);
  }
}

/*****************************************************************************
 * Stops functions
 *****************************************************************************/

/**
 * @brief Calculate the length of the minimum bounding interval
 * of the input sequence between the given start and end instants
 */
static double
mrr_distance_scalar(const TSequence *seq, int start, int end)
{
  assert(seq);
  assert(seq->temptype == T_TFLOAT);
  double min_value, max_value, curr_value;
  min_value = DatumGetFloat8(tinstant_val(TSEQUENCE_INST_N(seq, start)));
  max_value = min_value;
  for (int i = start + 1; i < end + 1; ++i)
  {
    curr_value = DatumGetFloat8(tinstant_val(TSEQUENCE_INST_N(seq, i)));
    min_value = fmin(min_value, curr_value);
    max_value = fmax(max_value, curr_value);
  }
  return max_value - min_value;
}

/**
 * @brief Return the subsequences where the temporal value stays within an area
 * with a given maximum size for at least the specified duration (iterator
 * function)
 * @param[in] seq Temporal sequence
 * @param[in] maxdist Maximum distance
 * @param[in] mintunits Minimum duration
 * @param[out] result Resulting sequences
 */
static int
tfloatseq_stops_iter(const TSequence *seq, double maxdist, int64 mintunits,
  TSequence **result)
{
  assert(seq->temptype == T_TFLOAT);
  assert(seq->count > 1);

  const TInstant *inst1 = NULL, *inst2 = NULL; /* make compiler quiet */
  int end, start = 0, nseqs = 0;
  bool  is_stopped = false,
        previously_stopped = false;

  for (end = 0; end < seq->count; ++end)
  {
    inst1 = TSEQUENCE_INST_N(seq, start);
    inst2 = TSEQUENCE_INST_N(seq, end);

    while (! is_stopped && end - start > 1
      && (int64)(inst2->t - inst1->t) >= mintunits)
    {
      inst1 = TSEQUENCE_INST_N(seq, ++start);
    }

    if (end - start == 0)
      continue;

    is_stopped = mrr_distance_scalar(seq, start, end) <= maxdist;

    inst2 = TSEQUENCE_INST_N(seq, end - 1);
    if (! is_stopped && previously_stopped
      && (int64)(inst2->t - inst1->t) >= mintunits) // Found a stop
    {
      const TInstant **insts = palloc(sizeof(TInstant *) * (end - start));
      for (int i = 0; i < end - start; ++i)
          insts[i] = TSEQUENCE_INST_N(seq, start + i);
      result[nseqs++] = tsequence_make(insts, end - start,
        true, true, LINEAR, NORMALIZE_NO);
      start = end;
    }
    previously_stopped = is_stopped;
  }

  inst2 = TSEQUENCE_INST_N(seq, end - 1);
  if (is_stopped && (int64)(inst2->t - inst1->t) >= mintunits)
  {
    const TInstant **insts = palloc(sizeof(TInstant *) * (end - start));
    for (int i = 0; i < end - start; ++i)
        insts[i] = TSEQUENCE_INST_N(seq, start + i);
    result[nseqs++] = tsequence_make(insts, end - start,
      true, true, LINEAR, NORMALIZE_NO);
  }
  return nseqs;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration
 * @param[in] seq Temporal sequence
 * @param[in] maxdist Maximum distance
 * @param[in] mintunits Minimum duration
 */
TSequenceSet *
tsequence_stops(const TSequence *seq, double maxdist, int64 mintunits)
{
  assert(seq);
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int nseqs = (seq->temptype == T_TFLOAT) ?
    tfloatseq_stops_iter(seq, maxdist, mintunits, sequences) :
    tpointseq_stops_iter(seq, maxdist, mintunits, sequences);
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration
 * @param[in] ss Temporal sequence set
 * @param[in] maxdist Maximum distance
 * @param[in] mintunits Minimum duration
 */
TSequenceSet *
tsequenceset_stops(const TSequenceSet *ss, double maxdist, int64 mintunits)
{
  assert(ss);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Instantaneous sequences do not have stops */
    if (seq->count == 1)
      continue;
    nseqs += (seq->temptype == T_TFLOAT) ?
      tfloatseq_stops_iter(seq, maxdist, mintunits, &sequences[nseqs]) :
      tpointseq_stops_iter(seq, maxdist, mintunits, &sequences[nseqs]);
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration
 * @param[in] temp Temporal value
 * @param[in] maxdist Maximum distance
 * @param[in] minduration Minimum duration
 */
TSequenceSet *
temporal_stops(const Temporal *temp, double maxdist,
  const Interval *minduration)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_not_negative_datum(Float8GetDatum(maxdist), T_FLOAT8))
    return NULL;

  /* We cannot call #ensure_valid_duration since the duration may be zero */
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = pg_interval_cmp(minduration, &intervalzero);
  if (cmp < 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The duration must be positive");
    return NULL;
  }
  int64 mintunits = interval_units(minduration);

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Input must be a temporal sequence (set) with linear interpolation");
    return NULL;
  }
  else if (temp->subtype == TSEQUENCE)
    return tsequence_stops((TSequence *) temp, maxdist, mintunits);
  else /* temp->subtype == TSEQUENCESET */
    return tsequenceset_stops((TSequenceSet *) temp, maxdist, mintunits);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the integral (area under the curve) of a temporal number
 * @param[in] temp Temporal value
 * @return On error return -1.0
 */
double
tnumber_integral(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tnumber_type(temp->temptype))
    return -1.0;

  assert(temptype_subtype(temp->subtype));
   switch (temp->subtype)
  {
    case TINSTANT:
      return 0.0;
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ? 0.0 :
        tnumberseq_integral((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnumberseqset_integral((TSequenceSet *) temp);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the time-weighted average of a temporal number
 * @param[in] temp Temporal value
 * @return On error return @p DBL_MAX
 * @csqlfn #Tnumber_twavg()
 */
double
tnumber_twavg(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tnumber_type(temp->temptype))
    return DBL_MAX;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tnumberinst_double((TInstant *) temp);
    case TSEQUENCE:
      return tnumberseq_twavg((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tnumberseqset_twavg((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Comparison functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the temporal values are equal
 * @param[in] temp1,temp2 Temporal values
 * @note The function #temporal_cmp is not used to increase efficiency
 * @csqlfn #Temporal_eq()
 */
bool
temporal_eq(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return false;

  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  /* If both are of the same temporal type use the specific equality */
  if (temp1->subtype == temp2->subtype)
  {
    switch (temp1->subtype)
    {
      case TINSTANT:
        return tinstant_eq((TInstant *) temp1, (TInstant *) temp2);
      case TSEQUENCE:
        return tsequence_eq((TSequence *) temp1, (TSequence *) temp2);
      default: /* TSEQUENCESET */
        return tsequenceset_eq((TSequenceSet *) temp1, (TSequenceSet *) temp2);
    }
  }

  /* Different temporal type */
  const TInstant *inst1;
  const TSequence *seq;
  const TSequenceSet *ss;
  if (temp1->subtype > temp2->subtype)
  {
    const Temporal *temp = (Temporal *) temp1;
    temp1 = temp2;
    temp2 = temp;
  }
  if (temp1->subtype == TINSTANT)
  {
    const TInstant *inst = (TInstant *) temp1;
    if (temp2->subtype == TSEQUENCE)
    {
      seq = (TSequence *) temp2;
      if (seq->count != 1)
        return false;
      inst1 = TSEQUENCE_INST_N(seq, 0);
      return tinstant_eq(inst, inst1);
    }
    if (temp2->subtype == TSEQUENCESET)
    {
      ss = (TSequenceSet *) temp2;
      if (ss->count != 1)
        return false;
      seq = TSEQUENCESET_SEQ_N(ss, 0);
      if (seq->count != 1)
        return false;
      inst1 = TSEQUENCE_INST_N(seq, 0);
      return tinstant_eq(inst, inst1);
    }
  }
  /* temp1->subtype == TSEQUENCE && temp2->subtype == TSEQUENCESET */
  seq = (TSequence *) temp1;
  ss = (TSequenceSet *) temp2;
  const TSequence *seq1;
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
  {
    for (int i = 0; i < seq->count; i++)
    {
      seq1 = TSEQUENCESET_SEQ_N(ss, i);
      if (seq1->count != 1)
        return false;
      inst1 = TSEQUENCE_INST_N(seq, i);
      const TInstant *inst2 = TSEQUENCE_INST_N(seq1, 0);
      if (! tinstant_eq(inst1, inst2))
        return false;
    }
    return true;
  }
  else
  {
    if (ss->count != 1)
      return false;
    seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    return tsequence_eq(seq, seq1);
  }
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the temporal values are different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_ne()
 */
bool
temporal_ne(const Temporal *temp1, const Temporal *temp2)
{
  return ! temporal_eq(temp1, temp2);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one
 * @param[in] temp1,temp2 Temporal values
 * @note Function used for B-tree comparison
 * @csqlfn #Temporal_cmp()
 */
int
temporal_cmp(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return INT_MAX;

  /* Compare bounding box */
  bboxunion box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  int result = temporal_bbox_cmp(&box1, &box2, temp1->temptype);
  if (result)
    return result;

  /* If both are of the same temporal type use the specific comparison */
  if (temp1->subtype == temp2->subtype)
  {
    assert(temptype_subtype(temp1->subtype));
    switch (temp1->subtype)
    {
      case TINSTANT:
        return tinstant_cmp((TInstant *) temp1, (TInstant *) temp2);
      case TSEQUENCE:
        return tsequence_cmp((TSequence *) temp1, (TSequence *) temp2);
      default: /* TSEQUENCESET */
        return tsequenceset_cmp((TSequenceSet *) temp1, (TSequenceSet *) temp2);
    }
  }

  /* Use the hash comparison */
  uint32 hash1 = temporal_hash(temp1);
  uint32 hash2 = temporal_hash(temp2);
  if (hash1 < hash2)
    return -1;
  else if (hash1 > hash2)
    return 1;

  /* Compare memory size */
  size_t size1 = VARSIZE(temp1);
  size_t size2 = VARSIZE(temp2);
  if (size1 < size2)
    return -1;
  else if (size1 > size2)
    return 1;

  /* Compare flags */
  if (temp1->flags < temp2->flags)
    return -1;
  if (temp1->flags > temp2->flags)
    return 1;

  /* Finally compare temporal subtype */
  if (temp1->subtype < temp2->subtype)
    return -1;
  else if (temp1->subtype > temp2->subtype)
    return 1;
  else
    return 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is less than the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_lt()
 */
bool
temporal_lt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp < 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_le()
 */
bool
temporal_le(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp <= 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_gt()
 */
bool
temporal_ge(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp >= 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_ge()
 */
bool
temporal_gt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp > 0;
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal value
 * @param[in] temp Temporal value
 * @result On error return @p INT_MAX
 * @csqlfn #Temporal_hash()
 */
uint32
temporal_hash(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return INT_MAX;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_hash((TInstant *) temp);
    case TSEQUENCE:
      return tsequence_hash((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tsequenceset_hash((TSequenceSet *) temp);
  }
}

/*****************************************************************************/
