/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Basic functions for temporal types of any subtype.
 */

#include "general/temporal.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* POSTGRESQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_libmeos.h"
#include "general/lifting.h"
#include "general/pg_types.h"
#include "general/temporaltypes.h"
#include "general/temporal_boxops.h"
#include "general/tnumber_distance.h"
#include "general/temporal_tile.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_spatialfuncs.h"
#endif

/*****************************************************************************
 * Global variables
 *****************************************************************************/

#define MEOS_SUBTYPE_STR_MAXLEN 12

/**
 * @brief Array storing the string representation of the concrete subtypes of
 * temporal types
 */
static char *_tempsubtypeName[] =
{
  "Any subtype",
  "Instant",
  "Sequence",
  "SequenceSet"
};

/**
 * @brief Return the string representation of the subtype of the temporal type
 * corresponding to the enum value
 */
const char *
tempsubtype_name(int8 subtype)
{
  return _tempsubtypeName[subtype];
}

/**
 * @brief Global array containing the interpolation names corresponding to the
 * enumeration interpType defined in file meos_catalog.h.
 */
char * _interpType_names[] =
{
  [INTERP_NONE] = "none",
  [DISCRETE] = "discrete",
  [STEP] = "step",
  [LINEAR] = "linear"
};

#define INTERP_STR_MAX_LEN 8

/**
 * @brief Return the string representation of the subtype of the temporal type
 * corresponding to the enum value
 */
const char *
interptype_name(int8 interpType)
{
  return _interpType_names[interpType];
}

/**
 * @brief Get the interpolation type from the interpolation string
 */
interpType
interptype_from_string(const char *interp_str)
{
  int n = sizeof(_interpType_names) / sizeof(char *);
  for (int i = 0; i < n; i++)
  {
    if (pg_strncasecmp(interp_str, _interpType_names[i], INTERP_STR_MAX_LEN) == 0)
      return i;
  }
  /* Error */
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown interpolation type: %s", interp_str);
  return INTERP_NONE; /* make compiler quiet */
}

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
ensure_one_shift_width(bool hasshift, bool haswidth)
{
  if (! hasshift && ! haswidth)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG,
      "At least one of the arguments shift or width must be given");
    return false;
  }
  return true;
}

#if DEBUG_BUILD
/**
 * @brief Ensure that the subtype of a temporal value is valid
 * @note Used for the dispatch functions
 */
bool
temptype_subtype(int16 subtype)
{
  if (subtype == TINSTANT || subtype == TSEQUENCE || subtype == TSEQUENCESET)
    return true;
  return false;
}

/**
 * @brief Ensure that the subtype of a temporal value is valid
 * @note Used for the the analyze and selectivity functions
 */
bool
temptype_subtype_all(int16 subtype)
{
  if (subtype == ANYTEMPSUBTYPE ||
    subtype == TINSTANT || subtype == TSEQUENCE || subtype == TSEQUENCESET)
    return true;
  return false;
}
#endif /* DEBUG_BUILD */

#if 0 /* not used */
/**
 * @brief Ensure that a temporal value has discrete interpolation
 */
bool
ensure_discrete_interpolation(int16 flags)
{
  if (! MEOS_FLAGS_DISCRETE_INTERP(flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The temporal value must have discrete interpolation");
    return false;
  }
  return true;
}
#endif /* not used */

/**
 * @brief Ensure that the interpolation is valid
 * @note Used for the constructor functions
 */
bool
ensure_valid_interpolation(meosType temptype, interpType interp)
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
 * @param[in] temp1,temp2 Input values
 */
bool
ensure_same_interpolation(const Temporal *temp1, const Temporal *temp2)
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
 * @brief Ensure that the temporal values have the same continuous interpolation
 */
bool
ensure_same_continuous_interpolation(int16 flags1, int16 flags2)
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
 * @brief Ensure that a temporal value does not have linear interpolation
 */
bool
ensure_nonlinear_interpolation(int16 flags)
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
 * @brief Ensure that a temporal value is of a temporal type
 */
bool
ensure_temporal_has_type(const Temporal *temp, meosType temptype)
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
 * @param[in] temp1,temp2 Input values
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
ensure_temporal_has_subtype(const Temporal *temp, uint8 subtype)
{
  if (temp->subtype != subtype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
      "The temporal value must be of subtype %s", tempsubtype_name(subtype));
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
ensure_same_temporal_basetype(const Temporal *temp, meosType basetype)
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

/**
 * @brief Ensure that a temporal number and a temporal box have the same span type
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
  assert(span_basetype(basetype));
  if (basetype == T_INT4 && DatumGetInt32(size) <= 0)
    return false;
  else if (basetype == T_FLOAT8 && DatumGetFloat8(size) <= 0.0)
    return false;
  /* basetype == T_TIMESTAMPTZ */
  else if (DatumGetInt64(size) <= 0)
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
    else /* basetype == T_TIMESTAMPTZ */
      sprintf(str, INT64_FORMAT, DatumGetInt64(size));
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
 * @note Return NULL for temporal instants since they do not have bounding box.
 */
void *
temporal_bbox_ptr(const Temporal *temp)
{
  void *result = NULL;
  /* Values of TINSTANT subtype have not bounding box */
  if (temp->subtype == TSEQUENCE)
    result = TSEQUENCE_BBOX_PTR((TSequence *) temp);
  else if (temp->subtype == TSEQUENCESET)
    result = TSEQUENCESET_BBOX_PTR((TSequenceSet *) temp);
  return result;
}

/**
 * @brief Temporally intersect the two temporal values
 * @param[in] temp1,temp2 Input values
 * @param[in] mode Either intersection or synchronization
 * @param[out] inter1,inter2 Output values
 * @result Return false if the values do not overlap on time
 */
bool
intersection_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  SyncMode mode, Temporal **inter1, Temporal **inter2)
{
  bool result = false;
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  if (temp1->subtype == TINSTANT)
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = intersection_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else /* temp2->subtype == TSEQUENCESET */
      result = intersection_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
  }
  else if (temp1->subtype == TSEQUENCE)
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TSEQUENCE)
    {
      bool disc1 = MEOS_FLAGS_DISCRETE_INTERP(temp1->flags);
      bool disc2 = MEOS_FLAGS_DISCRETE_INTERP(temp2->flags);
      if (disc1 && disc2)
        result = intersection_tdiscseq_tdiscseq(
            (TSequence *) temp1, (TSequence *) temp2,
            (TSequence **) inter1, (TSequence **) inter2);
      else if (disc1 && ! disc2)
        result = intersection_tdiscseq_tcontseq(
            (TSequence *) temp1, (TSequence *) temp2,
            (TSequence **) inter1, (TSequence **) inter2);
      else if (! disc1 && disc2)
        result = intersection_tcontseq_tdiscseq(
            (TSequence *) temp1, (TSequence *) temp2,
            (TSequence **) inter1, (TSequence **) inter2);
      else /* !disc1 && ! disc2 */
        result = synchronize_tsequence_tsequence(
            (TSequence *) temp1, (TSequence *) temp2,
            (TSequence **) inter1, (TSequence **) inter2,
              mode == SYNCHRONIZE_CROSS);
    }
    else /* temp2->subtype == TSEQUENCESET */
    {
      result = MEOS_FLAGS_DISCRETE_INTERP(temp1->flags) ?
        intersection_tdiscseq_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2,
          (TSequence **) inter1, (TSequence **) inter2) :
        intersection_tsequence_tsequenceset(
            (TSequence *) temp1, (TSequenceSet *) temp2, mode,
            (TSequenceSet **) inter1, (TSequenceSet **) inter2);
    }
  }
  else /* temp1->subtype == TSEQUENCESET */
  {
    if (temp2->subtype == TINSTANT)
      result = intersection_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2,
        (TInstant **) inter1, (TInstant **) inter2);
    else if (temp2->subtype == TSEQUENCE)
      result = MEOS_FLAGS_DISCRETE_INTERP(temp2->flags) ?
        intersection_tsequenceset_tdiscseq(
          (TSequenceSet *) temp1, (TSequence *) temp2,
          (TSequence **) inter1, (TSequence **) inter2) :
        synchronize_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, mode,
          (TSequenceSet **) inter1, (TSequenceSet **) inter2);
    else /* temp2->subtype == TSEQUENCESET */
      result = synchronize_tsequenceset_tsequenceset(
        (TSequenceSet *) temp1, (TSequenceSet *) temp2, mode,
        (TSequenceSet **) inter1, (TSequenceSet **) inter2);
  }
  return result;
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

