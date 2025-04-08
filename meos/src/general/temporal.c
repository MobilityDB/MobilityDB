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
 * @brief Basic functions for temporal types of any subtype
 */

#include "general/temporal.h"

/* C */
#include <assert.h>
#include <float.h>
#include <geos_c.h>
#include <limits.h>
/* PostgreSQL */
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_meos.h"
#include "general/lifting.h"
#include "general/pg_types.h"
#include "general/temporal_boxops.h"
#include "general/temporal_tile.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a MEOS type has X dimension
 */
bool
ensure_has_X(meosType type, int16 flags)
{
  if (MEOS_FLAGS_GET_X(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The %s must have X dimension", meostype_name(type));
  return false;
}

/**
 * @brief Ensure that a MEOS type has Z dimension
 */
bool
ensure_has_Z(meosType type, int16 flags)
{
  if (MEOS_FLAGS_GET_Z(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The %s must have Z dimension", meostype_name(type));
  return false;
}

/**
 * @brief Ensure that a MEOS type has not Z dimension
 */
bool
ensure_has_not_Z(meosType type, int16 flags)
{
  if (! MEOS_FLAGS_GET_Z(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The %s cannot have Z dimension", meostype_name(type));
  return false;
}

/**
 * @brief Ensure that a MEOS type has Z dimension
 */
bool
ensure_has_T(meosType type, int16 flags)
{
  if (MEOS_FLAGS_GET_T(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The %s must have T dimension", meostype_name(type));
  return false;
}

/**
 * @brief Ensure that the pointer is not null
 */
bool
ensure_not_null(void *ptr)
{
  if (ptr)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG, "Null pointer not allowed");
  return false;
}

/**
 * @brief Ensure that at least one of the pointers is not null
 */
bool
ensure_one_not_null(void *ptr1, void *ptr2)
{
  if (ptr1 || ptr2)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG,
    "At least one pointer must be not null");
  return false;
}

/**
 * @brief Ensure that at least one of the values is true
 */
bool
ensure_one_true(bool hasshift, bool haswidth)
{
  if (hasshift || haswidth)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG,
    "At least one of the arguments shift or width must be given");
  return false;
}

/**
 * @brief Ensure that an interpolation is valid
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
  if (temp->subtype != TINSTANT && ! MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Input must be a temporal continuous sequence (set)");
  return false;
}

/**
 * @brief Ensure that two temporal values have the same interpolation
 * @param[in] temp1,temp2 Temporal values
 */
bool
ensure_same_interp(const Temporal *temp1, const Temporal *temp2)
{
  if (MEOS_FLAGS_GET_INTERP(temp1->flags) ==
      MEOS_FLAGS_GET_INTERP(temp2->flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The temporal values must have the same interpolation");
  return false;
}

/**
 * @brief Ensure that two temporal values have the same continuous
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

/**
 * @brief Ensure that a temporal value has linear interpolation
 */
bool
ensure_linear_interp(int16 flags)
{
  if (MEOS_FLAGS_LINEAR_INTERP(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The temporal value must have linear interpolation");
  return false;
}

/**
 * @brief Ensure that a temporal value does not have linear interpolation
 */
bool
ensure_nonlinear_interp(int16 flags)
{
  if (! MEOS_FLAGS_LINEAR_INTERP(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The temporal value cannot have linear interpolation");
  return false;
}

/**
 * @brief Ensure that two temporal values have at least one common dimension
 */
bool
ensure_common_dimension(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) == MEOS_FLAGS_GET_X(flags2) ||
      MEOS_FLAGS_GET_T(flags1) == MEOS_FLAGS_GET_T(flags2))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The temporal values must have at least one common dimension");
  return false;
}

#if MEOS
/**
 * @brief Ensure that a temporal value has a given base type
 * @param[in] temp Temporal value
 * @param[in] basetype Base type
 */
bool
ensure_temporal_isof_basetype(const Temporal *temp, meosType basetype)
{
  if (temptype_basetype(temp->temptype) == basetype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed temporal type and base type: %s, %s",
    meostype_name(temp->temptype), meostype_name(basetype));
  return false;
}
#endif /* MEOS */

/**
 * @brief Ensure that a temporal value is of a temporal type
 */
bool
ensure_temporal_isof_type(const Temporal *temp, meosType temptype)
{
  if (temp->temptype == temptype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The temporal value must be of type %s", meostype_name(temptype));
  return false;
}

/**
 * @brief Ensure that two temporal values have the same temporal type
 * @param[in] temp1,temp2 Temporal values
 */
bool
ensure_same_temporal_type(const Temporal *temp1, const Temporal *temp2)
{
  if (temp1->temptype == temp2->temptype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed temporal types: %s and %s",
    meostype_name(temp1->temptype), meostype_name(temp2->temptype));
  return false;
}

/**
 * @brief Ensure that a temporal value is of a given subtype
 */
bool
ensure_temporal_isof_subtype(const Temporal *temp, tempSubtype subtype)
{
  if (temp->subtype == subtype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "The temporal value must be of subtype %s", tempsubtype_name(subtype));
  return false;
}

#if MEOS
/**
 * @brief Ensure that a temporal number and a span have the same span type
 * @param[in] temp Temporal value
 * @param[in] s Span value
 */
bool
ensure_valid_tnumber_numspan(const Temporal *temp, const Span *s)
{
  VALIDATE_TNUMBER(temp, false); VALIDATE_NOT_NULL(s, false);
  if (temptype_basetype(temp->temptype) == s->basetype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed temporal number and span types: %s, %s",
    meostype_name(temp->temptype), meostype_name(s->spantype));
  return false;
}

/**
 * @brief Ensure that a temporal number and a span set have the same span type
 * @param[in] temp Temporal value
 * @param[in] ss Span set value
 */
bool
ensure_valid_tnumber_numspanset(const Temporal *temp, const SpanSet *ss)
{
  VALIDATE_TNUMBER(temp, false); VALIDATE_NOT_NULL(ss, false);
  if (temptype_basetype(temp->temptype) == ss->basetype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed temporal number and span set types: %s, %s",
    meostype_name(temp->temptype), meostype_name(ss->spantype));
  return false;
}
#endif /* MEOS */

/**
 * @brief Ensure that a temporal number and a temporal box have the same span
 * type
 * @param[in] temp Temporal value
 * @param[in] box Temporal box value
 */
bool
ensure_valid_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  VALIDATE_TNUMBER(temp, false); VALIDATE_NOT_NULL(box, false);
  if (MEOS_FLAGS_GET_X(box->flags) &&
      temptype_basetype(temp->temptype) != box->span.basetype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal number and box span types: %s, %s",
      meostype_name(temp->temptype), meostype_name(box->span.spantype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a temporal number and a set have the same span type
 * @param[in] temp Temporal value
 * @param[in] s Span value
 */
bool
ensure_valid_temporal_set(const Temporal *temp, const Set *s)
{
  VALIDATE_NOT_NULL(temp, false); VALIDATE_NOT_NULL(s, false);
  if (temptype_basetype(temp->temptype) == s->basetype)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
    "Operation on mixed temporal and set types: %s, %s",
    meostype_name(temp->temptype), meostype_name(s->settype));
  return false;
}

/**
 * @brief Ensure that a temporal number and a temporal box have the same span
 * type
 * @param[in] temp Temporal value
 * @param[in] box Temporal box value
 */
bool
ensure_valid_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_NOT_NULL(temp1, false); VALIDATE_NOT_NULL(temp2, false);
  if (temp1->temptype != temp2->temptype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal types: %s, %s",
      meostype_name(temp1->temptype), meostype_name(temp2->temptype));
    return false;
  }
  return true;
}

/**
 * @brief Ensure that two temporal numbers have the same span type
 * @param[in] temp Temporal value
 * @param[in] box Temporal box value
 */
bool
ensure_valid_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TNUMBER(temp1, false); VALIDATE_TNUMBER(temp2, false);
  if (temp1->temptype != temp2->temptype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "Operation on mixed temporal number types: %s, %s",
      meostype_name(temp1->temptype), meostype_name(temp2->temptype));
    return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Ensure that a number is positive or zero
 */
bool
ensure_not_negative(int i)
{
  if (i >= 0)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The value cannot be negative: %d", i);
  return false;
}

/**
 * @brief Ensure that a number is positive
 */
bool
ensure_positive(int i)
{
  if (i > 0)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The value must be strictly positive: %d", i);
  return false;
}

/**
 * @brief Return true if a number is not negative
 */
bool
not_negative_datum(Datum d, meosType basetype)
{
  assert(span_basetype(basetype));
  if (basetype == T_INT4 && DatumGetInt32(d) < 0)
    return false;
  else if (basetype == T_FLOAT8 && DatumGetFloat8(d) < 0.0)
    return false;
  /* basetype == T_TIMESTAMPTZ */
  else if (DatumGetInt64(d) < 0)
    return false;
  return true;
}

/**
 * @brief Ensure that a number is not negative
 */
bool
ensure_not_negative_datum(Datum d, meosType basetype)
{
  if (not_negative_datum(d, basetype))
    return true;
  char str[256];
  assert(basetype == T_INT4 || basetype == T_FLOAT8 ||
    basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4)
    snprintf(str, sizeof(str), "%d", DatumGetInt32(d));
  else if (basetype == T_FLOAT8)
    snprintf(str, sizeof(str), "%f", DatumGetFloat8(d));
  else /* basetype == T_TIMESTAMPTZ */
    snprintf(str, sizeof(str), INT64_FORMAT, DatumGetInt64(d));
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The value cannot be negative: %s", str);
  return false;
}

/**
 * @brief Return true if a number is strictly positive
 */
bool
positive_datum(Datum d, meosType basetype)
{
  assert(basetype == T_INT4 || basetype == T_INT8 || basetype == T_FLOAT8 ||
    basetype == T_DATE || basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4 && DatumGetInt32(d) <= 0)
    return false;
  if (basetype == T_INT8 && DatumGetInt64(d) <= 0)
    return false;
  if (basetype == T_FLOAT8 && DatumGetFloat8(d) <= 0.0)
    return false;
  /* For dates the value expected are integers */
  if (basetype == T_DATE && DatumGetInt32(d) <= 0.0)
    return false;
  /* basetype == T_TIMESTAMPTZ */
  if (DatumGetInt64(d) <= 0)
    return false;
  return true;
}

/**
 * @brief Ensure that a number is strictly positive
 */
bool
ensure_positive_datum(Datum d, meosType basetype)
{
  if (positive_datum(d, basetype))
    return true;
  char str[256];
  if (basetype == T_INT4)
    snprintf(str, sizeof(str), "%d", DatumGetInt32(d));
  else if (basetype == T_FLOAT8)
    snprintf(str, sizeof(str), "%f", DatumGetFloat8(d));
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The value must be strictly positive: %s", str);
  return false;
}

/**
 * @brief Ensure that an interval does not have a month component
 * @note Binning by months is currently not supported
 */
bool
ensure_not_month_duration(const Interval *duration)
{
  if (! duration->month)
    return true;
  char *str = pg_interval_out(duration);
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Cannot have month intervals: %s", str);
  pfree(str);
  return false;
}

/**
 * @brief Ensure that a day interval for binning is valid
 * @note Binning by months is currently not supported
 */
bool
ensure_valid_day_duration(const Interval *duration)
{
  if (! ensure_not_month_duration(duration))
    return false;

  char *str;
  int64 day = USECS_PER_DAY;
  int64 tunits = interval_units(duration);
  if (tunits < day)
  {
    str = pg_interval_out(duration);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The interval must not have sub-day precision: %s", str);
    pfree(str);
    return false;
  }
  if (tunits % day != 0)
  {
    str = pg_interval_out(duration);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The interval must be a multiple of a day: %s", str);
    pfree(str);
    return false;
  }
  if (tunits < 0)
  {
    str = pg_interval_out(duration);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The interval must be positive: %s", str);
    pfree(str);
    return false;
  }
  return true;
}

/**
 * @brief Return true if an interval is a positive duration
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
 * @brief Ensure that an interval is a positive duration
 */
bool
ensure_valid_duration(const Interval *duration)
{
  if (valid_duration(duration))
    return true;

  if (! ensure_not_month_duration(duration))
    return false;

  char *str = pg_interval_out(duration);
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
 * @return Return NULL for temporal instants since they do not have bounding box
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  if (temp->subtype == TSEQUENCE)
    return TSEQUENCE_BBOX_PTR((TSequence *) temp);
  if (temp->subtype == TSEQUENCESET)
    return TSEQUENCESET_BBOX_PTR((TSequenceSet *) temp);
  /* Values of TINSTANT subtype have not bounding box */
  return NULL;
}

/**
 * @brief Return the temporal intersection of two temporal values
 * @param[in] temp1,temp2 Temporal values
 * @param[in] mode Either intersection or synchronization mode
 * @param[out] inter1,inter2 Output values
 * @return Return false if the values do not overlap on time
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

#define MOBDB_VERSION_STR_MAXLEN 256
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
  const char *geos_vers = GEOSversion();
  const char *json_c_vers = json_c_version();

  char *result = palloc(MOBDB_VERSION_STR_MAXLEN);
  size_t len = snprintf(result, MOBDB_VERSION_STR_MAXLEN,
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return temporal_parse(&str, temptype);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits
 */
char *
temporal_out(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
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

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * temporal values
 * @param[in] temparr Array of temporal value
 * @param[in] count Number of elements in the input array
 * @param[in] maxdd Number of decimal digits
 */
char **
temparr_out(const Temporal **temparr, int count, int maxdd)
{
  assert(temparr); assert(count > 0); assert(maxdd >=0);
  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = temporal_out(temparr[i], maxdd);
  return result;
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup meos_temporal_constructor
 * @brief Return a copy of a temporal value
 * @param[in] temp Temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  Temporal *result = palloc(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
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
 * or step depending on whether the argument temporal type is, respectively,
 * continuous or not.
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

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Convert a Boolean into an integer
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_bool_to_int(Datum d)
{
  return Int32GetDatum((int) DatumGetBool(d));
}

/**
 * @brief Convert an integer into a float
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_int_to_float(Datum d)
{
  return Float8GetDatum((double) DatumGetInt32(d));
}

/**
 * @brief Convert a float into an integer
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
 * @brief Convert a temporal Boolean into a temporal integer
 * @param[in] temp Temporal value
 * @csqlfn #Tbool_to_tint()
 */
Temporal *
tbool_tint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TBOOL(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_bool_to_int;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TBOOL;
  lfinfo.restype = T_TINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Convert a temporal integer into a temporal float
 * @param[in] temp Temporal value
 * @csqlfn #Tint_to_tfloat()
 */
Temporal *
tint_tfloat(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_int_to_float;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TINT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Convert a temporal float into a temporal integer
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_to_tint()
 */
Temporal *
tfloat_tint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
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
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return in the last argument the time span of a temporal value
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

/**
 * @ingroup meos_temporal_conversion
 * @brief Return the bounding period of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_to_tstzspan()
 */
Span *
temporal_tstzspan(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  Span *result = palloc(sizeof(Span));
  temporal_set_tstzspan(temp, result);
  return result;
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return in the last argument the value span of a temporal number
 * @param[in] temp Temporal value
 * @param[out] s Span
 */
void
tnumber_set_span(const Temporal *temp, Span *s)
{
  assert(temp); assert(s); assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));

  meosType basetype = temptype_basetype(temp->temptype);
  meosType spantype = basetype_spantype(basetype);
  if (temp->subtype == TINSTANT)
  {
    Datum value = tinstant_value_p((TInstant *) temp);
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
 * @ingroup meos_temporal_conversion
 * @brief Return the value span of a temporal number
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_to_span()
 */
Span *
tnumber_span(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL); 
  Span *result = palloc(sizeof(Span));
  tnumber_set_span(temp, result);
  return result;
}

/**
 * @ingroup meos_temporal_conversion
 * @brief Convert a temporal number into a temporal box
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_to_tbox()
 */
TBox *
tnumber_tbox(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL); 
  TBox *result = palloc(sizeof(TBox));
  tnumber_set_tbox(temp, result);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Return the function for rounding a base type
 */
datum_func2
round_fn(meosType basetype)
{
  assert(meos_basetype(basetype));
  switch (basetype)
  {
    case T_FLOAT8:
      return &datum_float_round;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return &datum_geo_round;
#if CBUFFER
    case T_CBUFFER:
      return &datum_cbuffer_round;
#endif
#if NPOINT
    case T_NPOINT:
      return &datum_npoint_round;
#endif
#if POSE || RGEO
    case T_POSE:
      return &datum_pose_round;
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown round function for type: %s", meostype_name(basetype));
    return NULL;
  }
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a temporal value rounded to a given number of decimal places
 * @param[in] temp Temporal value
 * @param[in] maxdd Maximum number of decimal digits to output
 */
Temporal *
temporal_round(const Temporal *temp, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) round_fn(temptype_basetype(temp->temptype));
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(maxdd);
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return an array of temporal floats with the precision of the
 * coordinates set to a number of decimal places
 * @param[in] temparr Array of temporal values
 * @param[in] count Number of values in the input array
 * @param[in] maxdd Maximum number of decimal digits
 */
Temporal **
temparr_round(const Temporal **temparr, int count, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temparr, NULL);
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
    result[i] = temporal_round(temparr[i], maxdd);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_base_transf
 * @brief Return a float number rounded to a given number of decimal places
 */
double
float_round(double d, int maxdd)
{
  assert(maxdd >= 0);
  double inf = get_float8_infinity();
  double result = d;
  if (d != -1 * inf && d != inf)
  {
    if (maxdd == 0)
      result = round(d);
    else
    {
      double power10 = pow(10.0, maxdd);
      result = round(d * power10) / power10;
    }
  }
  return result;
}

/**
 * @brief Return a float number rounded to a given number of decimal places
 */
Datum
datum_float_round(Datum value, Datum size)
{
  return Float8GetDatum(float_round(DatumGetFloat8(value),
    DatumGetInt32(size)));
}

/*****************************************************************************/

/**
 * @brief Return a number rounded down to the nearest integer
 */
Datum
datum_floor(Datum value)
{
  return Float8GetDatum(floor(DatumGetFloat8(value)));
}

/**
 * @brief Return a number rounded up to the nearest integer
 */
Datum
datum_ceil(Datum value)
{
  return Float8GetDatum(ceil(DatumGetFloat8(value)));
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float rounded down to the nearest temporal integer
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_floor()
 */
Temporal *
tfloat_floor(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_floor;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float rounded up to the nearest temporal integer
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_ceil()
 */
Temporal *
tfloat_ceil(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_ceil;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_transf
 * @brief Return a float converted from radians to degrees
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
 * @brief Return a double converted from radians to degrees
 */
Datum
datum_degrees(Datum value, Datum normalize)
{
  return Float8GetDatum(float_degrees(DatumGetFloat8(value),
    DatumGetBool(normalize)));
}

/**
 * @brief Return a double converted from degrees to radians
 */
Datum
datum_radians(Datum value)
{
  return Float8GetDatum(float8_mul(DatumGetFloat8(value), RADIANS_PER_DEGREE));
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float transformed from radians to degrees
 * @param[in] temp Temporal value
 * @param[in] normalize True when the result is normalized
 * @csqlfn #Tfloat_degrees()
 */
Temporal *
tfloat_degrees(const Temporal *temp, bool normalize)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_degrees;
  lfinfo.numparam = 1;
  lfinfo.param[0] = BoolGetDatum(normalize);
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_temporal_transf
 * @brief Return a temporal float transformed from degrees to radians
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_radians()
 */
Temporal *
tfloat_radians(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_radians;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_internal_temporal_transf
 * @brief Return a copy of a temporal value without any extra storage space
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
 * @brief Return a temporal sequence (set) that keepsg only the last n instants
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
 * @brief Return a temporal value transformed to a temporal instant
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_to_tinstant()
 */
TInstant *
temporal_to_tinstant(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
 * @brief Return a temporal value transformed to a temporal sequence
 * @param[in] temp Temporal value
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_to_tsequence()
 */
TSequence *
temporal_tsequence(const Temporal *temp, interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_valid_interp(temp->temptype, interp))
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
 * @brief Return a temporal value transformed to a temporal sequence
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string, may be NULL
 * @csqlfn #Temporal_to_tsequence()
 */
TSequence *
temporal_to_tsequence(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
 * @brief Return a temporal value transformed to a temporal sequence set
 * @param[in] temp Temporal value
 * @param[in] interp Interpolation
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
temporal_tsequenceset(const Temporal *temp, interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_valid_interp(temp->temptype, interp))
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
 * @brief Return a temporal value transformed to a temporal sequence set
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation string
 * @csqlfn #Temporal_to_tsequenceset()
 */
TSequenceSet *
temporal_to_tsequenceset(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
 * @brief Return a temporal value transformed to an interpolation
 * @param[in] temp Temporal value
 * @param[in] interp_str Interpolation
 * @return On error return @p NULL
 * @csqlfn #Temporal_set_interp()
 */
Temporal *
temporal_set_interp(const Temporal *temp, const char *interp_str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  interpType interp = interptype_from_string(interp_str);
  if (! ensure_valid_interp(temp->temptype, interp))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tinstant_to_tsequence((TInstant *) temp, interp);
    case TSEQUENCE:
      return (Temporal *) tsequence_set_interp((TSequence *) temp, interp);
    default: /* TSEQUENCESET */
      return (Temporal *) tsequenceset_set_interp((TSequenceSet *) temp,
        interp);
  }
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
  /* Ensure the validity of the arguments */
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_one_not_null((void *) shift, (void *) duration) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (shift) ?
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

#define MEOS_INTERP_STR_MAXLEN 9

#if MEOS || DEBUG_BUILD
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
#endif /* MEOS || DEBUG_BUILD */

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_subtype()
 */
const char *
temporal_subtype(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  assert(temptype_subtype(temp->subtype));
  return interptype_name(MEOS_FLAGS_GET_INTERP(temp->flags));
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return in the last argument the bounding box of a temporal value
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
 * @brief Return the array of **pointers** to the base values of a temporal
 * value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @see #temporal_values
 */
Datum *
temporal_values_p(const Temporal *temp, int *count)
{
  assert(temp); assert(count);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_values_p((TInstant *) temp, count);
    case TSEQUENCE:
      return tsequence_values_p((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_values_p((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return the array of **copies** of the **distinct** base values of a
 * temporal value
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 * @see #temporal_values_p
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
  assert(temp); assert(count);
  int count1;
  Datum *values = temporal_values_p(temp, &count1);
  meosType basetype = temptype_basetype(temp->temptype);
  datumarr_sort(values, count1, basetype);
  int newcount = datumarr_remove_duplicates(values, count1, basetype);
  Datum *result = palloc(sizeof(Datum) * newcount);
  for (int i = 0; i < newcount; i++)
    result[i] = datum_copy(values[i], basetype);
  *count = newcount;
  return result;
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the base values of a temporal number as a span set
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_valuespans()
 */
SpanSet *
tnumber_valuespans(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_tnumber_type(temp->temptype))
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
 * @brief Return a copy of the start base value of a temporal value
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
      return tinstant_value(
        TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0), 0));
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a copy of the end base value of a temporal value
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

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a copy of the minimum base value of a temporal value
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
      result = tinstant_value_p((TInstant *) temp);
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

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a copy of the maximum base value of a temporal value
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
      result = tinstant_value_p((TInstant *) temp);
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

/**
 * @ingroup meos_temporal_accessor
 * @brief Return in the last argument a copy of the n-th value of a temporal
 * value 
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @param[out] result Resulting timestamp
 * @return On error return false
 * @csqlfn #Temporal_value_n()
 */
bool
temporal_value_n(const Temporal *temp, int n, Datum *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(result, NULL);
  if (! ensure_positive(n))
    return false;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      if (n == 1)
      {
        *result = tinstant_value((TInstant *) temp);
        return true;
      }
      return false;
    }
    case TSEQUENCE:
    {
      if (n >= 1 && n <= ((TSequence *) temp)->count)
      {
        *result = tinstant_value(TSEQUENCE_INST_N((TSequence *) temp, n - 1));
        return true;
      }
      return false;
    }
    default: /* TSEQUENCESET */
      return tsequenceset_value_n((TSequenceSet *) temp, n, result);
  }
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal value
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, -1);
  if (! ensure_continuous(temp))
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  /* Ensure the validity of the arguments */
  if (! ensure_continuous(temp))
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_continuous(temp))
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
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 * @csqlfn #Temporal_sequence_n()
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);
  if (! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
  {
    if (n == 1)
      return tsequence_copy((const TSequence *) temp);
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
temporal_sequences_p(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  assert(temp); assert(count);
  if (! ensure_continuous(temp))
    return NULL;

  if (temp->subtype == TSEQUENCE)
  {
    *count = 1;
    return tsequence_seqs((TSequence *) temp, count);
  }
  else /* temp->subtype == TSEQUENCE */
  {
    *count = ((TSequenceSet *) temp)->count;
    return tsequenceset_sequences_p((TSequenceSet *) temp);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  /* We do a casting to avoid allocating a new array of sequences */
  TSequence **sequences = (TSequence **) temporal_sequences_p(temp, count);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "The temporal value must be of subtype sequence (set)");
      return NULL;
    case TSEQUENCE:
      return tsequence_segments((TSequence *) temp, count);
    default: /* TSEQUENCESET */
      return tsequenceset_segments((TSequenceSet *) temp, count);
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return true if the start instant of a temporal value is inclusive
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_lower_inc()
 */
bool
temporal_lower_inc(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return true;
    case TSEQUENCE:
      return ((TSequence *) temp)->period.lower_inc;
    default: /* TSEQUENCESET */
      return ((TSequenceSet *) temp)->period.lower_inc;
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return true if the end instant of a temporal value is inclusive
 * @param[in] temp Temporal value
 * @csqlfn #Temporal_upper_inc()
 */
bool
temporal_upper_inc(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, false);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return true;
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, -1);

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
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the start instant of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 */
const TInstant *
temporal_start_inst(const Temporal *temp)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (TInstant *) temp;
    case TSEQUENCE:
      return TSEQUENCE_INST_N((TSequence *) temp, 0);
    default: /* TSEQUENCESET */
      return TSEQUENCE_INST_N(TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0), 0);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  /* A temporal value always has a start instant */
  return tinstant_copy(temporal_start_inst(temp));
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the end instant of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @note This function is used for validity testing.
 */
const TInstant *
temporal_end_inst(const Temporal *temp)
{
  assert(temp);
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (TInstant *) temp;
    case TSEQUENCE:
      return TSEQUENCE_INST_N((TSequence *) temp,
        ((TSequence *) temp)->count - 1);
    default: /* TSEQUENCESET */
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
        ((TSequenceSet *) temp)->count - 1);
      return TSEQUENCE_INST_N(seq, seq->count - 1);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  /* A temporal value always has an end instant */
  return tinstant_copy(temporal_end_inst(temp));
}

/**
 * @ingroup meos_internal_temporal_accessor
 * @brief Return a pointer to the n-th instant of a temporal value
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 */
const TInstant *
temporal_inst_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (n == 1) ? (TInstant *) temp : NULL;
    case TSEQUENCE:
      return (n >= 1 && n <= ((TSequence *) temp)->count) ?
        TSEQUENCE_INST_N((TSequence *) temp, n - 1) : NULL;
    default: /* TSEQUENCESET */
      /* This test is necessary since the n-th DISTINCT instant is requested */
      return (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount) ?
        tsequenceset_inst_n((TSequenceSet *) temp, n) : NULL;
  }
}

/**
 * @ingroup meos_temporal_accessor
 * @brief Return a copy of the n-th instant of a temporal value
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @return On error return @p NULL
 * @csqlfn #Temporal_instant_n()
 */
TInstant *
temporal_instant_n(const Temporal *temp, int n)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  const TInstant *result = temporal_inst_n(temp, n);
  return result ? tinstant_copy(result) : NULL;
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
temporal_instants_p(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      *count = 1;
      return tinstant_insts((TInstant *) temp, count);
    case TSEQUENCE:
      *count = ((TSequence *) temp)->count;
      return tsequence_insts_p((TSequence *) temp);
    default: /* TSEQUENCESET */
    {
      const TInstant **result = tsequenceset_insts_p((TSequenceSet *) temp);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  TInstant **instants = (TInstant **) temporal_instants_p(temp, count);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, -1);

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
 * @brief Return the start timestamptz of a temporal value
 * @param[in] temp Temporal value
 * @return On error return DT_NOEND
 * @csqlfn #Temporal_start_timestamptz()
 */
TimestampTz
temporal_start_timestamptz(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, DT_NOEND);

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, DT_NOEND);

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
 * @brief Return in last argument the n-th distinct timestamptz of a temporal
 * value
 * @param[in] temp Temporal value
 * @param[in] n Number (1-based)
 * @param[out] result Resulting timestamp
 * @return On error return false
 * @csqlfn #Temporal_timestamptz_n()
 */
bool
temporal_timestamptz_n(const Temporal *temp, int n, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(result, NULL);

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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

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
 * @brief Return the length of the minimum bounding interval of a sequence 
 * between the given start and end instants
 */
static double
mrr_distance_scalar(const TSequence *seq, int start, int end)
{
  assert(seq);
  assert(seq->temptype == T_TFLOAT);
  double min_value, max_value;
  min_value = DatumGetFloat8(tinstant_value_p(TSEQUENCE_INST_N(seq, start)));
  max_value = min_value;
  for (int i = start + 1; i < end + 1; ++i)
  {
    double curr_value = DatumGetFloat8(tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    min_value = fmin(min_value, curr_value);
    max_value = fmax(max_value, curr_value);
  }
  return max_value - min_value;
}

/**
 * @brief Return the subsequences where the temporal value stays within a
 * given distance for at least a given duration (iterator function)
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
 * a given distance for at least a given duration
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
 * a given distance for at least a given duration
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
 * @brief Return the subsequences where a temporal value stays within
 * a given distance for at least a given duration
 * @param[in] temp Temporal value
 * @param[in] maxdist Maximum distance
 * @param[in] minduration Minimum duration
 */
TSequenceSet *
temporal_stops(const Temporal *temp, double maxdist,
  const Interval *minduration)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(minduration, NULL);
  if (! ensure_not_negative_datum(Float8GetDatum(maxdist), T_FLOAT8))
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
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, -1.0);

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
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, DBL_MAX);

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
 * @brief Return true if two temporal values are equal
 * @param[in] temp1,temp2 Temporal values
 * @note The function #temporal_cmp is not used to increase efficiency
 * @csqlfn #Temporal_eq()
 */
bool
temporal_eq(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return false;

  if (tspatial_type(temp1->temptype))
  {
    if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
        ! ensure_same_spatial_dimensionality(temp1->flags, temp2->flags))
    return false;
  }

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
 * @brief Return true if two temporal values are different
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_ne()
 */
inline bool
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_temporal_temporal(temp1, temp2))
    return INT_MAX;

  if (tspatial_type(temp1->temptype))
  {
    if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
        ! ensure_same_spatial_dimensionality(temp1->flags, temp2->flags))
    return INT_MAX;
  }

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
inline bool
temporal_lt(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_cmp(temp1, temp2) < 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_le()
 */
inline bool
temporal_le(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_cmp(temp1, temp2) <= 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_gt()
 */
inline bool
temporal_ge(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_cmp(temp1, temp2) >= 0;
}

/**
 * @ingroup meos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Temporal_ge()
 */
inline bool
temporal_gt(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_cmp(temp1, temp2) > 0;
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

/**
 * @ingroup meos_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal value
 * @param[in] temp Temporal value
 * @return On error return @p INT_MAX
 * @csqlfn #Temporal_hash()
 */
uint32
temporal_hash(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, INT_MAX);

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
