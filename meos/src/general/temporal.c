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
/* GEOS */
#include <geos_c.h>
/* POSTGRESQL */
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* POSTGIS */
#include <lwgeodetic.h>
#include <lwgeom_log.h>
#include <lwgeom_geos.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doxygen_libmeos.h"
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
 * Parameter tests
 *****************************************************************************/

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
void
ensure_discrete_interpolation(int16 flags)
{
  if (! MEOS_FLAGS_GET_DISCRETE(flags))
    elog(ERROR, "The temporal value must have discrete interpolation");
  return;
}

/**
 * @brief Ensure that a temporal value has continuous interpolation
 */
void
ensure_continuous_interpolation(int16 flags)
{
  if (! MEOS_FLAGS_GET_CONTINUOUS(flags))
    elog(ERROR, "The temporal value must have continuous interpolation");
  return;
}
#endif /* not used */

/**
 * @brief Ensure that the interpolation is valid
 * @note Used for the constructor functions
 */
void
ensure_valid_interpolation(meosType temptype, interpType interp)
{
  if (interp == LINEAR && ! temptype_continuous(temptype))
    elog(ERROR, "The temporal type cannot have linear interpolation");
  return;
}

/**
 * @brief Ensure that the subtype of temporal type is a sequence (set)
 */
void
ensure_continuous(const Temporal *temp)
{
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_GET_DISCRETE(temp->flags))
    elog(ERROR, "Input must be a temporal continuous sequence (set)");
  return;
}

/**
 * @brief Ensure that two temporal values have the same interpolation
 * @param[in] temp1,temp2 Input values
 */
void
ensure_same_interpolation(const Temporal *temp1, const Temporal *temp2)
{
  interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
  interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
  if (interp1 != interp2)
    elog(ERROR, "The temporal values must have the same interpolation");
  return;
}

/**
 * @brief Ensure that a temporal value does not have linear interpolation
 */
void
ensure_nonlinear_interpolation(int16 flags)
{
  if (MEOS_FLAGS_GET_LINEAR(flags))
    elog(ERROR, "The temporal value cannot have linear interpolation");
  return;
}

/**
 * @brief Ensure that two temporal values have at least one common dimension
 * based on their flags
 */
void
ensure_common_dimension(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) != MEOS_FLAGS_GET_X(flags2) &&
    MEOS_FLAGS_GET_T(flags1) != MEOS_FLAGS_GET_T(flags2))
    elog(ERROR, "The temporal values must have at least one common dimension");
  return;
}

/*****************************************************************************/

/**
 * @brief Ensure that the number is positive
 */
void
ensure_positive_datum(Datum size, meosType basetype)
{
  assert(span_basetype(basetype));
  if (basetype == T_INT4)
  {
    int isize = DatumGetInt32(size);
    if (isize <= 0)
      elog(ERROR, "The value must be positive: %d", isize);
  }
  else if (basetype == T_FLOAT8)
  {
    double dsize = DatumGetFloat8(size);
    if (dsize <= 0.0)
      elog(ERROR, "The value must be positive: %f", dsize);
  }
  else /* basetype == T_TIMESTAMPTZ */
  {
    int64 isize = DatumGetInt64(size);
    if (isize <= 0)
      elog(ERROR, "The value must be positive: " INT64_FORMAT "", isize);
  }
  return;
}

/**
 * @brief Ensure that the interval is a positive and absolute duration
 */