#define MOBDB_VERSION_STR_MAXLEN 256
/**
 * @brief Version of the MobilityDB extension
 */
char *
mobilitydb_version(void)
{
  char *result = MOBILITYDB_VERSION_STRING;
  return result;
}

/**
 * @brief Versions of the MobilityDB extension and its dependencies
 */
char *
mobilitydb_full_version(void)
{
  const char *proj_ver;
#if POSTGIS_PROJ_VERSION < 61
  proj_ver = pj_get_release();
#else
  PJ_INFO pji = proj_info();
  proj_ver = pji.version;
#endif
  const char* geos_version = GEOSversion();

  char *result = palloc(sizeof(char) * MOBDB_VERSION_STR_MAXLEN);
  int len = snprintf(result, MOBDB_VERSION_STR_MAXLEN,
    "%s, %s, %s, GEOS %s, PROJ %s",
    MOBILITYDB_VERSION_STRING, POSTGRESQL_VERSION_STRING,
    POSTGIS_VERSION_STRING, geos_version, proj_ver);
  result[len] = '\0';
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return a temporal value from its Well-Known Text (WKT) representation.
 *
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
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation.
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
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal integer from its Well-Known Text (WKT)
 * representation.
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
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation.
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
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation.
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
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal value.
 */
char *
temporal_out(const Temporal *temp, int maxdd)
{
  assert(temp);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_out((TInstant *) temp, maxdd);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_out((TSequence *) temp, maxdd);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_out((TSequenceSet *) temp, maxdd);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal boolean from its Well-Known Text (WKT)
 * representation.
 */
char *
tbool_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal integer from its Well-Known Text (WKT)
 * representation.
 */
char *
tint_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation.
 */
char *
tfloat_out(const Temporal *temp, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return NULL;
  return temporal_out(temp, maxdd);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation.
 */
char *
ttext_out(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;
  return temporal_out(temp, 0);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal geometric/geographic point from its Well-Known Text
 * (WKT) representation.
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
 * @ingroup libmeos_temporal_constructor
 * @brief Return a copy of a temporal value
 */
Temporal *
temporal_copy(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  Temporal *result = palloc(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
 */
static TSequence *
tdiscseq_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  assert(seq);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tinstant_make(value, temptype, (TSEQUENCE_INST_N(seq, i))->t);
  return tsequence_make_free(instants, seq->count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set.
 */
TSequence *
tsequence_from_base_temp(Datum value, meosType temptype, const TSequence *seq)
{
  assert(seq);
  TSequence *result = MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
    tdiscseq_from_base_temp(value, temptype, seq) :
    tsequence_from_base_tstzspan(value, temptype, &seq->period,
      temptype_continuous(temptype) ? LINEAR : STEP);
  return result;
}

/**
 * @brief Construct a temporal sequence set from a base value and the time
 * frame of another temporal sequence set. The interpolation of the result is
 * step or linear depending on whether the base type is continous or not.
 * @param[in] value Base value
 * @param[in] temptype Temporal type
 * @param[in] ss Sequence set
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
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal value from a base value and the time frame of
 * another temporal value. For continuous sequence (sets), the resulting
 * interpolation will be linear or step depending on whether the temporal type
 * is, respectively, continuous or not.
 */
Temporal *
temporal_from_base_temp(Datum value, meosType temptype, const Temporal *temp)
{
  assert(temp);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_make(value, temptype,
      ((TInstant *) temp)->t);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_from_base_temp(value, temptype,
      (TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_from_base_temp(value, temptype,
      (TSequenceSet *) temp);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal boolean from a boolean and the time frame of
 * another temporal value.
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
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal integer from an integer and the time frame of
 * another temporal value.
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
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal float from a float and the time frame of
 * another temporal value.
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
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal text from a text and the time frame of
 * another temporal value.
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
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point from a point and the time frame
 * of another temporal value.
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
 * Append and merge functions
 ****************************************************************************/

/**
 * @ingroup libmeos_temporal_modif
 * @brief Append an instant to a temporal value.
 * @param[in,out] temp Temporal value
 * @param[in] inst Temporal instant
 * @param[in] maxdist Maximum distance for defining a gap
 * @param[in] maxt Maximum time interval for defining a gap
 * @param[in] expand True when reserving space for additional instants
 * @sqlfunc appendInstantGaps
 */
Temporal *
temporal_append_tinstant(Temporal *temp, const TInstant *inst, double maxdist,
  Interval *maxt, bool expand)
{
  /* Validity tests */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) inst) ||
      ! ensure_same_temporal_type(temp, (Temporal *) inst) ||
      ! ensure_temporal_has_subtype((Temporal *) inst, TINSTANT) ||
      ! ensure_spatial_validity(temp, (const Temporal *) inst))
    return NULL;

  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_merge((const TInstant *) temp, inst);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_append_tinstant((TSequence *) temp,
      inst, maxdist, maxt, expand);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_append_tinstant((TSequenceSet *) temp,
      inst, maxdist, maxt, expand);
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Append a sequence to a temporal value.
 * @param[in,out] temp Temporal value
 * @param[in] seq Temporal sequence
 * @param[in] expand True when reserving space for additional sequences
 * @sqlfunc appendSequence
 */
Temporal *
temporal_append_tsequence(Temporal *temp, const TSequence *seq, bool expand)
{
  /* Validity tests */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) seq) ||
      ! ensure_same_temporal_type(temp, (Temporal *) seq) ||
      ! ensure_temporal_has_subtype((Temporal *) seq, TSEQUENCE) ||
      ! ensure_same_interpolation(temp, (Temporal *) seq) ||
      ! ensure_spatial_validity(temp, (Temporal *) seq))
    return NULL;

  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */

  interpType interp2 = MEOS_FLAGS_GET_INTERP(seq->flags);
  Temporal *result;
  if (temp->subtype == TINSTANT)
  {
    TSequence *seq1 = tinstant_to_tsequence((TInstant *) temp, interp2);
    result = (Temporal *) tsequence_append_tsequence((TSequence *) seq1,
      (TSequence *) seq, expand);
    pfree(seq1);
  }
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_append_tsequence((TSequence *) temp, seq,
      expand);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_append_tsequence((TSequenceSet *) temp,
      seq, expand);
  return result;
}

/**
 * @brief Convert two temporal values into a common subtype
 * @param[in] temp1,temp2 Input values
 * @param[out] out1,out2 Output values
 * @note Each of the output values may be equal to the input values to avoid
 * unnecessary calls to palloc. The calling function must test whether
 * (tempx == outx) to determine if a pfree is needed.
 */
static void
temporal_convert_same_subtype(const Temporal *temp1, const Temporal *temp2,
  Temporal **out1, Temporal **out2)
{
  assert(temptype_subtype(temp1->subtype));
  assert(temp1->temptype == temp2->temptype);

  /* If both are of the same subtype do nothing */
  if (temp1->subtype == temp2->subtype)
  {
    interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
    interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
    if (interp1 == interp2)
    {
      *out1 = (Temporal *) temp1;
      *out2 = (Temporal *) temp2;
    }
    else
    {
      interpType interp = Max(interp1, interp2);
      *out1 = temporal_to_tsequenceset(temp1, interp);
      *out2 = temporal_to_tsequenceset(temp2, interp);
    }
    return;
  }

  /* Different subtype */
  bool swap = false;
  const Temporal *new1, *new2;
  if (temp1->subtype > temp2->subtype)
  {
    new1 = temp2;
    new2 = temp1;
    swap = true;
  }
  else
  {
    new1 = temp1;
    new2 = temp2;
  }

  Temporal *new;
  if (new1->subtype == TINSTANT)
  {
    interpType interp = MEOS_FLAGS_GET_INTERP(new2->flags);
    if (new2->subtype == TSEQUENCE)
      new = (Temporal *) tinstant_to_tsequence((TInstant *) new1, interp);
    else /* new2->subtype == TSEQUENCESET */
      new = (Temporal *) tinstant_to_tsequenceset((TInstant *) new1, interp);
  }
  else /* new1->subtype == TSEQUENCE && new2->subtype == TSEQUENCESET */
    new = (Temporal *) tsequence_to_tsequenceset((TSequence *) new1);
  if (swap)
  {
    *out1 = (Temporal *) temp1;
    *out2 = new;
  }
  else
  {
    *out1 = new;
    *out2 = (Temporal *) temp2;
  }
  return;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Merge two temporal values.
 * @result Merged value. Return NULL if both arguments are NULL.
 * If one argument is null the other argument is output.
 * @sqlfunc merge
 */
Temporal *
temporal_merge(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *result;
  /* Cannot do anything with null inputs */
  if (! temp1 && ! temp2)
    return NULL;
  /* One argument is null, return a copy of the other temporal */
  if (! temp1)
    return temporal_copy(temp2);
  if (! temp2)
    return temporal_copy(temp1);

  /* Ensure validity of the arguments */
  if (! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_continuous_interpolation(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  /* Convert to the same subtype */
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  assert(temptype_subtype(new1->subtype));
  if (new1->subtype == TINSTANT)
    result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
  else if (new1->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_merge((TSequence *) new1,
      (TSequence *) new2);
  else /* new1->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_merge((TSequenceSet *) new1,
      (TSequenceSet *) new2);
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  return result;
}

/**
 * @brief Convert the array of temporal values into a common subtype
 * @param[in] temparr Array of values
 * @param[in] count Number of values
 * @param[in] subtype common subtype
 * @param[in] interp Interpolation
 * @result  Array of output values
 */
static Temporal **
temporalarr_convert_subtype(Temporal **temparr, int count, uint8 subtype,
  interpType interp)
{
  assert(temparr);
  assert(temptype_subtype(subtype));
  Temporal **result = palloc(sizeof(Temporal *) * count);
  for (int i = 0; i < count; i++)
  {
    uint8 subtype1 = temparr[i]->subtype;
    assert(subtype >= subtype1);
    if (subtype == subtype1)
      result[i] = temporal_copy(temparr[i]);
    else if (subtype1 == TINSTANT)
    {
      if (subtype == TSEQUENCE)
        result[i] = (Temporal *) tinstant_to_tsequence((TInstant *) temparr[i],
          interp);
      else /* subtype == TSEQUENCESET */
        result[i] = (Temporal *) tinstant_to_tsequenceset((TInstant *) temparr[i],
          interp);
    }
    else /* subtype1 == TSEQUENCE && subtype == TSEQUENCESET */
      result[i] = (Temporal *) tsequence_to_tsequenceset((TSequence *) temparr[i]);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Merge an array of temporal values.
 * @sqlfunc merge
 */
Temporal *
temporal_merge_array(Temporal **temparr, int count)
{
  /* Ensure validity of the arguments */
  if( ! ensure_not_null((void *) temparr) || ! ensure_positive(count))
    return NULL;

  if (count == 1)
    return temporal_copy(temparr[0]);

  /* Ensure all values have the same interpolation and, if they are spatial,
   * have the same SRID and dimensionality, and determine subtype of the
   * result */
  uint8 subtype, origsubtype;
  subtype = origsubtype = temparr[0]->subtype;
  interpType interp = MEOS_FLAGS_GET_INTERP(temparr[0]->flags);
  bool spatial = tgeo_type(temparr[0]->temptype);
  bool convert = false;
  for (int i = 1; i < count; i++)
  {
    uint8 subtype1 = temparr[i]->subtype;
    interpType interp1 = MEOS_FLAGS_GET_INTERP(temparr[i]->flags);
    if (subtype != subtype1 || interp != interp1)
    {
      convert = true;
      uint8 newsubtype = Max(subtype, subtype1);
      interpType newinterp = Max(interp, interp1);
      /* A discrete TSequence cannot be converted to a continuous TSequence */
      if (subtype == TSEQUENCE && subtype1 == TSEQUENCE && interp != newinterp)
        newsubtype = TSEQUENCESET;
      subtype = newsubtype;
      interp |= newinterp;
    }
    if (spatial && ! ensure_spatial_validity(temparr[0], temparr[i]))
      return NULL;
  }
  /* Convert all temporal values to a single subtype if needed */
  Temporal **newtemps;
  if (convert)
    newtemps = temporalarr_convert_subtype(temparr, count, subtype, interp);
  else
    newtemps = temparr;

  Temporal *result;
  assert(temptype_subtype(subtype));
  if (subtype == TINSTANT)
    result = (Temporal *) tinstant_merge_array(
      (const TInstant **) newtemps, count);
  else if (subtype == TSEQUENCE)
    result = (Temporal *) tsequence_merge_array(
      (const TSequence **) newtemps, count);
  else /* subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_merge_array(
      (const TSequenceSet **) newtemps, count);
  if (subtype != origsubtype)
    pfree_array((void **) newtemps, count);
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Convert an integer to a float
 */
static Datum
datum_int_to_float(Datum d)
{
  return Float8GetDatum((double) DatumGetInt32(d));
}

/**
 * @brief Convert a float to an integer
 */
static Datum
datum_float_to_int(Datum d)
{
  return Int32GetDatum((int) DatumGetFloat8(d));
}

/**
 * @ingroup libmeos_temporal_conversion
 * @brief Convert a temporal integer to a temporal float.
 * @sqlop @p ::
 */
Temporal *
tint_to_tfloat(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
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
 * @ingroup libmeos_temporal_conversion
 * @brief Convert a temporal float to a temporal integer.
 * @sqlop @p ::
 */
Temporal *
tfloat_to_tint(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
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
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding period of a temporal value.
 */
void
temporal_set_tstzspan(const Temporal *temp, Span *s)
{
  assert(temp); assert(s);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    tinstant_set_tstzspan((TInstant *) temp, s);
  else if (temp->subtype == TSEQUENCE)
    tsequence_set_tstzspan((TSequence *) temp, s);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_set_tstzspan((TSequenceSet *) temp, s);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_conversion
 * @brief Return the bounding period of a temporal value.
 * @sqlfunc period
 * @sqlop @p ::
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
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding value span of a temporal number.
 */
void
tnumber_set_span(const Temporal *temp, Span *s)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  assert(temptype_subtype(temp->subtype));
  meosType basetype = temptype_basetype(temp->temptype);
  if (temp->subtype == TINSTANT)
  {
    Datum value = tinstant_value((TInstant *) temp);
    span_set(value, value, true, true, basetype, s);
  }
  else
  {
    TBox *box = (TBox *) temporal_bbox_ptr(temp);
    memcpy(s, &box->span, sizeof(Span));
  }
  return;
}

/**
 * @ingroup libmeos_temporal_conversion
 * @brief Return the value span of a temporal number.
 * @sqlfunc valueSpan
 */
Span *
tnumber_to_span(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tnumber_type(temp->temptype))
    return NULL;

  Span *result = palloc(sizeof(Span));
  tnumber_set_span(temp, result);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_box_conversion
 * @brief Return the bounding box of a temporal number.
 * @sqlop @p ::
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

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_transf
 * @brief Restart a temporal sequence (set) by keeping only the last n instants
 * or sequences.
 */
void
temporal_restart(Temporal *temp, int count)
{
  assert(temp);
  assert(count > 0);
  assert(temptype_subtype(temp->subtype));
  assert(temp->subtype != TINSTANT);

  if (temp->subtype == TSEQUENCE)
    tsequence_restart((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_restart((TSequenceSet *) temp, count);
  return;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal instant.
 * @sqlfunc tbool_inst, tint_inst, tfloat_inst, ttext_inst, etc.
 */
Temporal *
temporal_to_tinstant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tinstant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_to_tinstant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence
 * @sqlfunc tbool_seq, tint_seq, tfloat_seq, ttext_seq, etc.
 */
Temporal *
temporal_to_tsequence(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interpolation(temp->temptype, interp))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequence((TInstant *) temp, interp);
  else if (temp->subtype == TSEQUENCE)
  {
    interpType interp1 = MEOS_FLAGS_GET_INTERP(temp->flags);
    if (interp1 == DISCRETE && interp != DISCRETE && 
      ((TSequence *) temp)->count > 1)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Cannot transform input value to a temporal sequence with %s interpolation",
        interptype_name(interp));
      return NULL;
    }
    result = (Temporal *) tsequence_set_interp((TSequence *) temp, interp);
  }
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_to_tsequence((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed into a temporal sequence set.
 * @sqlfunc tbool_seqset, tint_seqset, tfloat_seqset, ttext_seqset, etc.
 */
Temporal *
temporal_to_tsequenceset(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interpolation(temp->temptype, interp))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequenceset((TInstant *) temp, interp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tsequenceset_interp((TSequence *) temp,
      interp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_set_interp((TSequenceSet *) temp,
      interp);
  return result;
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value transformed to the given interpolation.
 * @sqlfunc setInterp
 */
Temporal *
temporal_set_interp(const Temporal *temp, interpType interp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_valid_interpolation(temp->temptype, interp))
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
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return a temporal number with the value dimension shifted and/or
 * scaled by the values.
 * @param[in] temp Temporal value
 * @param[in] shift Value for shift
 * @param[in] width Value for scale
 * @param[in] hasshift, haswidth True when the corresponding argument was given
 * @sqlfunc shiftValue(), scaleValue(), shiftScaleValue()
 */
Temporal *
tnumber_shift_scale_value(const Temporal *temp, Datum shift, Datum width,
  bool hasshift, bool haswidth)
{
  assert(temp);
  meosType basetype = temptype_basetype(temp->temptype);
  /* Ensure validity of the arguments */
  if (! ensure_one_shift_width(hasshift, haswidth) ||
      (width && ! ensure_positive_datum(width, basetype)))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (hasshift) ?
      (Temporal *) tnuminst_shift_value((TInstant *) temp, shift) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tnumberseq_shift_scale_value((TSequence *) temp,
      shift, width, hasshift, haswidth);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_shift_scale_value((TSequenceSet *) temp,
      shift, width, hasshift, haswidth);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value.
 */
Temporal *
tint_shift_value(const Temporal *temp, int shift)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return NULL;

  return tnumber_shift_scale_value(temp, Int32GetDatum(shift), 0, true, false);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal integer whose value dimension is shifted by a value.
 */
Temporal *
tfloat_shift_value(const Temporal *temp, double shift)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return NULL;

  return tnumber_shift_scale_value(temp, Float8GetDatum(shift), 0, true, false);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value.
 */
Temporal *
tint_scale_value(const Temporal *temp, int width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return NULL;

  return tnumber_shift_scale_value(temp, 0, Int32GetDatum(width), false, true);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value.
 */
Temporal *
tfloat_scale_value(const Temporal *temp, double width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return NULL;

  return tnumber_shift_scale_value(temp, 0, Float8GetDatum(width), false, true);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value.
 */
Temporal *
tint_shift_scale_value(const Temporal *temp, int shift, int width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return NULL;

  return tnumber_shift_scale_value(temp, Int32GetDatum(shift),
    Int32GetDatum(width), true, true);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal number whose value dimension is scaled by a value.
 */
Temporal *
tfloat_shift_scale_value(const Temporal *temp, double shift, double width)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return NULL;

  return tnumber_shift_scale_value(temp, Float8GetDatum(shift),
    Float8GetDatum(width), true, true);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value shifted and/or scaled by the intervals.
 * @param[in] temp Temporal value
 * @param[in] shift Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 * @sqlfunc shiftTime(), scaleTime(), shiftScaleTime()
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

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (shift != NULL) ?
      (Temporal *) tinstant_shift_time((TInstant *) temp, shift) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_shift_scale_time((TSequence *) temp,
      shift, duration);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_shift_scale_time((TSequenceSet *) temp,
      shift, duration);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value shifted by the interval.
 */
Temporal *
temporal_shift_time(const Temporal *temp, const Interval *shift)
{
  return temporal_shift_scale_time(temp, shift, NULL);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value scaled by the interval.
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

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the size in bytes of a temporal value
 * @sqlfunc memSize
 */
size_t
temporal_mem_size(const Temporal *temp)
{
  assert(temp);
  return VARSIZE(temp);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value.
 * @sqlfunc tempSubtype;
 */
char *
temporal_subtype(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  char *result = palloc(sizeof(char) * MEOS_SUBTYPE_STR_MAXLEN);
  assert(temptype_subtype(temp->subtype));
  strcpy(result, _tempsubtypeName[temp->subtype]);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the interpolation of a temporal
 * value.
 * @sqlfunc interp
 */
char *
temporal_interp(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  char *result = palloc(sizeof(char) * MEOS_INTERP_STR_MAXLEN);
  assert(temptype_subtype(temp->subtype));
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (temp->subtype == TINSTANT)
    strcpy(result, "None");
  else if (interp == DISCRETE)
    strcpy(result, "Discrete");
  else if (interp == STEP)
    strcpy(result, "Step");
  else
    strcpy(result, "Linear");
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding box of a temporal value
 * @note For temporal instants the bounding box must be computed. For the
 * other subtypes, a copy of the precomputed bounding box is made.
 * @sqlfunc period, tbox, stbox
 * @sqlop @p ::
 */
void
temporal_set_bbox(const Temporal *temp, void *box)
{
  assert(temp); assert(box);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    tinstant_set_bbox((TInstant *) temp, box);
  else if (temp->subtype == TSEQUENCE)
    tsequence_set_bbox((TSequence *) temp, box);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_set_bbox((TSequenceSet *) temp, box);
  return;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the array of distinct base values of a temporal value.
 * @sqlfunc values
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
  assert(temp); assert(count);

  Datum *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_values((TInstant *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_values((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_values((TSequenceSet *) temp, count);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of a temporal boolean
 * @sqlfunc values
 */
bool *
tbool_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return NULL;

  Datum *datumarr = temporal_values(temp, count);
  bool *result = palloc(sizeof(bool) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetBool(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of a temporal integer
 * @sqlfunc values
 */
int *
tint_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return NULL;

  Datum *datumarr = temporal_values(temp, count);
  int *result = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetInt32(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of a temporal float
 * @sqlfunc values
 */
double *
tfloat_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return NULL;

  Datum *datumarr = temporal_values(temp, count);
  double *result = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetFloat8(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of a temporal text
 * @sqlfunc values
 */
text **
ttext_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;

  Datum *datumarr = temporal_values(temp, count);
  text **result = palloc(sizeof(text *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetTextP(datumarr[i]);
  pfree(datumarr);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of base values of a temporal geometric point
 * @sqlfunc values
 */
GSERIALIZED **
tpoint_values(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_tgeo_type(temp->temptype))
    return NULL;

  Datum *datumarr = temporal_values(temp, count);
  GSERIALIZED **result = palloc(sizeof(GSERIALIZED *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = DatumGetGserializedP(datumarr[i]);
  pfree(datumarr);
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the base values of a temporal number as a span set.
 * @sqlfunc getValues
 */
SpanSet *
tnumber_valuespans(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_tnumber_type(temp->temptype))
    return NULL;

  SpanSet *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tnumberinst_valuespans((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tnumberseq_valuespans((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnumberseqset_valuespans((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the time frame of a temporal value as a period set.
 * @sqlfunc getTime
 */
SpanSet *
temporal_time(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  SpanSet *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_time((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_time((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_time((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the start base value of a temporal value
 * @sqlfunc startValue
 */
Datum
temporal_start_value(const Temporal *temp)
{
  assert(temp);
  Datum result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tinstant_value_copy(TSEQUENCE_INST_N((TSequence *) temp, 0));
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
    result = tinstant_value_copy(TSEQUENCE_INST_N(seq, 0));
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal boolean
 * @sqlfunc startValue
 */
bool
tbool_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return false;
  return DatumGetBool(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal integer
 * @return On error return INT_MAX
 * @sqlfunc startValue
 */
int
tint_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @return On error return DBL_MAX
 * @sqlfunc startValue
 */
double
tfloat_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal text
 * @return On error return NULL
 * @sqlfunc startValue
 */
text *
ttext_start_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal geometric point
 * @return On error return NULL
 * @sqlfunc startValue
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
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return the end base value of a temporal value
 */
Datum
temporal_end_value(const Temporal *temp)
{
  assert(temp);
  Datum result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tinstant_value_copy(TSEQUENCE_INST_N((TSequence *) temp,
      ((TSequence *) temp)->count - 1));
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = tinstant_value_copy(TSEQUENCE_INST_N(seq, seq->count - 1));
  }
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal boolean
 * @sqlfunc endValue
 */
bool
tbool_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return false;
  return DatumGetBool(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal integer
 * @return On error return INT_MAX
 * @sqlfunc endValue
 */
int
tint_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal float
 * @return On error return DBL_MAX
 * @sqlfunc endValue
 */
double
tfloat_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal text
 * @return On error return NULL
 * @sqlfunc endValue
 */
text *
ttext_end_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal point
 * @return On error return NULL
 * @sqlfunc endValue
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
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a copy of the minimum base value of a temporal value
 */
Datum
temporal_min_value(const Temporal *temp)
{
  assert(temp);
  Datum result;
  meosType basetype = temptype_basetype(temp->temptype);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = datum_copy(tsequence_min_value((TSequence *) temp), basetype);
  else /* temp->subtype == TSEQUENCESET */
    result = datum_copy(tsequenceset_min_value((TSequenceSet *) temp), basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal integer
 * @return On error return INT_MAX
 * @sqlfunc minValue
 */
int
tint_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal float
 * @return On error return DBL_MAX
 * @sqlfunc minValue
 */
double
tfloat_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal text
 * @return On error return NULL
 * @sqlfunc minValue
 */
text *
ttext_min_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_min_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a copy of the maximum base value of a temporal value.
 * @sqlfunc maxValue
 */
Datum
temporal_max_value(const Temporal *temp)
{
  assert(temp);
  Datum result;
  meosType basetype = temptype_basetype(temp->temptype);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_value_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = datum_copy(tsequence_max_value((TSequence *) temp), basetype);
  else /* temp->subtype == TSEQUENCESET */
    result = datum_copy(tsequenceset_max_value((TSequenceSet *) temp), basetype);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal integer
 * @return On error return INT_MAX
 * @sqlfunc maxValue
 */
int
tint_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return INT_MAX;
  return DatumGetInt32(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal float
 * @return On error return DBL_MAX
 * @sqlfunc maxValue
 */
double
tfloat_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return DBL_MAX;
  return DatumGetFloat8(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal text
 * @return On error return NULL
 * @sqlfunc maxValue
 */
text *
ttext_max_value(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return NULL;
  return DatumGetTextP(temporal_max_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal value.
 * @return On error return NULL
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @sqlfunc minInstant
 */
const TInstant *
temporal_min_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  const TInstant *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_min_instant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_min_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of a
 * temporal value.
 * @return On error return NULL
 * @sqlfunc maxInstant
 */
const TInstant *
temporal_max_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  const TInstant *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_max_instant((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_max_instant((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the duration of a temporal value.
 * @return On error return NULL
 * @sqlfunc duration
 */
Interval *
temporal_duration(const Temporal *temp, bool boundspan)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  Interval *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT ||
      (MEOS_FLAGS_DISCRETE_INTERP(temp->flags) && ! boundspan))
    result = palloc0(sizeof(Interval));
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_duration((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_duration((TSequenceSet *) temp, boundspan);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of sequences of a temporal sequence (set).
 * @return On error return -1
 * @sqlfunc numSequences
 */
int
temporal_num_sequences(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return -1;

  int result = 1;
  if (temp->subtype == TSEQUENCESET)
    result = ((TSequenceSet *) temp)->count;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start sequence of a temporal sequence (set).
 * @return On error return NULL
 * @sqlfunc startSequence
 */
TSequence *
temporal_start_sequence(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  TSequence *result;
  if (temp->subtype == TSEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    result = tsequence_copy(TSEQUENCESET_SEQ_N(ss, 0));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end sequence of a temporal sequence (set).
 * @return On error return NULL
 * @sqlfunc endSequence
 */
TSequence *
temporal_end_sequence(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  TSequence *result;
  if (temp->subtype == TSEQUENCE)
    result = tsequence_copy((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    result = tsequence_copy(TSEQUENCESET_SEQ_N(ss, ss->count - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th sequence of a temporal sequence (set).
 * @return On error return NULL
 * @note n is assumed to be 1-based.
 * @sqlfunc sequenceN
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_continuous(temp))
    return NULL;

  TSequence *result = NULL;
  if (temp->subtype == TSEQUENCE)
  {
    if (i == 1)
      result = tsequence_copy((TSequence *) temp);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    if (i >= 1 && i <= ss->count)
      result = tsequence_copy(TSEQUENCESET_SEQ_N(ss, i - 1));
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of sequences of a temporal sequence (set).
 * @return On error return NULL
 * @sqlfunc sequences
 */
TSequence **
temporal_sequences(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_continuous(temp))
    return NULL;

  TSequence **result;
  if (temp->subtype == TSEQUENCE)
  {
    result = tsequence_sequences((TSequence *) temp, count);
    *count = 1;
  }
  else /* temp->subtype == TSEQUENCE */
  {
    result = tsequenceset_sequences((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->count;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of segments of a temporal value.
 * @return On error return NULL
 * @sqlfunc segments
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

  TSequence **result;
  if (temp->subtype == TSEQUENCE)
    result = tsequence_segments((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_segments((TSequenceSet *) temp, count);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct instants of a temporal value.
 * @return On error return -1
 * @sqlfunc numInstants
 */
int
temporal_num_instants(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = 1;
  else if (temp->subtype == TSEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_num_instants((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start instant of a temporal value.
 * @return On error return NULL
 * @sqlfunc startInstant
 */
const TInstant *
temporal_start_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  const TInstant *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TSEQUENCE)
    result = TSEQUENCE_INST_N((TSequence *) temp, 0);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp, 0);
    result = TSEQUENCE_INST_N(seq, 0);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end instant of a temporal value.
 * @return On error return NULL
 * @note This function is used for validity testing.
 * @sqlfunc endInstant
 */
const TInstant *
temporal_end_instant(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  const TInstant *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (TInstant *) temp;
  else if (temp->subtype == TSEQUENCE)
    result = TSEQUENCE_INST_N((TSequence *) temp,
      ((TSequence *) temp)->count - 1);
  else /* temp->subtype == TSEQUENCESET */
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N((TSequenceSet *) temp,
      ((TSequenceSet *) temp)->count - 1);
    result = TSEQUENCE_INST_N(seq, seq->count - 1);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th instant of a temporal value.
 * @return On error return NULL
 * @note n is assumed 1-based
 * @sqlfunc instantN
 */
const TInstant *
temporal_instant_n(const Temporal *temp, int n)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  const TInstant *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    if (n == 1)
      result = (const TInstant *) temp;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
      result = TSEQUENCE_INST_N((TSequence *) temp, n - 1);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    /* This test is necessary since the n-th DISTINCT instant is requested */
    if (n >= 1 && n <= ((TSequenceSet *) temp)->totalcount)
      result = tsequenceset_inst_n((TSequenceSet *) temp, n);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of instants of a temporal value.
 * @return On error return NULL
 * @sqlfunc instants
 */
const TInstant **
temporal_instants(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count))
    return NULL;

  const TInstant **result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_instants((TInstant *) temp, count);
  else if (temp->subtype == TSEQUENCE)
  {
    result = tsequence_instants((TSequence *) temp);
    *count = ((TSequence *) temp)->count;
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = tsequenceset_instants((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->totalcount;
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the number of distinct timestamps of a temporal value.
 * @return On error return -1
 * @sqlfunc numTimestamps
 */
int
temporal_num_timestamps(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return -1;

  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = 1;
  else if (temp->subtype == TSEQUENCE)
    result = ((TSequence *) temp)->count;
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_num_timestamps((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start timestamp of a temporal value.
 * @return On error return DT_NOEND
 * @sqlfunc startTimestamp
 */
TimestampTz
temporal_start_timestamp(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return DT_NOEND;

  TimestampTz result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_start_timestamp((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_start_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end timestamp of a temporal value.
 * @return On error return DT_NOEND
 * @sqlfunc endTimestamp
 */
TimestampTz
temporal_end_timestamp(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return DT_NOEND;

  TimestampTz result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = ((TInstant *) temp)->t;
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_end_timestamp((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_end_timestamp((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the n-th distinct timestamp of a temporal value in the last
 * argument
 * @return On error return false
 * @note n is assumed 1-based
 * @sqlfunc timestampN
 */
bool
temporal_timestamp_n(const Temporal *temp, int n, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) result))
    return false;

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    if (n == 1)
    {
      *result = ((TInstant *) temp)->t;
      return true;
    }
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (n >= 1 && n <= ((TSequence *) temp)->count)
    {
      *result = (TSEQUENCE_INST_N((TSequence *) temp, n - 1))->t;
      return true;
    }
  }
  else /* temp->subtype == TSEQUENCESET */
    return tsequenceset_timestamp_n((TSequenceSet *) temp, n, result);
  return false;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct timestamps of a temporal value.
 * @return On error return NULL
 * @sqlfunc timestamps
 */
TimestampTz *
temporal_timestamps(const Temporal *temp, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count))
    return NULL;

  TimestampTz *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_timestamps((TInstant *) temp, count);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_timestamps((TSequence *) temp, count);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_timestamps((TSequenceSet *) temp, count);
  return result;
}

/*****************************************************************************
 * Ever/always functions
 * The functions assume that the temporal value and the datum value are of
 * the same basetype. Ever/always equal are valid for all temporal types
 * including temporal points. All the other comparisons are only valid for
 * temporal alphanumeric types.
 *****************************************************************************/

/**
 * @brief Return true if the bounding box of a temporal value is ever/always
 * equal to a base value
 * @param[in] temp Temporal value
 * @param[in] value Value to be found
 * @param[in] ever True when testing ever, false when testing always
 */
bool
temporal_bbox_ev_al_eq(const Temporal *temp, Datum value, bool ever)
{
  assert(temp);

  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBox box;
    temporal_set_bbox(temp, &box);
    Datum upper = box.span.basetype == T_INT4 ?
      Int32GetDatum(DatumGetInt32(box.span.upper) - 1) : box.span.upper;
    return (ever && contains_span_value(&box.span, value, box.span.basetype)) ||
      (!ever && datum_eq(box.span.lower, value, box.span.basetype) &&
      datum_eq(upper, value, box.span.basetype));
  }
  else if (tspatial_type(temp->temptype))
  {
    STBox box1, box2;
    temporal_set_bbox(temp, &box1);
    if (tgeo_type(temp->temptype))
      geo_set_stbox(DatumGetGserializedP(value), &box2);
#if NPOINT
    else if (temp->temptype == T_TNPOINT)
    {
      GSERIALIZED *geom = npoint_geom(DatumGetNpointP(value));
      geo_set_stbox(geom, &box2);
      pfree(geom);
    }
#endif
    return (ever && contains_stbox_stbox(&box1, &box2)) ||
      (!ever && same_stbox_stbox(&box1, &box2));
  }
  return true;
}

/**
 * @brief Return true if the bounding box of a temporal value is ever/always
 * less than or equal to the base value.
 *
 * The same test is used for both since the bounding box does not
 * distinguish between inclusive/exclusive bounds.
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] ever True when testing ever, false when testing always
 */
bool
temporal_bbox_ev_al_lt_le(const Temporal *temp, Datum value, bool ever)
{
  assert(temp);

  if (tnumber_type(temp->temptype))
  {
    TBox box;
    temporal_set_bbox(temp, &box);
    Datum upper = box.span.basetype == T_INT4 ?
      Int32GetDatum(DatumGetInt32(box.span.upper) - 1) : box.span.upper;
    if ((ever && datum_lt(value, box.span.lower, box.span.basetype)) ||
      (! ever && datum_lt(value, upper, box.span.basetype)))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever equal to a base value.
 */
bool
temporal_ever_eq(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_eq((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_ever_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal boolean is ever equal to a boolean.
 * @sqlop @p ?=
 */
bool
tbool_ever_eq(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return false;
  return temporal_ever_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever equal to an integer.
 * @sqlop @p ?=
 */
bool
tint_ever_eq(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_ever_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is ever equal to a float.
 * @sqlop @p ?=
 */
bool
tfloat_ever_eq(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_ever_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is ever equal to a text.
 * @sqlop @p ?=
 */
bool
ttext_ever_eq(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_ever_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always equal to a base value.
 */
bool
temporal_always_eq(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_always_eq((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_eq((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_eq((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal boolean is always equal to a boolean.
 * @sqlop @p %=
 */
bool tbool_always_eq(const Temporal *temp, bool b)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TBOOL))
    return false;
  return temporal_always_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is always equal to an integer.
 * @sqlop @p %=
 */
bool tint_always_eq(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_always_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is always equal to a float.
 * @sqlop @p %=
 */
bool tfloat_always_eq(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) &&
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_always_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is always equal to a text.
 * @sqlop @p %=
 */
bool ttext_always_eq(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_always_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than a base value.
 */
bool
temporal_ever_lt(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_lt((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_lt((TSequence *) temp, value);
  else /* subtype == TSEQUENCESET */
    result = tsequenceset_ever_lt((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than an integer.
 * @sqlop @p ?<
 */
bool tint_ever_lt(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_ever_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than a float.
 * @sqlop @p ?<
 */
bool tfloat_ever_lt(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_ever_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than a text.
 * @sqlop @p ?<
 */
bool ttext_ever_lt(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_ever_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than a base value.
 */
bool
temporal_always_lt(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_always_lt((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_lt((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_lt((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than an integer.
 * @sqlop @p %<
 */
bool
tint_always_lt(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_always_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than  a float.
 * @sqlop @p %<
 */
bool
tfloat_always_lt(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_always_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than  a text.
 * @sqlop @p %<
 */
bool
ttext_always_lt(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_always_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is ever less than or equal to a
 * base value.
 */
bool
temporal_ever_le(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_ever_le((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_ever_le((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_ever_le((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is ever less than or equal to an integer.
 * @sqlop @p ?<=
 */
bool
tint_ever_le(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_ever_le(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is ever less than or equal to a float.
 * @sqlop @p ?<=
 */
bool
tfloat_ever_le(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_ever_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is ever less than or equal to a text.
 * @sqlop @p ?<=
 */
bool
ttext_ever_le(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_ever_le(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_comp_ever
 * @brief Return true if a temporal value is always less than or equal to a
 * base value.
 */
bool
temporal_always_le(const Temporal *temp, Datum value)
{
  assert(temp);
  bool result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_always_le((TInstant *) temp, value);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_always_le((TSequence *) temp, value);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_always_le((TSequenceSet *) temp, value);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal integer is always less than or equal to an integer.
 * @sqlop @p %<=
 */
bool
tint_always_le(const Temporal *temp, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TINT))
    return false;
  return temporal_always_le(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal float is always less than or equal to a float.
 * @sqlop @p %<=
 */
bool
tfloat_always_le(const Temporal *temp, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_has_type(temp, T_TFLOAT))
    return false;
  return temporal_always_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_comp_ever
 * @brief Return true if a temporal text is always less than or equal to a text.
 * @sqlop @p %<=
 */
bool
ttext_always_le(const Temporal *temp, text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) txt) ||
      ! ensure_temporal_has_type(temp, T_TTEXT))
    return false;
  return temporal_always_le(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/*****************************************************************************
 * Bounding box tests for the restriction functions
 *****************************************************************************/

/**
 * @brief Return true if the bounding box of a temporal value contains a base
 * value
 */
bool
temporal_bbox_restrict_value(const Temporal *temp, Datum value)
{
  assert(temp);

  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    Span span1, span2;
    tnumber_set_span(temp, &span1);
    value_set_span(value, temptype_basetype(temp->temptype), &span2);
    return contains_span_span(&span1, &span2);
  }
  if (tgeo_type(temp->temptype))
  {
    /* Test that the geometry is not empty */
    GSERIALIZED *gs = DatumGetGserializedP(value);
    assert(gserialized_get_type(gs) == POINTTYPE);
    assert(tpoint_srid(temp) == gserialized_get_srid(gs));
    assert(MEOS_FLAGS_GET_Z(temp->flags) == FLAGS_GET_Z(gs->gflags));
    if (gserialized_is_empty(gs))
      return false;
    if (temp->subtype != TINSTANT)
    {
      STBox box1, box2;
      temporal_set_bbox(temp, &box1);
      geo_set_stbox(gs, &box2);
      return contains_stbox_stbox(&box1, &box2);
    }
  }
  return true;
}

/**
 * @brief Return true if the bounding box of the temporal number overlaps the
 * span of base values
 */
bool
tnumber_bbox_restrict_span(const Temporal *temp, const Span *s)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  numspan_set_tbox(s, &box2);
  return overlaps_tbox_tbox(&box1, &box2);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a base value.
 * @note This function does a bounding box test for the temporal types
 * different from instant. The singleton tests are done in the functions for
 * the specific temporal types.
 * @sqlfunc atValue, minusValue
 */
Temporal *
temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc)
{
  assert(temp);
  /* Ensure validity of the arguments */
  if (tgeo_type(temp->temptype))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value);
    if (! ensure_point_type(gs) ||
        ! ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs)) ||
        ! ensure_same_dimensionality_tpoint_gs(temp, gs))
    return NULL;
  }

  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_value(temp, value))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE ||
          MEOS_FLAGS_DISCRETE_INTERP(temp->flags)) ?
        temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_value((TInstant *) temp, value,
      atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tdiscseq_restrict_value((TSequence *) temp, value, atfunc) :
      (Temporal *) tcontseq_restrict_value((TSequence *) temp, value, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_value((TSequenceSet *) temp,
      value, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Return true if the bounding box of the temporal and the set overlap
 * values.
 */
bool
temporal_bbox_restrict_set(const Temporal *temp, const Set *s)
{
  assert(temp); assert(s);
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    Span span1, span2;
    tnumber_set_span(temp, &span1);
    set_set_span(s, &span2);
    return overlaps_span_span(&span1, &span2);
  }
  if (tgeo_type(temp->temptype) && temp->subtype != TINSTANT)
  {
    STBox box;
    temporal_set_bbox(temp, &box);
    return contains_stbox_stbox(&box, (STBox *) SET_BBOX_PTR(s));
  }
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) an array of base
 * values.
 * @sqlfunc atValues, minusValues
 */
Temporal *
temporal_restrict_values(const Temporal *temp, const Set *s, bool atfunc)
{
  assert(temp); assert(s);
  if (tgeo_type(temp->temptype))
  {
    assert(tpoint_srid(temp) == geoset_srid(s));
    assert(same_spatial_dimensionality(temp->flags, s->flags));
  }

  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_set(temp, s))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE) ? temporal_copy(temp) :
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  }

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_values((TInstant *) temp, s,
      atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tdiscseq_restrict_values((TSequence *) temp, s, atfunc) :
      (Temporal *) tcontseq_restrict_values((TSequence *) temp, s, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_values((TSequenceSet *) temp,
      s, atfunc);

  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span of base values.
 * @sqlfunc atSpan, minusSpan
 */
Temporal *
tnumber_restrict_span(const Temporal *temp, const Span *s, bool atfunc)
{
  assert(temp); assert(s);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! tnumber_bbox_restrict_span(temp, s))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype == TSEQUENCE && interp != DISCRETE) ?
        (Temporal *) tsequence_to_tsequenceset((TSequence *) temp) :
        temporal_copy(temp);
  }

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tnumberinst_restrict_span((TInstant *) temp,
      s, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tnumberdiscseq_restrict_span((TSequence *) temp, s,
        atfunc) :
      (Temporal *) tnumbercontseq_restrict_span((TSequence *) temp, s,
        atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_span((TSequenceSet *) temp, s,
      atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span set.
 * @sqlfunc atSpanset, minusSpanset
 */
Temporal *
tnumber_restrict_spanset(const Temporal *temp, const SpanSet *ss, bool atfunc)
{
  assert(temp); assert(ss);
  assert(tnumber_type(temp->temptype));
  /* Bounding box test */
  Span s;
  tnumber_set_span(temp, &s);
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! overlaps_span_span(&s, &ss->span))
  {
    if (atfunc)
      return NULL;
    else
    {
      if (temp->subtype == TSEQUENCE && interp != DISCRETE)
        return (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
      else
        return temporal_copy(temp);
    }
  }

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tnumberinst_restrict_spanset((TInstant *) temp, ss,
      atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tnumberdiscseq_restrict_spanset((TSequence *) temp, ss,
        atfunc) :
      (Temporal *) tnumbercontseq_restrict_spanset((TSequence *) temp, ss,
        atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_spanset((TSequenceSet *) temp,
      ss, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a minimum base value
 * @sqlfunc atMin, atMax, minusMin, minusMax
 */
Temporal *
temporal_restrict_minmax(const Temporal *temp, bool min, bool atfunc)
{
  assert(temp);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
      (Temporal *) tdiscseq_restrict_minmax((TSequence *) temp, min, atfunc) :
      (Temporal *) tcontseq_restrict_minmax((TSequence *) temp, min, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_minmax((TSequenceSet *) temp,
      min, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to a timestamp.
 * @sqlfunc atTime, minusTime
 */
Temporal *
temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc)
{
  assert(temp);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t, atfunc);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
      result = atfunc ?
        (Temporal *) tdiscseq_at_timestamp((TSequence *) temp, t) :
        (Temporal *) tdiscseq_minus_timestamp((TSequence *) temp, t);
    else
      result = atfunc ?
        (Temporal *) tcontseq_at_timestamp((TSequence *) temp, t) :
        (Temporal *) tcontseq_minus_timestamp((TSequence *) temp, t);
  }
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestamp((TSequenceSet *) temp,
      t, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Return the base value of a temporal value at the timestamp
 * @sqlfunc valueAtTimestamp
 */
bool
temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  assert(temp); assert(result);
  bool found = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    found = tinstant_value_at_timestamp((TInstant *) temp, t, result);
  else if (temp->subtype == TSEQUENCE)
    found = MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
      tdiscseq_value_at_timestamp((TSequence *) temp, t, result) :
      tsequence_value_at_timestamp((TSequence *) temp, t, strict, result);
  else /* subtype == TSEQUENCESET */
    found = tsequenceset_value_at_timestamp((TSequenceSet *) temp, t, strict,
      result);
  return found;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a timestamp set
 * @sqlfunc atTime, minusTime
 */
Temporal *
temporal_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc)
{
  assert(temp); assert(s);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzset(
      (TInstant *) temp, s, atfunc);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
      result = (Temporal *) tdiscseq_restrict_tstzset((TSequence *) temp,
        s, atfunc);
    else
      result = atfunc ?
        (Temporal *) tcontseq_at_tstzset((TSequence *) temp, s) :
        (Temporal *) tcontseq_minus_tstzset((TSequence *) temp, s);
  }
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_tstzset(
      (TSequenceSet *) temp, s, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period.
 * @sqlfunc atTime, minusTime
 */
Temporal *
temporal_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc)
{
  assert(temp); assert(s);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzspan(
      (TInstant *) temp, s, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_restrict_tstzspan((TSequence *) temp, s, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_tstzspan(
      (TSequenceSet *) temp, s, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period set.
 */
Temporal *
temporal_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool atfunc)
{
  assert(temp); assert(ss);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzspanset(
      (TInstant *) temp, ss, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_restrict_tstzspanset((TSequence *) temp, ss, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_tstzspanset(
      (TSequenceSet *) temp, ss, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal number to a temporal box.
 * @sqlfunc atTbox
 */
Temporal *
tnumber_at_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_tbox(temp, box))
    return NULL;

  /* Bounding box test */
  TBox box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return NULL;

  /* At least one of MEOS_FLAGS_GET_T and MEOS_FLAGS_GET_X is true */
  Temporal *temp1;
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  if (hast)
    /* Due the bounding box test above, temp1 is never NULL */
    temp1 = temporal_restrict_tstzspan(temp, &box->period, REST_AT);
  else
    temp1 = (Temporal *) temp;

  Temporal *result;
  if (hasx)
  {
    /* Ensure function is called for temporal numbers */
    assert(tnumber_type(temp->temptype));
    result = tnumber_restrict_span(temp1, &box->span, REST_AT);
  }
  else
    result = temp1;
  if (hasx && hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup libmeos_temporal_restrict
 * @brief Restrict a temporal number to the complement of a temporal box.
 *
 * It is not possible to make the difference from each dimension separately,
 * i.e., restrict at the period and then restrict to the span. Therefore, we
 * compute the `atTbox` and then compute the complement of the value obtained.
 * @sqlfunc minusTbox
 */
Temporal *
tnumber_minus_tbox(const Temporal *temp, const TBox *box)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_valid_tnumber_tbox(temp, box))
    return NULL;

  /* Bounding box test */
  TBox box1;
  temporal_set_bbox(temp, &box1);
  if (! overlaps_tbox_tbox(box, &box1))
    return temporal_copy(temp);

  Temporal *result = NULL;
  Temporal *temp1 = tnumber_at_tbox(temp, box);
  if (temp1 != NULL)
  {
    SpanSet *ps = temporal_time(temp1);
    result = temporal_restrict_tstzspanset(temp, ps, REST_MINUS);
    pfree(temp1); pfree(ps);
  }
  return result;
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_modif
 * @brief Insert the second temporal value into the first one.
 * @sqlfunc insert
 */
Temporal *
temporal_insert(const Temporal *temp1, const Temporal *temp2, bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_continuous_interpolation(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  /* Convert to the same subtype */
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  Temporal *result;
  assert(temptype_subtype(new1->subtype));
  if (new1->subtype == TINSTANT)
    result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
  else if (new1->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_insert((TSequence *) new1,
      (TSequence *) new2, connect);
  else /* new1->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_merge((TSequenceSet *) new1,
        (TSequenceSet *) new2) :
      (Temporal *) tsequenceset_insert((TSequenceSet *) new1,
        (TSequenceSet *) new2);
  }
  if (temp1 != new1)
    pfree(new1);
  if (temp2 != new2)
    pfree(new2);
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Update the first temporal value with the second one.
 * @sqlfunc update
 */
Temporal *
temporal_update(const Temporal *temp1, const Temporal *temp2, bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2) ||
      ! ensure_same_continuous_interpolation(temp1->flags, temp2->flags) ||
      ! ensure_spatial_validity(temp1, temp2))
    return NULL;

  SpanSet *ps = temporal_time(temp2);
  Temporal *rest = temporal_restrict_tstzspanset(temp1, ps, REST_MINUS);
  if (! rest)
    return temporal_copy((Temporal *) temp2);
  Temporal *result = temporal_insert(rest, temp2, connect);
  pfree(rest); pfree(ps);
  return (Temporal *) result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Delete a timestamp from a temporal value connecting the instants
 * before and after the given timestamp (if any).
 * @sqlfunc deleteTime
 */
Temporal *
temporal_delete_timestamp(const Temporal *temp, TimestampTz t, bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_delete_timestamp((TSequence *) temp, t,
      connect);
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_restrict_timestamp((TSequenceSet *) temp, t,
        REST_MINUS) :
      (Temporal *) tsequenceset_delete_timestamp((TSequenceSet *) temp, t);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Delete a timestamp set from a temporal value connecting the instants
 * before and after the given timestamp (if any).
 * @sqlfunc deleteTime
 */
Temporal *
temporal_delete_tstzset(const Temporal *temp, const Set *s, bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzset((TInstant *) temp, s,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_delete_tstzset((TSequence *) temp, s,
      connect);
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_tstzset((TSequenceSet *) temp, s) :
      (Temporal *) tsequenceset_restrict_tstzset((TSequenceSet *) temp, s,
        REST_MINUS);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Delete a period from a temporal value connecting the instants
 * before and after the given timestamp (if any).
 * @sqlfunc deleteTime
 */
Temporal *
temporal_delete_tstzspan(const Temporal *temp, const Span *s, bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) s))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzspan((TInstant *) temp, s,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_delete_tstzspan((TSequence *) temp, s,
      connect);
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_tstzspan((TSequenceSet *) temp, s) :
      (Temporal *) tsequenceset_restrict_tstzspan((TSequenceSet *) temp, s,
        REST_MINUS);
  }
  return result;
}

/**
 * @ingroup libmeos_temporal_modif
 * @brief Delete a period set from a temporal value connecting the instants
 * before and after the given timestamp (if any).
 * @sqlfunc deleteTime
 */
Temporal *
temporal_delete_tstzspanset(const Temporal *temp, const SpanSet *ss,
  bool connect)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ss))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_tstzspanset((TInstant *) temp, ss,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_delete_tstzspanset((TSequence *) temp, ss,
      connect);
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_tstzspanset((TSequenceSet *) temp, ss) :
      (Temporal *) tsequenceset_restrict_tstzspanset((TSequenceSet *) temp, ss,
        REST_MINUS);
  }
  return result;
}

/*****************************************************************************
 * Stops functions
 *****************************************************************************/

/**
 * @brief Calculate the length of the minimum bounding interval
 * of the input sequence between the given start and end instants.
 */
static double
mrr_distance_scalar(const TSequence *seq, int start, int end)
{
  assert(seq);
  assert(seq->temptype == T_TFLOAT);
  double min_value, max_value, curr_value;
  min_value = DatumGetFloat8(tinstant_value(TSEQUENCE_INST_N(seq, start)));
  max_value = min_value;
  for (int i = start + 1; i < end + 1; ++i)
  {
    curr_value = DatumGetFloat8(tinstant_value(TSEQUENCE_INST_N(seq, i)));
    min_value = fmin(min_value, curr_value);
    max_value = fmax(max_value, curr_value);
  }
  return max_value - min_value;
}

/**
 * @brief Return the subsequences where the temporal value stays within an area
 * with a given maximum size for at least the specified duration
 * (iterator function)
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
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration.
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
 * @ingroup libmeos_internal_temporal_transf
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration.
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return the subsequences where the temporal value stays within
 * an area with a given maximum size for at least the specified duration.
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

  TSequenceSet *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Input must be a temporal sequence (set) with linear interpolation");
    return NULL;
  }
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_stops((TSequence *) temp, maxdist, mintunits);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_stops((TSequenceSet *) temp, maxdist, mintunits);
  return result;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal number
 * @return On error return -1.0
 */
double
tnumber_integral(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tnumber_type(temp->temptype))
    return -1.0;

  double result = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
    ;
  else if (temp->subtype == TSEQUENCE)
    result = tnumberseq_integral((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnumberseqset_integral((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Return the time-weighted average of a temporal number
 * @return On error return DBL_MAX
 * @sqlfunc twAvg
 */
double
tnumber_twavg(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_tnumber_type(temp->temptype))
    return DBL_MAX;

  double result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tnumberinst_double((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tnumberseq_twavg((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tnumberseqset_twavg((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Compact function for final append aggregate
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_agg
 * @brief Compact the temporal value by removing extra storage space
 */
Temporal *
temporal_compact(const Temporal *temp)
{
  assert(temp);
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_compact((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_compact((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the temporal values are equal.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 */
bool
temporal_eq(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return false;

  const TInstant *inst1;
  const TSequence *seq;
  const TSequenceSet *ss;
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  /* If both are of the same temporal type use the specific equality */
  if (temp1->subtype == temp2->subtype)
  {
    if (temp1->subtype == TINSTANT)
      return tinstant_eq((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == TSEQUENCE)
      return tsequence_eq((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == TSEQUENCESET */
      return tsequenceset_eq((TSequenceSet *) temp1, (TSequenceSet *) temp2);
  }

  /* Different temporal type */
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
  if (MEOS_FLAGS_DISCRETE_INTERP(seq->flags))
  {
    for (int i = 0; i < seq->count; i++)
    {
      const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
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
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, 0);
    return tsequence_eq(seq, seq1);
  }
}

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the temporal values are different
 * @sqlop @p <>
 */
bool
temporal_ne(const Temporal *temp1, const Temporal *temp2)
{
  bool result = ! temporal_eq(temp1, temp2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc tbool_cmp, tint_cmp, tfloat_cmp, ttext_cmp, etc.
 */
int
temporal_cmp(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) ||
      ! ensure_same_temporal_type(temp1, temp2))
    return INT_MAX;

  /* Compare bounding period
   * We need to compare periods AND bounding boxes since the bounding boxes
   * do not distinguish between inclusive and exclusive bounds */
  Span p1, p2;
  temporal_set_tstzspan(temp1, &p1);
  temporal_set_tstzspan(temp2, &p2);
  int result = span_cmp(&p1, &p2);
  if (result)
    return result;

  /* Compare bounding box */
  bboxunion box1, box2;
  temporal_set_bbox(temp1, &box1);
  temporal_set_bbox(temp2, &box2);
  result = temporal_bbox_cmp(&box1, &box2, temp1->temptype);
  if (result)
    return result;

  /* If both are of the same temporal type use the specific comparison */
  if (temp1->subtype == temp2->subtype)
  {
    assert(temptype_subtype(temp1->subtype));
    if (temp1->subtype == TINSTANT)
      return tinstant_cmp((TInstant *) temp1, (TInstant *) temp2);
    else if (temp1->subtype == TSEQUENCE)
      return tsequence_cmp((TSequence *) temp1, (TSequence *) temp2);
    else /* temp1->subtype == TSEQUENCESET */
      return tsequenceset_cmp((TSequenceSet *) temp1, (TSequenceSet *) temp2);
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
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the first temporal value is less than the second one
 * @sqlop @p <
 */
bool
temporal_lt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
temporal_le(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 * @sqlop @p >
 */
bool
temporal_ge(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than the second one
 * @sqlop @p >=
 */
bool
temporal_gt(const Temporal *temp1, const Temporal *temp2)
{
  int cmp = temporal_cmp(temp1, temp2);
  return cmp > 0;
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the 32-bit hash value of a temporal value.
 * @result On error return INT_MAX
 */
uint32
temporal_hash(const Temporal *temp)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return INT_MAX;

  uint32 result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_hash((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_hash((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_hash((TSequenceSet *) temp);
  return result;
}

/*****************************************************************************/