void
ensure_valid_duration(const Interval *duration)
{
  if (duration->month != 0)
  {
    elog(ERROR, "Interval defined in terms of month, year, century etc. not supported");
  }
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = pg_interval_cmp(duration, &intervalzero);
  if (cmp <= 0)
    elog(ERROR, "The interval must be positive");
  return;
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
      bool disc1 = MEOS_FLAGS_GET_DISCRETE(temp1->flags);
      bool disc2 = MEOS_FLAGS_GET_DISCRETE(temp2->flags);
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
      result = MEOS_FLAGS_GET_DISCRETE(temp1->flags) ?
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
      result = MEOS_FLAGS_GET_DISCRETE(temp2->flags) ?
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
  return temporal_parse(&str, T_TINT);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation.
 */
Temporal *
tfloat_in(const char *str)
{
  return temporal_parse(&str, T_TFLOAT);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation.
 */
Temporal *
ttext_in(const char *str)
{
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
  return temporal_out(temp, 0);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal float from its Well-Known Text (WKT) representation.
 */
char *
tfloat_out(const Temporal *temp, int maxdd)
{
  return temporal_out(temp, maxdd);
}

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return a temporal text from its Well-Known Text (WKT) representation.
 */
char *
ttext_out(const Temporal *temp)
{
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
  Temporal *result = palloc(VARSIZE(temp));
  memcpy(result, temp, VARSIZE(temp));
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_constructor
 * @brief Construct a temporal value from a base value and the time frame of
 * another temporal value.
 */
Temporal *
temporal_from_base_temp(Datum value, meosType temptype, const Temporal *temp)
{
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
  return temporal_from_base_temp(PointerGetDatum(txt), T_TTEXT, temp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geometric point from a point and the time frame
 * of another temporal value.
 */
Temporal *
tgeompoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp)
{
  return temporal_from_base_temp(PointerGetDatum(gs), T_TGEOMPOINT, temp);
}

/**
 * @ingroup libmeos_temporal_constructor
 * @brief Construct a temporal geographic point from a point and the time frame
 * of another temporal value.
 */
Temporal *
tgeogpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp)
{
  return temporal_from_base_temp(PointerGetDatum(gs), T_TGEOGPOINT, temp);
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
  assert(temp->temptype == inst->temptype);
  if (inst->subtype != TINSTANT)
    elog(ERROR, "The second argument must be of instant subtype");
  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */
  ensure_spatial_validity(temp, (const Temporal *) inst);

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
  assert(temp->temptype == seq->temptype);
  if (seq->subtype != TSEQUENCE)
    elog(ERROR, "The second argument must be of sequence subtype");
  ensure_same_interpolation(temp, (Temporal *) seq);
  /* The test to ensure the increasing timestamps must be done in the
   * subtype function since the inclusive/exclusive bounds must be
   * taken into account for temporal sequences and sequence sets */
  ensure_spatial_validity(temp, (Temporal *) seq);

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
    bool discrete1 = MEOS_FLAGS_GET_DISCRETE(temp1->flags);
    bool discrete2 = MEOS_FLAGS_GET_DISCRETE(temp2->flags);
    if (discrete1 == discrete2)
    {
      *out1 = (Temporal *) temp1;
      *out2 = (Temporal *) temp2;
    }
    else
    {
      *out1 = temporal_to_tsequenceset(temp1);
      *out2 = temporal_to_tsequenceset(temp2);
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
  /* Can't do anything with null inputs */
  if (! temp1 && ! temp2)
    return NULL;
  /* One argument is null, return a copy of the other temporal */
  if (! temp1)
    return temporal_copy(temp2);
  if (! temp2)
    return temporal_copy(temp1);

  /* Convert to the same subtype */
  assert(temp1->temptype == temp2->temptype);
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
  assert(count > 0);
  if (count == 1)
  {
    Temporal *result = temporal_copy(temparr[0]);
    return result;
  }

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
    if (spatial)
      ensure_spatial_validity(temparr[0], temparr[i]);
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
 * Cast functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal integer to a temporal float.
 * @sqlop @p ::
 */
Temporal *
tint_to_tfloat(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tintinst_to_tfloatinst((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tintseq_to_tfloatseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tintseqset_to_tfloatseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Cast a temporal float to a temporal integer.
 * @sqlop @p ::
 */
Temporal *
tfloat_to_tint(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tfloatinst_to_tintinst((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tfloatseq_to_tintseq((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tfloatseqset_to_tintseqset((TSequenceSet *) temp);
  return result;
}

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Compute the bounding period of a temporal value.
 */
void
temporal_set_period(const Temporal *temp, Span *p)
{
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    tinstant_set_period((TInstant *) temp, p);
  else if (temp->subtype == TSEQUENCE)
    tsequence_set_period((TSequence *) temp, p);
  else /* temp->subtype == TSEQUENCESET */
    tsequenceset_set_period((TSequenceSet *) temp, p);
  return;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of a temporal value.
 * @sqlfunc period
 * @sqlop @p ::
 * @pymeosfunc period
 */
Span *
temporal_to_period(const Temporal *temp)
{
  Span *result = palloc(sizeof(Span));
  temporal_set_period(temp, result);
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
    floatspan_set_numspan(&box->span, s, basetype);
  }
  return;
}

/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the value span of a temporal number.
 * @sqlfunc valueSpan
 */
Span *
tnumber_to_span(const Temporal *temp)
{
  Span *result = palloc(sizeof(Span));
  tnumber_set_span(temp, result);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_box_cast
 * @brief Return the bounding box of a temporal number.
 * @sqlop @p ::
 */
TBox *
tnumber_to_tbox(const Temporal *temp)
{
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
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    elog(ERROR, "Input must be a temporal sequence (set)");
  }
  else if (temp->subtype == TSEQUENCE)
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
temporal_to_tsequence(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequence((TInstant *) temp,
      MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP);
  else if (temp->subtype == TSEQUENCE)
    return (Temporal *) tsequence_copy((TSequence *) temp);
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
temporal_to_tsequenceset(const Temporal *temp)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_to_tsequenceset((TInstant *) temp,
      MEOS_FLAGS_GET_CONTINUOUS(temp->flags) ? LINEAR : STEP);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_to_tsequenceset((TSequence *) temp);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_copy((TSequenceSet *) temp);
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
  ensure_valid_interpolation(temp->temptype, interp);
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

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value shifted and/or scaled by the intervals.
 * @param[in] temp Temporal value
 * @param[in] shift Interval for shift
 * @param[in] duration Interval for scale
 * @pre The duration is greater than 0 if is not NULL
 * @sqlfunc shift, scale, shiftTscale
 * @pymeosfunc shift
 */
Temporal *
temporal_shift_tscale(const Temporal *temp, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (shift != NULL) ?
      (Temporal *) tinstant_shift((TInstant *) temp, shift) :
      (Temporal *) tinstant_copy((TInstant *) temp);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_shift_tscale((TSequence *) temp,
      shift, duration);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_shift_tscale((TSequenceSet *) temp,
      shift, duration);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value shifted by the interval.
 */
Temporal *
temporal_shift(const Temporal *temp, const Interval *shift)
{
  return temporal_shift_tscale(temp, shift, NULL);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Return a temporal value scaled by the interval.
 */
Temporal *
temporal_tscale(const Temporal *temp, const Interval *duration)
{
  return temporal_shift_tscale(temp, NULL, duration);
}
#endif /* MEOS */

/*****************************************************************************
 * Time precision functions for time values
 *****************************************************************************/

/**
 * @brief Set the precision of a temporal value according to time buckets.
 * @param[in] inst Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TInstant *
tinstant_tprecision(const TInstant *inst, const Interval *duration,
  TimestampTz torigin)
{
  ensure_valid_duration(duration);
  TimestampTz lower = timestamptz_bucket(inst->t, duration, torigin);
  Datum value = tinstant_value(inst);
  TInstant *result = tinstant_make(value, inst->temptype, lower);
  return result;
}

/**
 * @brief Aggregate the values that are merged due to a temporal granularity
 * change
 * @param[in] temp Temporal value
 */
static Datum
temporal_tprecision_agg_values(const Temporal *temp)
{
  assert(tnumber_type(temp->temptype) || tgeo_type(temp->temptype));
  Datum result;
  if (tnumber_type(temp->temptype))
    result = Float8GetDatum(tnumber_twavg(temp));
  else /* tgeo_type(temp->temptype) */
    result = PointerGetDatum(tpoint_twcentroid(temp));
  return result;
}

/**
 * @brief Set the precision of a temporal value according to period buckets.
 * @param[in] seq Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TSequence *
tsequence_tprecision(const TSequence *seq, const Interval *duration,
  TimestampTz torigin)
{
  assert(seq->temptype == T_TINT || seq->temptype == T_TFLOAT ||
    seq->temptype == T_TGEOMPOINT || seq->temptype == T_TGEOGPOINT );
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(seq->period.lower);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  TimestampTz lower_bucket = timestamptz_bucket(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz upper_bucket = timestamptz_bucket(upper, duration, torigin) +
    tunits;
  /* Number of buckets */
  int count = (int) (((int64) upper_bucket - (int64) lower_bucket) / tunits);
  TInstant **ininsts = palloc(sizeof(TInstant *) * seq->count);
  TInstant **outinsts = palloc(sizeof(TInstant *) * count);
  lower = lower_bucket;
  upper = lower_bucket + tunits;
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* New instants computing the value at the beginning/end of the bucket */
  TInstant *start = NULL, *end = NULL;
  // bool tofree = false;
  TSequence *seq1;
  Datum value;
  int k = 0; /* Number of instants for computing the twAvg/twCentroid */
  int l = 0; /* Number of instants of the output sequence */
  /* Loop for each instant of the sequence */
  for (int i = 0; i < seq->count; i++)
  {
    /* Get the next instant */
    TInstant *inst = (TInstant *) TSEQUENCE_INST_N(seq, i);
    /* If the instant is not in the current bucket */
    if (k > 0 && timestamptz_cmp_internal(inst->t, upper) > 0)
    {
      /* Compute the value at the end of the bucket if we do not have it */
      if (timestamptz_cmp_internal(ininsts[k - 1]->t, upper) < 0)
      {
        tsequence_value_at_timestamp(seq, upper, false, &value);
        ininsts[k++] = end = tinstant_make(value, seq->temptype, upper);
      }
      seq1 = tsequence_make((const TInstant **) ininsts, k, true, true, interp,
        NORMALIZE);
      /* Compute the twAvg/twCentroid for the bucket */
      value = tnumber_type(seq->temptype) ?
        Float8GetDatum(tnumberseq_twavg(seq1)) :
        PointerGetDatum(tpointseq_twcentroid(seq1));
      outinsts[l++] = tinstant_make(value, seq->temptype, lower);
      pfree(seq1);
      if (! tnumber_type(seq->temptype))
        pfree(DatumGetPointer(value));
      /* Free the instant at the beginning of the bucket if it was generated */
      if (start)
      {
        pfree(start); start = NULL;
      }
      /* Save the instant at the bucket end as start for the next bucket */
      ininsts[0] = ininsts[k - 1];
      if (end)
      {
        start = end; end = NULL;
      }
      k = 1;
      lower = upper;
      upper += tunits;
    }
    ininsts[k++] = inst;
  }
  /* Compute the twAvg/twCentroid of the last bucket */
  if (k > 0)
  {
    seq1 = tsequence_make((const TInstant **) ininsts, k, true, true, interp,
      NORMALIZE);
    value = tnumber_type(seq->temptype) ?
      Float8GetDatum(tnumberseq_twavg(seq1)) :
      PointerGetDatum(tpointseq_twcentroid(seq1));
    outinsts[l++] = tinstant_make(value, seq->temptype, lower);
    if (! tnumber_type(seq->temptype))
      pfree(DatumGetPointer(value));
    if (start)
      pfree(start);
  }
  TSequence *result = tsequence_make_free(outinsts, l, true, true, interp,
    NORMALIZE);
  pfree(ininsts);
  return result;
}

/**
 * @brief Set the precision of a temporal value according to period buckets.
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TSequenceSet *
tsequenceset_tprecision(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(ss->period.lower);
  TimestampTz upper = DatumGetTimestampTz(ss->period.upper);
  TimestampTz lower_bucket = timestamptz_bucket(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz upper_bucket = timestamptz_bucket(upper, duration, torigin) +
    tunits;
  /* Number of buckets */
  int count = (int) (((int64) upper_bucket - (int64) lower_bucket) / tunits);
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  lower = lower_bucket;
  upper = lower_bucket + tunits;
  bool linear = MEOS_FLAGS_GET_LINEAR(ss->flags);
  int nseqs = 0;
  /* Loop for each bucket */
  for (int i = 0; i < count; i++)
  {
    Span p;
    span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper),
      true, false, T_TIMESTAMPTZ, &p);
    span_set(TimestampTzGetDatum(lower),
      TimestampTzGetDatum(upper), true, false, T_TIMESTAMPTZ, &p);
    TSequenceSet *proj = tsequenceset_restrict_period(ss, &p, REST_AT);
    if (proj)
    {
      Datum value = temporal_tprecision_agg_values((Temporal *) proj);
      sequences[nseqs++] = tsequence_from_base_period(value, ss->temptype, &p,
        linear ? LINEAR : STEP);
      pfree(proj);
    }
    lower += tunits;
    upper += tunits;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Set the precision of a temporal value according to period buckets.
 * @sqlfunc tprecision;
 * @pymeosfunc tprecision
 */
Temporal *
temporal_tprecision(const Temporal *temp, const Interval *duration,
  TimestampTz torigin)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_tprecision((TInstant *) temp, duration,
      torigin);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_tprecision((TSequence *) temp, duration,
      torigin);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_tprecision((TSequenceSet *) temp,
      duration, torigin);
  return result;
}

/*****************************************************************************/

/**
 * @brief Sample the temporal value according to period buckets.
 * @param[in] inst Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TInstant *
tinstant_tsample(const TInstant *inst, const Interval *duration,
  TimestampTz torigin)
{
  ensure_valid_duration(duration);
  TimestampTz lower = timestamptz_bucket(inst->t, duration, torigin);
  if (timestamp_cmp_internal(lower, inst->t) == 0)
    return tinstant_copy(inst);
  return NULL;
}

/**
 * @brief Sample the temporal value according to period buckets.
 * @param[in] seq Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TSequence *
tsequence_tsample(const TSequence *seq, const Interval *duration,
  TimestampTz torigin)
{
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(seq->period.lower);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  TimestampTz lower_bucket = timestamptz_bucket(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz upper_bucket = timestamptz_bucket(upper, duration, torigin) +
    tunits;
  /* Number of buckets */
  int count = (int) (((int64) upper_bucket - (int64) lower_bucket) / tunits) + 1;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  const TInstant *end = TSEQUENCE_INST_N(seq, 1);
  lower = lower_bucket;
  /* Loop for each segment */
  bool lower_inc = seq->period.lower_inc;
  int i = 1; /* Current segment of the sequence */
  int ninsts = 0; /* Number of instants of the result */
  while (i < seq->count && lower < upper_bucket)
  {
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    int cmp1 = timestamptz_cmp_internal(start->t, lower);
    int cmp2 = timestamptz_cmp_internal(lower, end->t);
    /* If the segment contains the lower bound of the bucket */
    if ((cmp1 < 0 || (cmp1 == 0 && lower_inc)) &&
        (cmp2 < 0 || (cmp2 == 0 && upper_inc)))
    {
      Datum value = tsegment_value_at_timestamp(start, end, linear, lower);
      instants[ninsts++] = tinstant_make(value, seq->temptype, lower);
      /* Advance the bucket */
      lower += tunits;
    }
    /* Advance the bucket if it is after the start of the segment */
    else if (cmp1 >= 0)
      lower += tunits;
    /* Advance the segment if it is after the lower bound of the bucket */
    else if (cmp2 >= 0)
    {
      /* If there are no more segments */
      if (i == seq->count - 1)
        break;
      start = end;
      end = TSEQUENCE_INST_N(seq, ++i);
    }
  }
  if (ninsts == 0)
  {
    pfree(instants);
    return NULL;
  }
  return tsequence_make_free(instants, ninsts, true, true, interp, NORMALIZE);
}

/**
 * @brief Sample the temporal value according to period buckets.
 * @param[in] ss Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TSequenceSet *
tsequenceset_tsample(const TSequenceSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  /* Loop for each segment */
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TSequence *sample = tsequence_tsample(seq, duration, torigin);
    if (sample)
      sequences[nseqs++] = sample;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_transf
 * @brief Sample the temporal value according to period buckets.
 * @sqlfunc tsample
 * @pymeosfunc tsample
 */
Temporal *
temporal_tsample(const Temporal *temp, const Interval *duration,
  TimestampTz torigin)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_tsample((TInstant *) temp, duration,
      torigin);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tsequence_tsample((TSequence *) temp, duration,
      torigin);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_tsample((TSequenceSet *) temp,
      duration, torigin);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#define MEOS_SUBTYPE_STR_MAXLEN 12
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
  return VARSIZE(temp);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value.
 * @sqlfunc tempSubtype;
 * @pymeosfunc tempSubtype
 */
char *
temporal_subtype(const Temporal *temp)
{
  char *result = palloc(sizeof(char) * MEOS_SUBTYPE_STR_MAXLEN);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    strcpy(result, "Instant");
  else if (temp->subtype == TSEQUENCE)
    strcpy(result, "Sequence");
  else /* temp->subtype == TSEQUENCESET */
    strcpy(result, "SequenceSet");
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the string representation of the interpolation of a temporal
 * value.
 * @sqlfunc interp
 * @pymeosfunc interp
 */
char *
temporal_interp(const Temporal *temp)
{
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
 * @ingroup libmeos_temporal_accessor
 * @brief Return the array of distinct base values of a temporal value.
 * @sqlfunc values
 */
Datum *
temporal_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc getValues
 */
bool *
tbool_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc getValues
 */
int *
tint_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc getValues
 */
double *
tfloat_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc getValues
 */
text **
ttext_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc getValues
 */
GSERIALIZED **
tpoint_values(const Temporal *temp, int *count)
{
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
 * @pymeosfunc TFloat.getValues
 */
SpanSet *
tnumber_valuespans(const Temporal *temp)
{
  SpanSet *result;
  assert(tnumber_type(temp->temptype));
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
 * @pymeosfunc getTime
 */
SpanSet *
temporal_time(const Temporal *temp)
{
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
 * @pymeosfunc startValue
 */
Datum
temporal_start_value(const Temporal *temp)
{
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
 * @pymeosfunc startValue
 */
bool
tbool_start_value(const Temporal *temp)
{
  return DatumGetBool(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal integer
 * @sqlfunc startValue
 * @pymeosfunc startValue
 */
int
tint_start_value(const Temporal *temp)
{
  return DatumGetInt32(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal float
 * @sqlfunc startValue
 * @pymeosfunc startValue
 */
double
tfloat_start_value(const Temporal *temp)
{
  return DatumGetFloat8(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal text
 * @sqlfunc startValue
 * @pymeosfunc startValue
 */
text *
ttext_start_value(const Temporal *temp)
{
  return DatumGetTextP(temporal_start_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start value of a temporal geometric point
 * @sqlfunc startValue
 * @pymeosfunc startValue
 */
GSERIALIZED *
tpoint_start_value(const Temporal *temp)
{
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
 * @pymeosfunc endValue
 */
bool
tbool_end_value(const Temporal *temp)
{
  return DatumGetBool(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal integer
 * @sqlfunc endValue
 * @pymeosfunc endValue
 */
int
tint_end_value(const Temporal *temp)
{
  return DatumGetInt32(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal float
 * @sqlfunc endValue
 * @pymeosfunc endValue
 */
double
tfloat_end_value(const Temporal *temp)
{
  return DatumGetFloat8(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal text
 * @sqlfunc endValue
 * @pymeosfunc endValue
 */
text *
ttext_end_value(const Temporal *temp)
{
  return DatumGetTextP(temporal_end_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the end value of a temporal point
 * @sqlfunc endValue
 * @pymeosfunc endValue
 */
GSERIALIZED *
tpoint_end_value(const Temporal *temp)
{
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
 * @sqlfunc minValue
 * @pymeosfunc minValue
 */
int
tint_min_value(const Temporal *temp)
{
  return DatumGetInt32(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal float
 * @sqlfunc minValue
 * @pymeosfunc minValue
 */
double
tfloat_min_value(const Temporal *temp)
{
  return DatumGetFloat8(temporal_min_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the minimum value of a temporal text
 * @sqlfunc minValue
 * @pymeosfunc minValue
 */
text *
ttext_min_value(const Temporal *temp)
{
  return DatumGetTextP(temporal_min_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_accessor
 * @brief Return a copy of the maximum base value of a temporal value.
 * @sqlfunc maxValue
 * @pymeosfunc maxValue
 */
Datum
temporal_max_value(const Temporal *temp)
{
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
 * @sqlfunc maxValue
 * @pymeosfunc maxValue
 */
int
tint_max_value(const Temporal *temp)
{
  return DatumGetInt32(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal float
 * @sqlfunc maxValue
 * @pymeosfunc maxValue
 */
double
tfloat_max_value(const Temporal *temp)
{
  return DatumGetFloat8(temporal_max_value(temp));
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the maximum value of a temporal text
 * @sqlfunc maxValue
 * @pymeosfunc maxValue
 */
text *
ttext_max_value(const Temporal *temp)
{
  return DatumGetTextP(temporal_max_value(temp));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return a pointer to the instant with minimum base value of a
 * temporal value.
 *
 * @note The function does not take into account whether the instant is at
 * an exclusive bound or not.
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 * @sqlfunc minInstant
 */
const TInstant *
temporal_min_instant(const Temporal *temp)
{
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
 * @sqlfunc maxInstant
 */
const TInstant *
temporal_max_instant(const Temporal *temp)
{
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
 * @sqlfunc duration
 * @pymeosfunc duration
 */
Interval *
temporal_duration(const Temporal *temp, bool boundspan)
{
  Interval *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT ||
      (MEOS_FLAGS_GET_DISCRETE(temp->flags) && ! boundspan))
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
 * @sqlfunc numSequences
 */
int
temporal_num_sequences(const Temporal *temp)
{
  ensure_continuous(temp);
  int result = 1;
  if (temp->subtype == TSEQUENCESET)
    result = ((TSequenceSet *) temp)->count;
  return result;
}

/**
 * @ingroup libmeos_temporal_accessor
 * @brief Return the start sequence of a temporal sequence (set).
 * @sqlfunc startSequence
 */
TSequence *
temporal_start_sequence(const Temporal *temp)
{
  ensure_continuous(temp);
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
 * @sqlfunc endSequence
 */
TSequence *
temporal_end_sequence(const Temporal *temp)
{
  ensure_continuous(temp);
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
 * @note n is assumed to be 1-based.
 * @sqlfunc sequenceN
 */
TSequence *
temporal_sequence_n(const Temporal *temp, int i)
{
  ensure_continuous(temp);
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
 * @sqlfunc sequences
 */
TSequence **
temporal_sequences(const Temporal *temp, int *count)
{
  TSequence **result;
  ensure_continuous(temp);
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
 * @sqlfunc segments
 */
TSequence **
temporal_segments(const Temporal *temp, int *count)
{
  if (temp->subtype == TINSTANT)
    elog(ERROR, "The temporal value must be of subtype sequence (set)");

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
 * @sqlfunc numInstants
 * @pymeosfunc numInstants
 */
int
temporal_num_instants(const Temporal *temp)
{
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
 * @sqlfunc startInstant
 * @pymeosfunc startInstant
 */
const TInstant *
temporal_start_instant(const Temporal *temp)
{
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
 * @note This function is used for validity testing.
 * @sqlfunc endInstant
 * @pymeosfunc endInstant
 */
const TInstant *
temporal_end_instant(const Temporal *temp)
{
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
 * @note n is assumed 1-based
 * @sqlfunc instantN
 * @pymeosfunc instantN
 */
const TInstant *
temporal_instant_n(const Temporal *temp, int n)
{
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
 * @sqlfunc instants
 * @pymeosfunc instants
 */
const TInstant **
temporal_instants(const Temporal *temp, int *count)
{
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
 * @sqlfunc numTimestamps
 * @pymeosfunc numTimestamps
 */
int
temporal_num_timestamps(const Temporal *temp)
{
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
 * @sqlfunc startTimestamp
 * @pymeosfunc startTimestamp
 */
TimestampTz
temporal_start_timestamp(const Temporal *temp)
{
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
 * @sqlfunc endTimestamp
 * @pymeosfunc endTimestamp
 */
TimestampTz
temporal_end_timestamp(const Temporal *temp)
{
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
 * @note n is assumed 1-based
 * @sqlfunc timestampN
 * @pymeosfunc timestampN
 */
bool
temporal_timestamp_n(const Temporal *temp, int n, TimestampTz *result)
{
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
 * @sqlfunc timestamps
 * @pymeosfunc timestamps
 */
TimestampTz *
temporal_timestamps(const Temporal *temp, int *count)
{
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
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    TBox box;
    temporal_set_bbox(temp, &box);
    meosType basetype = temptype_basetype(temp->temptype);
    Datum dvalue = Float8GetDatum(datum_double(value, basetype));
    return (ever &&
        datum_le(box.span.lower, dvalue, box.span.basetype) &&
        datum_le(dvalue, box.span.upper, box.span.basetype)) ||
      (!ever && box.span.lower == dvalue &&
        dvalue == box.span.upper);
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
  if (tnumber_type(temp->temptype))
  {
    TBox box;
    temporal_set_bbox(temp, &box);
    meosType basetype = temptype_basetype(temp->temptype);
    Datum dvalue = Float8GetDatum(datum_double(value, basetype));
    if ((ever && datum_lt(dvalue, box.span.lower, box.span.basetype)) ||
      (! ever && datum_lt(dvalue, box.span.upper, box.span.basetype)))
      return false;
  }
  return true;
}

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is ever equal to a base value.
 */
bool
temporal_ever_eq(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal boolean is ever equal to a boolean.
 * @sqlop @p ?=
 */
bool tbool_ever_eq(const Temporal *temp, bool b)
{
  return temporal_ever_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever equal to an integer.
 * @sqlop @p ?=
 */
bool tint_ever_eq(const Temporal *temp, int i)
{
  return temporal_ever_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever equal to a float.
 * @sqlop @p ?=
 */
bool tfloat_ever_eq(const Temporal *temp, double d)
{
  return temporal_ever_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever equal to a text.
 * @sqlop @p ?=
 */
bool ttext_ever_eq(const Temporal *temp, text *txt)
{
  return temporal_ever_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is always equal to a base value.
 */
bool
temporal_always_eq(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal boolean is always equal to a boolean.
 * @sqlop @p %=
 */
bool tbool_always_eq(const Temporal *temp, bool b)
{
  return temporal_always_eq(temp, BoolGetDatum(b));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always equal to an integer.
 * @sqlop @p %=
 */
bool tint_always_eq(const Temporal *temp, int i)
{
  return temporal_always_eq(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always equal to a float.
 * @sqlop @p %=
 */
bool tfloat_always_eq(const Temporal *temp, double d)
{
  return temporal_always_eq(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always equal to a text.
 * @sqlop @p %=
 */
bool ttext_always_eq(const Temporal *temp, text *txt)
{
  return temporal_always_eq(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is ever less than a base value.
 */
bool
temporal_ever_lt(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever less than an integer.
 * @sqlop @p ?<
 */
bool tint_ever_lt(const Temporal *temp, int i)
{
  return temporal_ever_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever less than a float.
 * @sqlop @p ?<
 */
bool tfloat_ever_lt(const Temporal *temp, double d)
{
  return temporal_ever_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever less than a text.
 * @sqlop @p ?<
 */
bool ttext_ever_lt(const Temporal *temp, text *txt)
{
  return temporal_ever_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is always less than a base value.
 */
bool
temporal_always_lt(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always less than an integer.
 * @sqlop @p %<
 */
bool tint_always_lt(const Temporal *temp, int i)
{
  return temporal_always_lt(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always less than  a float.
 * @sqlop @p %<
 */
bool tfloat_always_lt(const Temporal *temp, double d)
{
  return temporal_always_lt(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always less than  a text.
 * @sqlop @p %<
 */
bool ttext_always_lt(const Temporal *temp, text *txt)
{
  return temporal_always_lt(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is ever less than or equal to a
 * base value.
 */
bool
temporal_ever_le(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is ever less than or equal to an integer.
 * @sqlop @p ?<=
 */
bool tint_ever_le(const Temporal *temp, int i)
{
  return temporal_ever_le(temp, Int32GetDatum(i));

}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is ever less than or equal to a float.
 * @sqlop @p ?<=
 */
bool tfloat_ever_le(const Temporal *temp, double d)
{
  return temporal_ever_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is ever less than or equal to a text.
 * @sqlop @p ?<=
 */
bool ttext_ever_le(const Temporal *temp, text *txt)
{
  return temporal_ever_le(temp, PointerGetDatum(txt));
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_temporal_ever
 * @brief Return true if a temporal value is always less than or equal to a
 * base value.
 */
bool
temporal_always_le(const Temporal *temp, Datum value)
{
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
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal integer is always less than or equal to an integer.
 * @sqlop @p %<=
 */
bool tint_always_le(const Temporal *temp, int i)
{
  return temporal_always_le(temp, Int32GetDatum(i));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal float is always less than or equal to a float.
 * @sqlop @p %<=
 */
bool tfloat_always_le(const Temporal *temp, double d)
{
  return temporal_always_le(temp, Float8GetDatum(d));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal text is always less than or equal to a text.
 * @sqlop @p %<=
 */
bool ttext_always_le(const Temporal *temp, text *txt)
{
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
    ensure_point_type(gs);
    ensure_same_srid(tpoint_srid(temp), gserialized_get_srid(gs));
    ensure_same_dimensionality_tpoint_gs(temp, gs);
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
tnumber_bbox_restrict_span(const Temporal *temp, const Span *span)
{
  /* Bounding box test */
  assert(tnumber_type(temp->temptype));
  TBox box1, box2;
  temporal_set_bbox(temp, &box1);
  numspan_set_tbox(span, &box2);
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
  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_value(temp, value))
  {
    if (atfunc)
      return NULL;
    else
      return (temp->subtype != TSEQUENCE ||
          MEOS_FLAGS_GET_DISCRETE(temp->flags)) ?
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
temporal_bbox_restrict_set(const Temporal *temp, const Set *set)
{
  /* Bounding box test */
  if (tnumber_type(temp->temptype))
  {
    Span span1, span2;
    tnumber_set_span(temp, &span1);
    set_set_span(set, &span2);
    return overlaps_span_span(&span1, &span2);
  }
  if (tgeo_type(temp->temptype) && temp->subtype != TINSTANT)
  {
    STBox box;
    temporal_set_bbox(temp, &box);
    return contains_stbox_stbox(&box, (STBox *) SET_BBOX_PTR(set));
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
temporal_restrict_values(const Temporal *temp, const Set *set, bool atfunc)
{
  /* Ensure validity of arguments */
  if (tgeo_type(temp->temptype))
  {
    ensure_same_srid(tpoint_srid(temp), geoset_srid(set));
    ensure_same_spatial_dimensionality(temp->flags, set->flags);
  }

  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! temporal_bbox_restrict_set(temp, set))
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
    result = (Temporal *) tinstant_restrict_values((TInstant *) temp, set,
      atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tdiscseq_restrict_values((TSequence *) temp, set, atfunc) :
      (Temporal *) tcontseq_restrict_values((TSequence *) temp, set, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_values((TSequenceSet *) temp,
      set, atfunc);

  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a span of base values.
 * @sqlfunc atSpan, minusSpan
 */
Temporal *
tnumber_restrict_span(const Temporal *temp, const Span *span, bool atfunc)
{
  /* Bounding box test */
  interpType interp = MEOS_FLAGS_GET_INTERP(temp->flags);
  if (! tnumber_bbox_restrict_span(temp, span))
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
      span, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = (interp == DISCRETE) ?
      (Temporal *) tnumberdiscseq_restrict_span((TSequence *) temp, span,
        atfunc) :
      (Temporal *) tnumbercontseq_restrict_span((TSequence *) temp, span,
        atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tnumberseqset_restrict_span((TSequenceSet *) temp,
      span, atfunc);
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
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = atfunc ? (Temporal *) tinstant_copy((TInstant *) temp) : NULL;
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
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
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t, atfunc);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
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
 * @pymeosfunc valueAtTimestamp
 */
bool
temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict,
  Datum *result)
{
  bool found = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    found = tinstant_value_at_timestamp((TInstant *) temp, t, result);
  else if (temp->subtype == TSEQUENCE)
    found = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
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
temporal_restrict_timestampset(const Temporal *temp, const Set *ts, bool atfunc)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestampset(
      (TInstant *) temp, ts, atfunc);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
      result = (Temporal *) tdiscseq_restrict_timestampset((TSequence *) temp,
        ts, atfunc);
    else
      result = atfunc ?
        (Temporal *) tcontseq_at_timestampset((TSequence *) temp, ts) :
        (Temporal *) tcontseq_minus_timestampset((TSequence *) temp, ts);
  }
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_timestampset(
      (TSequenceSet *) temp, ts, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period.
 * @sqlfunc atTime, minusTime
 */
Temporal *
temporal_restrict_period(const Temporal *temp, const Span *p, bool atfunc)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_period(
      (TInstant *) temp, p, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_restrict_period((TSequence *) temp, p, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_period(
      (TSequenceSet *) temp, p, atfunc);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_restrict
 * @brief Restrict a temporal value to (the complement of) a period set.
 * @sqlfunc atTime, minusTime
 */
Temporal *
temporal_restrict_periodset(const Temporal *temp, const SpanSet *ps,
  bool atfunc)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_periodset(
      (TInstant *) temp, ps, atfunc);
  else if (temp->subtype == TSEQUENCE)
    result = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal *) tdiscseq_restrict_periodset((TSequence *) temp, ps, atfunc) :
      (Temporal *) tcontseq_restrict_periodset((TSequence *) temp, ps, atfunc);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tsequenceset_restrict_periodset(
      (TSequenceSet *) temp, ps, atfunc);
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
    temp1 = temporal_restrict_period(temp, &box->period, REST_AT);
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
    result = temporal_restrict_periodset(temp, ps, REST_MINUS);
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
  /* Convert to the same subtype */
  assert(temp1->temptype == temp2->temptype);
  Temporal *new1, *new2;
  temporal_convert_same_subtype(temp1, temp2, &new1, &new2);

  Temporal *result;
  assert(temptype_subtype(new1->subtype));
  if (new1->subtype == TINSTANT)
    result = tinstant_merge((TInstant *) new1, (TInstant *) new2);
  else if (new1->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(new1->flags) || ! connect)
      result = (Temporal *) tsequence_merge((TSequence *) new1,
        (TSequence *) new2);
    else
      result = (Temporal *) tcontseq_insert((TSequence *) new1,
        (TSequence *) new2);
  }
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
  SpanSet *ps = temporal_time(temp2);
  Temporal *rest = temporal_restrict_periodset(temp1, ps, REST_MINUS);
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
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestamp((TInstant *) temp, t,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
      result = (Temporal *) tdiscseq_minus_timestamp((TSequence *) temp, t);
    else
      result = connect ?
        (Temporal *) tcontseq_delete_timestamp((TSequence *) temp, t) :
        (Temporal *) tcontseq_minus_timestamp((TSequence *) temp, t);
  }
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
temporal_delete_timestampset(const Temporal *temp, const Set *ts,
  bool connect)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_timestampset((TInstant *) temp, ts,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
      result = (Temporal *) tdiscseq_restrict_timestampset((TSequence *) temp,
        ts, REST_MINUS);
    else
      result = connect ?
        (Temporal *) tcontseq_delete_timestampset((TSequence *) temp, ts) :
        (Temporal *) tcontseq_minus_timestampset((TSequence *) temp, ts);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_timestampset((TSequenceSet *) temp, ts) :
      (Temporal *) tsequenceset_restrict_timestampset((TSequenceSet *) temp, ts,
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
temporal_delete_period(const Temporal *temp, const Span *p, bool connect)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_period((TInstant *) temp, p,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
      result = (Temporal *) tdiscseq_restrict_period((TSequence *) temp, p,
        REST_MINUS);
    else
      result = connect ?
        (Temporal *) tcontseq_delete_period((TSequence *) temp, p) :
        (Temporal *) tcontseq_minus_period((TSequence *) temp, p);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_period((TSequenceSet *) temp, p) :
      (Temporal *) tsequenceset_restrict_period((TSequenceSet *) temp, p,
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
temporal_delete_periodset(const Temporal *temp, const SpanSet *ps,
  bool connect)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tinstant_restrict_periodset((TInstant *) temp, ps,
      REST_MINUS);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MEOS_FLAGS_GET_DISCRETE(temp->flags))
      result = (Temporal *) tdiscseq_restrict_periodset((TSequence *) temp, ps,
        REST_MINUS);
    else
      result = connect ?
        (Temporal *) tcontseq_delete_periodset((TSequence *) temp, ps) :
        (Temporal *) tcontseq_restrict_periodset((TSequence *) temp, ps,
          REST_MINUS);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = connect ?
      (Temporal *) tsequenceset_delete_periodset((TSequenceSet *) temp, ps) :
      (Temporal *) tsequenceset_restrict_periodset((TSequenceSet *) temp, ps,
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

/* Defined in liblwgeom_internal.h */
#define PGIS_FP_TOLERANCE 1e-12

/**
 * @brief Calculate the distance between two geographic points
 * given as GEOS geometries.
 */
static double
geog_distance_geos(const GEOSGeometry *pt1, const GEOSGeometry *pt2)
{
  /* Skip PostGIS function calls */
  double x1, y1, x2, y2;
  GEOSGeomGetX(pt1, &x1);
  GEOSGeomGetY(pt1, &y1);
  GEOSGeomGetX(pt2, &x2);
  GEOSGeomGetY(pt2, &y2);

  /* Code taken from ptarray_distance_spheroid function in lwgeodetic.c */

  GEOGRAPHIC_POINT g1, g2;
  geographic_point_init(x1, y1, &g1);
  geographic_point_init(x2, y2, &g2);

  SPHEROID s;
  spheroid_init(&s, WGS84_MAJOR_AXIS, WGS84_MINOR_AXIS);

  /* Sphere special case, axes equal */
  double distance = s.radius * sphere_distance(&g1, &g2);
  if ( s.a == s.b )
    return distance;
  /* Below tolerance, actual distance isn't of interest */
  else if ( distance < 0.95 * PGIS_FP_TOLERANCE )
    return distance;
  /* Close or greater than tolerance, get the real answer to be sure */
  else
    return spheroid_distance(&g1, &g2, &s);
}

/**
 * @brief Calculate the length of the diagonal of the minimum rotated rectangle
 * of the input GEOS geometry.
 * @note The computation is always done in 2D
 */
static double
mrr_distance_geos(GEOSGeometry *geom, bool geodetic)
{

  double result = 0;
  int numGeoms = GEOSGetNumGeometries(geom);
  if (numGeoms == 2)
  {
    const GEOSGeometry *pt1 = GEOSGetGeometryN(geom, 0);
    const GEOSGeometry *pt2 = GEOSGetGeometryN(geom, 1);
    if (geodetic)
      result = geog_distance_geos(pt1, pt2);
    else
      GEOSDistance(pt1, pt2, &result);
  }
  else if (numGeoms > 2)
  {
    GEOSGeometry *mrr_geom = GEOSMinimumRotatedRectangle(geom);
    GEOSGeometry *pt1, *pt2;
    switch (GEOSGeomTypeId(mrr_geom))
    {
      case GEOS_POINT:
        result = 0;
        break;
      case GEOS_LINESTRING: /* compute length of linestring */
        if (geodetic)
        {
          pt1 = GEOSGeomGetStartPoint(mrr_geom);
          pt2 = GEOSGeomGetEndPoint(mrr_geom);
          result = geog_distance_geos(pt1, pt2);
          GEOSGeom_destroy(pt1);
          GEOSGeom_destroy(pt2);
        }
        else
          GEOSGeomGetLength(mrr_geom, &result);
        break;
      case GEOS_POLYGON: /* compute length of diagonal */
        pt1 = GEOSGeomGetPointN(GEOSGetExteriorRing(mrr_geom), 0);
        pt2 = GEOSGeomGetPointN(GEOSGetExteriorRing(mrr_geom), 2);
        if (geodetic)
          result = geog_distance_geos(pt1, pt2);
        else
          GEOSDistance(pt1, pt2, &result);
        GEOSGeom_destroy(pt1);
        GEOSGeom_destroy(pt2);
        break;
      default:
        elog(ERROR, "Invalid geometry type for Minimum Rotated Rectangle");
    }
  }
  return result;
}

/**
 * @brief Create a GEOS Multipoint geometry from a part (defined by start and
 * end) of a temporal point sequence
 */
static GEOSGeometry *
multipoint_make(const TSequence *seq, int start, int end)
{
  GSERIALIZED *gs = NULL; /* make compiler quiet */
  GEOSGeometry **geoms = palloc(sizeof(GEOSGeometry *) * (end - start + 1));
  for (int i = 0; i < end - start + 1; ++i)
  {
    if (tgeo_type(seq->temptype))
      gs = DatumGetGserializedP(
        tinstant_value(TSEQUENCE_INST_N(seq, start + i)));
#if NPOINT
    else if (seq->temptype == T_TNPOINT)
      gs = npoint_geom(DatumGetNpointP(
        tinstant_value(TSEQUENCE_INST_N(seq, start + i))));
#endif
    else
      elog(ERROR, "Sequence must have a spatial base type");
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    geoms[i] = GEOSGeom_createPointFromXY(pt->x, pt->y);
  }
  GEOSGeometry *result = GEOSGeom_createCollection(GEOS_MULTIPOINT, geoms, end - start + 1);
  pfree(geoms);
  return result;
}

/**
 * @brief Add the point stored in the given instant to a GEOS multipoint geometry
 */
static GEOSGeometry *
multipoint_add_inst_free(GEOSGeometry *geom, const TInstant *inst)
{
  GSERIALIZED *gs = NULL; /* make compiler quiet */
  if (tgeo_type(inst->temptype))
    gs = DatumGetGserializedP(tinstant_value(inst));
#if NPOINT
  else if (inst->temptype == T_TNPOINT)
    gs = npoint_geom(DatumGetNpointP(tinstant_value(inst)));
#endif
  else
    elog(ERROR, "Instant must have a spatial base type");
  const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
  GEOSGeometry *geom1 = GEOSGeom_createPointFromXY(pt->x, pt->y);
  GEOSGeometry *result = GEOSUnion(geom, geom1);
  GEOSGeom_destroy(geom1);
  GEOSGeom_destroy(geom);
  return result;
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
tsequence_stops_iter(const TSequence *seq, double maxdist, int64 mintunits,
  TSequence **result)
{
  assert(seq->count > 1);
  assert(seq->temptype == T_TFLOAT || tgeo_type(seq->temptype) ||
    seq->temptype == T_TNPOINT);

  /* Use GEOS only for non-scalar input */
  bool use_geos_dist = seq->temptype != T_TFLOAT;
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(seq->flags);

  const TInstant *inst1 = NULL, *inst2 = NULL; /* make compiler quiet */
  GEOSGeometry *geom = NULL;

  if (use_geos_dist)
  {
    initGEOS(lwnotice, lwgeom_geos_error);
    geom = GEOSGeom_createEmptyCollection(GEOS_MULTIPOINT);
  }

  int end, start = 0, nseqs = 0;
  bool  is_stopped = false,
        previously_stopped = false,
        rebuild_geom = false;

  for (end = 0; end < seq->count; ++end)
  {
    inst1 = TSEQUENCE_INST_N(seq, start);
    inst2 = TSEQUENCE_INST_N(seq, end);

    while (! is_stopped && end - start > 1
      && (int64)(inst2->t - inst1->t) >= mintunits)
    {
      inst1 = TSEQUENCE_INST_N(seq, ++start);
      rebuild_geom = true;
    }

    if (rebuild_geom && use_geos_dist)
    {
      GEOSGeom_destroy(geom);
      geom = multipoint_make(seq, start, end);
      rebuild_geom = false;
    }
    else if (use_geos_dist)
      geom = multipoint_add_inst_free(geom, inst2);

    if (end - start == 0)
      continue;

    if (use_geos_dist)
      is_stopped = mrr_distance_geos(geom, geodetic) <= maxdist;
    else
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

      if (use_geos_dist)
        rebuild_geom = true;
    }

    previously_stopped = is_stopped;
  }

  if (use_geos_dist)
    GEOSGeom_destroy(geom);

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
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count);
  int nseqs = tsequence_stops_iter(seq, maxdist, mintunits, sequences);
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
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->totalcount);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Instantaneous sequences do not have stops */
    if (seq->count == 1)
      continue;
    nseqs += tsequence_stops_iter(seq, maxdist, mintunits, &sequences[nseqs]);
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
  if (maxdist < 0)
    elog(ERROR, "The maximum distance must be positive: %f", maxdist);
  Interval intervalzero;
  memset(&intervalzero, 0, sizeof(Interval));
  int cmp = pg_interval_cmp(minduration, &intervalzero);
  if (cmp < 0)
    elog(ERROR, "The duration must be positive");
  int64 mintunits = interval_units(minduration);

  TSequenceSet *result = NULL;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || ! MEOS_FLAGS_GET_LINEAR(temp->flags))
  {
    elog(ERROR, "Input must be a temporal sequence (set) with linear interpolation");
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
 */
double
tnumber_integral(const Temporal *temp)
{
  double result = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT || MEOS_FLAGS_GET_DISCRETE(temp->flags))
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
 * @sqlfunc twAvg
 */
double
tnumber_twavg(const Temporal *temp)
{
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
 * @ingroup libmeos_temporal_comp
 * @brief Return true if the temporal values are equal.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__
 */
bool
temporal_eq(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->temptype == temp2->temptype);
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));

  const TInstant *inst1;
  const TSequence *seq;
  const TSequenceSet *ss;
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
  if (MEOS_FLAGS_GET_DISCRETE(seq->flags))
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
 * @ingroup libmeos_temporal_comp
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
 * @ingroup libmeos_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value is
 * less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc tbool_cmp, tint_cmp, tfloat_cmp, ttext_cmp, etc.
 */
int
temporal_cmp(const Temporal *temp1, const Temporal *temp2)
{
  assert(temp1->temptype == temp2->temptype);

  /* Compare bounding period
   * We need to compare periods AND bounding boxes since the bounding boxes
   * do not distinguish between inclusive and exclusive bounds */
  Span p1, p2;
  temporal_set_period(temp1, &p1);
  temporal_set_period(temp2, &p2);
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
 * @ingroup libmeos_temporal_comp
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
 * @ingroup libmeos_temporal_comp
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
 * @ingroup libmeos_temporal_comp
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
 * @ingroup libmeos_temporal_comp
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
 */
uint32
temporal_hash(const Temporal *temp)
{
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
