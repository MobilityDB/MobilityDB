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
 * @brief Basic functions for temporal types of any subtype.
 */

#include "pg_general/temporal.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <access/heaptoast.h>
  #include <access/detoast.h>
#else
  #include <access/tuptoaster.h>
#endif
#include <libpq/pqformat.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_out.h"
#include "general/temporal_util.h"
#include "general/pg_call.h"
#include "general/temporaltypes.h"
#include "general/temporal_boxops.h"
/* MobilityDB */
#include "pg_general/doxygen_mobilitydb_api.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_util.h"
#include "pg_general/tinstant.h"
#include "pg_general/tsequence.h"
#include "pg_point/tpoint_spatialfuncs.h"

/* To avoid including fmgrprotos.h */
extern Datum timestamp_mi(PG_FUNCTION_ARGS);
extern Datum interval_cmp(PG_FUNCTION_ARGS);

/*
 * This is required in a SINGLE file for builds against PostgreSQL
 */
PG_MODULE_MAGIC;

/*****************************************************************************
 * Initialization function
 *****************************************************************************/

/**
 * @brief Initialize the MobilityDB extension
 */
void
_PG_init(void)
{
  /* elog(WARNING, "This is MobilityDB."); */
  temporalgeom_init();
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that the array is not empty
 *
 * @note Used for the constructor functions
 */
void
ensure_non_empty_array(ArrayType *array)
{
  if (ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)) == 0)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("The input array cannot be empty")));
  return;
}

/*****************************************************************************
 * Typmod functions
 *****************************************************************************/

/**
 * @brief Array storing the string representation of the concrete subtypes of
 * temporal types
 */
static char *tempsubtypeName[] =
{
  "AnyDuration",
  "Instant",
  "Sequence",
  "SequenceSet"
};

/**
 * @brief Array storing the mapping between the string representation of the
 * subtypes of temporal types and the corresponding enum value
 */
struct tempsubtype_struct tempsubtype_struct_array[] =
{
  {"ANYTEMPSUBTYPE", ANYTEMPSUBTYPE},
  {"INSTANT", TINSTANT},
  {"SEQUENCE", TSEQUENCE},
  {"SEQUENCESET", TSEQUENCESET},
};

/**
 * @brief Return the string representation of the subtype of the temporal type
 * corresponding to the enum value
 */
const char *
tempsubtype_name(int16 subtype)
{
  return tempsubtypeName[subtype];
}

/**
 * @brief Return the enum value corresponding to the string representation
 * of the concrete subtype of a temporal type.
 */
bool
tempsubtype_from_string(const char *str, int16 *subtype)
{
  char *tmpstr;
  size_t tmpstartpos, tmpendpos;
  size_t i;

  /* Initialize */
  *subtype = 0;
  /* Locate any leading/trailing spaces */
  tmpstartpos = 0;
  for (i = 0; i < strlen(str); i++)
  {
    if (str[i] != ' ')
    {
      tmpstartpos = i;
      break;
    }
  }
  tmpendpos = strlen(str) - 1;
  for (i = strlen(str) - 1; i != 0; i--)
  {
    if (str[i] != ' ')
    {
      tmpendpos = i;
      break;
    }
  }
  tmpstr = palloc(tmpendpos - tmpstartpos + 2);
  for (i = tmpstartpos; i <= tmpendpos; i++)
    tmpstr[i - tmpstartpos] = str[i];
  /* Add NULL to terminate */
  tmpstr[i - tmpstartpos] = '\0';
  size_t len = strlen(tmpstr);
  /* Now check for the type */
  for (i = 0; i < TEMPSUBTYPE_STRUCT_ARRAY_LEN; i++)
  {
    if (len == strnlen(tempsubtype_struct_array[i].subtypeName,
        TEMPSUBTYPE_MAX_LEN) &&
      ! strncasecmp(tmpstr, tempsubtype_struct_array[i].subtypeName,
        TEMPSUBTYPE_MAX_LEN))
    {
      *subtype = tempsubtype_struct_array[i].subtype;
      pfree(tmpstr);
      return true;
    }
  }
  pfree(tmpstr);
  return false;
}

/**
 * @brief Ensure that the temporal type of a temporal value corresponds to the
 * typmod
 */
static Temporal *
temporal_valid_typmod(Temporal *temp, int32_t typmod)
{
  /* No typmod (-1) */
  if (typmod < 0)
    return temp;
  uint8 typmod_subtype = TYPMOD_GET_SUBTYPE(typmod);
  uint8 subtype = temp->subtype;
  /* Typmod has a preference */
  if (typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal type (%s) does not match column type (%s)",
      tempsubtype_name(subtype), tempsubtype_name(typmod_subtype))));
  return temp;
}

PG_FUNCTION_INFO_V1(Temporal_typmod_in);
/**
 * @brief Input typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  Datum *elem_values;
  int n = 0;

  if (ARR_ELEMTYPE(array) != CSTRINGOID)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("typmod array must be type cstring[]")));
  if (ARR_NDIM(array) != 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("typmod array must be one-dimensional")));
  if (ARR_HASNULL(array))
    ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
      errmsg("typmod array must not contain nulls")));

  deconstruct_array(array, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
  if (n != 1)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid temporal type modifier")));

  /* Temporal Type */
  char *s = DatumGetCString(elem_values[0]);
  if (strlen(s) == 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Empty temporal type modifier")));

  int16 subtype = ANYTEMPSUBTYPE;
  if (! tempsubtype_from_string(s, &subtype))
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Invalid temporal type modifier: %s", s)));

  pfree(elem_values);
  PG_RETURN_INT32((int32) subtype);
}

PG_FUNCTION_INFO_V1(Temporal_typmod_out);
/**
 * @brief Output typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  int16 subtype = TYPMOD_GET_SUBTYPE(typmod);
  /* No type? Then no typmod at all. Return empty string.  */
  if (typmod < 0 || !subtype)
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  sprintf(str, "(%s)", tempsubtype_name(subtype));
  PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(Temporal_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal types
 */
PGDLLEXPORT Datum
Temporal_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if temporal typmod is consistent with the supplied one */
  temp = temporal_valid_typmod(temp, typmod);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Peak into a temporal datum to find the bounding box. If the datum needs
 * needs to be detoasted, extract only the header and not the full object.
 */
void
temporal_bbox_slice(Datum tempdatum, void *box)
{
  Temporal *temp = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tempdatum))
    temp = (Temporal *) PG_DETOAST_DATUM_SLICE(tempdatum, 0,
      temporal_max_header_size());
  else
    temp = (Temporal *) tempdatum;
  temporal_set_bbox(temp, box);
  PG_FREE_IF_COPY_P(temp, DatumGetPointer(tempdatum));
  return;
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Mobilitydb_version);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Version of the MobilityDB extension
 * @sqlfunc mobilitydb_version()
 */
PGDLLEXPORT Datum
Mobilitydb_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char *version = mobilitydb_version();
  text *result = cstring_to_text(version);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Mobilitydb_full_version);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Versions of the MobilityDB extension and its dependencies
 * @sqlfunc mobilitydb_full_version()
 */
PGDLLEXPORT Datum
Mobilitydb_full_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char *version = mobilitydb_full_version();
  text *result = cstring_to_text(version);
  pfree(version);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_in);
/**
 * @ingroup mobilitydb_temporal_in_out
 * @brief Generic input function for temporal types
 * @sqlfunc tbool_in(), tint_in(), tfloat_in(), ttext_in(),
 */
PGDLLEXPORT Datum
Temporal_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_in(input, oid_type(temptypid));
  int32 temp_typmod = -1;
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
    temp_typmod = PG_GETARG_INT32(2);
  if (temp_typmod >= 0)
    result = temporal_valid_typmod(result, temp_typmod);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_out);
/**
 * @ingroup mobilitydb_temporal_in_out
 * @brief Generic output function for temporal types
 * @sqlfunc tbool_out(), tint_out(), tfloat_out(), ttext_out(),
 */
PGDLLEXPORT Datum
Temporal_out(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *result = temporal_out(temp, OUT_DEFAULT_DECIMAL_DIGITS);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_CSTRING(result);
}

/**
 * @brief Return a temporal value from its binary representation read from
 * a buffer.
 * @note Function needed for temporal aggregation and thus only instant and
 * sequence subtypes must be considered
 */
Temporal *
temporal_recv(StringInfo buf)
{
  uint8 temptype = pq_getmsgbyte(buf);
  uint8 subtype = pq_getmsgbyte(buf);
  Temporal *result;
  assert(subtype == TINSTANT || subtype == TSEQUENCE);
  if (subtype == TINSTANT)
    result = (Temporal *) tinstant_recv(buf, temptype);
  else /* subtype == TSEQUENCE */
    result = (Temporal *) tsequence_recv(buf, temptype);
  return result;
}

/**
 * @brief Write the binary representation of a temporal value into a buffer.
 * @note Function needed for temporal aggregation and thus only instant and
 * sequence subtypes must be considered
 */
void
temporal_write(const Temporal *temp, StringInfo buf)
{
  pq_sendbyte(buf, temp->temptype);
  pq_sendbyte(buf, temp->subtype);
  assert(temp->subtype == TINSTANT || temp->subtype == TSEQUENCE);
  if (temp->subtype == TINSTANT)
    tinstant_write((TInstant *) temp, buf);
  else /* temp->subtype == TSEQUENCE */
    tsequence_write((TSequence *) temp, buf);
  return;
}

PG_FUNCTION_INFO_V1(Temporal_recv);
/**
 * @ingroup mobilitydb_temporal_in_out
 * @brief Generic receive function for temporal types
 * @sqlfunc tbool_recv(), tint_recv(), tfloat_recv(), ttext_recv(),
 */
PGDLLEXPORT Datum
Temporal_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Temporal *result = temporal_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_send);
/*
 * @ingroup mobilitydb_temporal_in_out
 * @brief Generic send function for temporal types
 * @sqlfunc tbool_send(), tint_send(), tfloat_send(), ttext_send(),
 */
PGDLLEXPORT Datum
Temporal_send(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(temp);
  uint8_t *wkb = temporal_as_wkb(temp, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

PG_FUNCTION_INFO_V1(Tinstant_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal instant from the arguments
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst(),
 */
PGDLLEXPORT Datum
Tinstant_constructor(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  mobdbType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  Temporal *result = (Temporal *) tinstant_make(value, temptype, t);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Construct a temporal sequence from the array of temporal instants
 */
static Datum
tsequence_constructor_ext(FunctionCallInfo fcinfo, bool get_bounds,
  bool get_linear)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  bool lower_inc = (! get_bounds) ? true : PG_GETARG_BOOL(1);
  bool upper_inc = (! get_bounds) ? true : PG_GETARG_BOOL(2);
  bool linear = get_linear ? PG_GETARG_BOOL(3) : false;
  interpType interp = (! get_bounds) ? DISCRETE : (linear ? LINEAR : STEPWISE);
  ensure_non_empty_array(array);
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  Temporal *result = (Temporal *) tsequence_make((const TInstant **) instants,
    count, count, lower_inc, upper_inc, interp, NORMALIZE);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdiscseq_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence with discrete interpolation from the
 * array of temporal instants
 * @sqlfunc tbool_seq(), tint_seq(), ttext_seq(),
 */
PGDLLEXPORT Datum
Tdiscseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor_ext(fcinfo, false, false);
}

PG_FUNCTION_INFO_V1(Tstepseq_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence with stepwise interpolation from the
 * array of temporal instants
 * @sqlfunc tbool_seq(), tint_seq(), ttext_seq(),
 */
PGDLLEXPORT Datum
Tstepseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor_ext(fcinfo, true, false);
}

PG_FUNCTION_INFO_V1(Tlinearseq_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence with linear interpolation from the
 * array of temporal instants
 * @sqlfunc tfloat_seq()
 */
PGDLLEXPORT Datum
Tlinearseq_constructor(PG_FUNCTION_ARGS)
{
  return tsequence_constructor_ext(fcinfo, true, true);
}

PG_FUNCTION_INFO_V1(Tsequenceset_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence set from the array of temporal sequences
 * @sqlfunc tbool_seqset(), tint_seqset(), tfloat_seqset(), ttext_seqset(),
 */
PGDLLEXPORT Datum
Tsequenceset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TSequence **sequences = (TSequence **) temporalarr_extract(array, &count);
  Temporal *result = (Temporal *) tsequenceset_make(
    (const TSequence **) sequences, count, NORMALIZE);
  pfree(sequences);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Construct a temporal sequence set from the array of temporal instants that
 * are split in various sequences if two consecutive instants have a spatial
 * or temporal gap defined by the arguments
 */
Datum
tsequenceset_constructor_gaps_ext(FunctionCallInfo fcinfo, bool get_linear)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  interpType interp;
  float maxdist;
  Interval *maxt;
  if (get_linear)
  {
    interp = PG_GETARG_BOOL(1) ? LINEAR : STEPWISE;
    maxdist = PG_GETARG_FLOAT8(2);
    maxt = PG_GETARG_INTERVAL_P(3);
  }
  else
  {
    interp = STEPWISE;
    maxdist = PG_GETARG_FLOAT8(1);
    maxt = PG_GETARG_INTERVAL_P(2);
  }
  /* Store fcinfo into a global variable */
  /* Needed for the distance function for temporal geographic points */
  store_fcinfo(fcinfo);
  /* Extract the array of instants */
  int count;
  TInstant **instants = (TInstant **) temporalarr_extract(array, &count);
  TSequenceSet *result = tsequenceset_make_gaps((const TInstant **) instants,
    count, interp, maxdist, maxt);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tstepseqset_constructor_gaps);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence set with stepwise interpolation from the
 * array of temporal instants
 * @sqlfunc tbool_seqset_gaps(), tint_seqset_gaps(), ttext_seqset_gaps(),
 */
PGDLLEXPORT Datum
Tstepseqset_constructor_gaps(PG_FUNCTION_ARGS)
{
  return tsequenceset_constructor_gaps_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tlinearseqset_constructor_gaps);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence set with linear or stepwise interpolation
 * from the array of temporal instants
 * @sqlfunc tfloat_seqset_gaps()
 */
PGDLLEXPORT Datum
Tlinearseqset_constructor_gaps(PG_FUNCTION_ARGS)
{
  return tsequenceset_constructor_gaps_ext(fcinfo, true);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tdiscseq_from_base_time);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal discrete sequence from a base value and a
 * timestamp set
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq()
 */
PGDLLEXPORT Datum
Tdiscseq_from_base_time(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  mobdbType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequence *result = tdiscseq_from_base_time(value, temptype, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tsequence_from_base_time);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence from a base value and a period
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq()
 */
PGDLLEXPORT Datum
Tsequence_from_base_time(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Period *p = PG_GETARG_SPAN_P(1);
  interpType interp = LINEAR;
  if (PG_NARGS() > 2)
    interp = PG_GETARG_BOOL(2) ? LINEAR : STEPWISE;
  mobdbType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequence *result = tsequence_from_base_time(value, temptype, p, interp);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tsequenceset_from_base_time);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Construct a temporal sequence set from from a base value and a
 * timestamp set
 * @sqlfunc tbool_seqset(), tint_seqset(), tfloat_seqset(), ttext_seqset()
 */
PGDLLEXPORT Datum
Tsequenceset_from_base_time(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  interpType interp = LINEAR;
  if (PG_NARGS() > 2)
    interp = PG_GETARG_BOOL(2) ? LINEAR : STEPWISE;
  mobdbType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequenceSet *result = tsequenceset_from_base_time(value, temptype, ps,
    interp);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tint_to_tfloat);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Cast a temporal integer as a temporal float
 * @sqlfunc tfloat()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Tint_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tint_to_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tfloat_to_tint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Cast a temporal float as a temporal integer
 * @sqlfunc tint()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Tfloat_to_tint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_to_tint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_period);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Return the bounding period on which a temporal value is defined
 *
 * @note We cannot detoast only the header since we don't know whether the
 * lower and upper bounds of the period are inclusive or not
 * @sqlfunc period()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Temporal_to_period(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *result = palloc(sizeof(Period));
  temporal_set_period(temp, result);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SPAN_P(result);
}

PG_FUNCTION_INFO_V1(Tnumber_to_span);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Return the value span of a temporal integer
 * @sqlfunc span()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Tnumber_to_span(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *result = tnumber_to_span(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SPAN_P(result);
}

PG_FUNCTION_INFO_V1(Tnumber_to_tbox);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Return the bounding box of a temporal number
 * @sqlfunc tbox()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Tnumber_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  TBOX *result = palloc(sizeof(TBOX));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_subtype);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the string representation of the subtype of a temporal value
 * @sqlfunc tempSubtype()
 */
PGDLLEXPORT Datum
Temporal_subtype(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_subtype(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_interpolation);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the string representation of the interpolation of a temporal value
 * @sqlfunc interpolation()
 */
PGDLLEXPORT Datum
Temporal_interpolation(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_interpolation(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_memory_size);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the size in bytes of a temporal value
 * @sqlfunc memSize()
 */
PGDLLEXPORT Datum
Temporal_memory_size(PG_FUNCTION_ARGS)
{
  Datum result = toast_datum_size(PG_GETARG_DATUM(0));
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Tinstant_get_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base value of a temporal instant
 * @sqlfunc getValue()
 */
PGDLLEXPORT Datum
Tinstant_get_value(PG_FUNCTION_ARGS)
{
   TInstant *inst = PG_GETARG_TINSTANT_P(0);
  if (inst->subtype != TINSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must be of subtype instant")));

  Datum result = tinstant_value_copy(inst);
  PG_FREE_IF_COPY(inst, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_values);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base values of a temporal value as an array
 * @sqlfunc getValues()
 */
PGDLLEXPORT Datum
Temporal_values(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Datum *values = temporal_values(temp, &count);
  ArrayType *result = datumarr_to_array(values, count,
    temptype_basetype(temp->temptype));
  pfree(values);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tfloat_spans);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base values of a temporal float as an array of spans
 * @sqlfunc getValues()
 */
PGDLLEXPORT Datum
Tfloat_spans(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Span **spans = tfloat_spans(temp, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  pfree_array((void **) spans, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start base value of a temporal value
 * @sqlfunc startValue()
 */
PGDLLEXPORT Datum
Temporal_start_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_start_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end base value of a temporal value
 * @sqlfunc endValue()
 */
PGDLLEXPORT Datum
Temporal_end_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_end_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_min_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the minimum base value of a temporal value
 * @sqlfunc minValue()
 */
PGDLLEXPORT Datum
Temporal_min_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_min_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_max_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the maximum base value of a temporal value
 * @sqlfunc maxValue()
 */
PGDLLEXPORT Datum
Temporal_max_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_max_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Temporal_min_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return a pointer to the instant with the minimum base value of the
 * temporal value
 * @sqlfunc minInstant()
 */
PGDLLEXPORT Datum
Temporal_min_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_min_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_max_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return a pointer to the instant with maximum base value of the
 * temporal value.
 * @sqlfunc maxInstant()
 */
PGDLLEXPORT Datum
Temporal_max_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_max_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tinstant_timestamp);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the timestamp of a temporal instant
 * @sqlfunc getTimestamp()
 */
PGDLLEXPORT Datum
Tinstant_timestamp(PG_FUNCTION_ARGS)
{
  TInstant *inst = PG_GETARG_TINSTANT_P(0);
  if (inst->subtype != TINSTANT)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The temporal value must be of subtype instant")));

  TimestampTz result = inst->t;
  PG_FREE_IF_COPY(inst, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_time);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the time on which a temporal value is defined as a period set
 * @sqlfunc getTime()
 */
PGDLLEXPORT Datum
Temporal_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *result = temporal_time(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_timespan);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the timespan of a temporal value
 * @sqlfunc timespan()
 */
PGDLLEXPORT Datum
Temporal_timespan(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *result = temporal_timespan(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_duration);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the duration of a temporal value
 * @sqlfunc duration()
 */
PGDLLEXPORT Datum
Temporal_duration(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *result = temporal_duration(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_sequences);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of sequences of a temporal sequence (set)
 * @sqlfunc numSequences()
 */
PGDLLEXPORT Datum
Temporal_num_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_sequences(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_sequence);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start sequence of a temporal sequence (set)
 * @sqlfunc startSequence()
 */
PGDLLEXPORT Datum
Temporal_start_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_start_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_sequence);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end sequence of a temporal sequence (set)
 * @sqlfunc endSequence()
 */
PGDLLEXPORT Datum
Temporal_end_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_end_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_sequence_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th sequence of a temporal sequence (set)
 * @sqlfunc sequenceN()
 */
PGDLLEXPORT Datum
Temporal_sequence_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TSequence *result = temporal_sequence_n(temp, n);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_sequences);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the sequences of a temporal sequence (set) as an array
 * @sqlfunc sequences()
 */
PGDLLEXPORT Datum
Temporal_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TSequence **sequences = temporal_sequences(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) sequences,
    count);
  pfree_array((void **) sequences, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_segments);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the segments of a temporal sequence (set) as an array
 * @sqlfunc segments()
 */
PGDLLEXPORT Datum
Temporal_segments(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TSequence **segments = temporal_segments(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) segments,
    count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_instants);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of distinct instants of a temporal value
 * @sqlfunc numInstants()
 */
PGDLLEXPORT Datum
Temporal_num_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_instants(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start instant of a temporal value
 * @sqlfunc startInstant()
 */
PGDLLEXPORT Datum
Temporal_start_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_start_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end instant of a temporal value
 * @sqlfunc endInstant()
 */
PGDLLEXPORT Datum
Temporal_end_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = tinstant_copy(temporal_end_instant(temp));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_instant_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th instant of a temporal value
 * @sqlfunc instantN()
 */
PGDLLEXPORT Datum
Temporal_instant_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  const TInstant *inst = temporal_instant_n(temp, n);
  TInstant *result = (inst == NULL) ? NULL : tinstant_copy(inst);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_instants);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the distinct instants of a temporal value as an array
 * @sqlfunc instants()
 */
PGDLLEXPORT Datum
Temporal_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  const TInstant **instants = temporal_instants(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) instants,
    count);
  pfree(instants);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_num_timestamps);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of distinct timestamps of a temporal value
 * @sqlfunc numTimestamps()
 */
PGDLLEXPORT Datum
Temporal_num_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_timestamps(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_start_timestamp);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start timestamp of a temporal value
 * @sqlfunc startTimestamp()
 */
PGDLLEXPORT Datum
Temporal_start_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_start_timestamp(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_end_timestamp);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end timestamp of a temporal value
 * @sqlfunc endTimestamp()
 */
PGDLLEXPORT Datum
Temporal_end_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_end_timestamp(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_timestamp_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th distinct timestamp of a temporal value
 * @sqlfunc timestampN()
 */
PGDLLEXPORT Datum
Temporal_timestamp_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = temporal_timestamp_n(temp, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Temporal_timestamps);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the distinct timestamps of a temporal value as an array
 * @sqlfunc timestamps()
 */
PGDLLEXPORT Datum
Temporal_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TimestampTz *times = temporal_timestamps(temp, &count);
  ArrayType *result = timestamparr_to_array(times, count);
  pfree(times);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators

 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
temporal_ev_al_comp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  bool result = func(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, temptype_basetype(temp->temptype), 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ever_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever equal to a base value
 * @sqlfunc ever_eq()
 * @sqlop @p ?=
 */
PGDLLEXPORT Datum
Temporal_ever_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_eq);
}

PG_FUNCTION_INFO_V1(Temporal_always_eq);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always equal to the base value
 * @sqlfunc always_eq()
 * @sqlop @p %=
 */
PGDLLEXPORT Datum
Temporal_always_eq(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_eq);
}

PG_FUNCTION_INFO_V1(Temporal_ever_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever different from a base value
 * @sqlfunc ever_eq()
 * @sqlop @p ?<>
 */
PGDLLEXPORT Datum
Temporal_ever_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_eq);
}

PG_FUNCTION_INFO_V1(Temporal_always_ne);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always different from a base value
 * @sqlfunc always_ne()
 * @sqlop @p %<>
 */
PGDLLEXPORT Datum
Temporal_always_ne(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_eq);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_ever_lt);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever less than a base value
 * @sqlfunc ever_lt()
 * @sqlop @p ?<
 */
PGDLLEXPORT Datum
Temporal_ever_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_lt);
}

PG_FUNCTION_INFO_V1(Temporal_always_lt);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always less than a base value
 * @sqlfunc always_lt()
 * @sqlop @p %<
 */
PGDLLEXPORT Datum
Temporal_always_lt(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_lt);
}

PG_FUNCTION_INFO_V1(Temporal_ever_le);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever less than or equal to a base value
 * @sqlfunc ever_le()
 * @sqlop @p ?<=
 */
PGDLLEXPORT Datum
Temporal_ever_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_ever_le);
}

PG_FUNCTION_INFO_V1(Temporal_always_le);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always less than or equal to a base value
 * @sqlfunc always_le()
 * @sqlop @p %<=
 */
PGDLLEXPORT Datum
Temporal_always_le(PG_FUNCTION_ARGS)
{
  return temporal_ev_al_comp_ext(fcinfo, &temporal_always_le);
}

PG_FUNCTION_INFO_V1(Temporal_ever_gt);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever greater than a base value
 * @sqlfunc ever_gt()
 * @sqlop @p ?>
 */
PGDLLEXPORT Datum
Temporal_ever_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_le);
}

PG_FUNCTION_INFO_V1(Temporal_always_gt);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always greater than a base value
 * @sqlfunc always_gt()
 * @sqlop @p %>
 */
PGDLLEXPORT Datum
Temporal_always_gt(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_le);
}

PG_FUNCTION_INFO_V1(Temporal_ever_ge);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is ever greater than or equal
 * to a base value
 * @sqlfunc ever_ge()
 * @sqlop @p ?>=
 */
PGDLLEXPORT Datum
Temporal_ever_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_always_lt);
}

PG_FUNCTION_INFO_V1(Temporal_always_ge);
/**
 * @ingroup mobilitydb_temporal_ever
 * @brief Return true if a temporal value is always greater than or equal
 * to a base value
 * @sqlfunc always_ge()
 * @sqlop @p %>=
 */
PGDLLEXPORT Datum
Temporal_always_ge(PG_FUNCTION_ARGS)
{
  return ! temporal_ev_al_comp_ext(fcinfo, &temporal_ever_lt);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_to_tinstant);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transform a temporal value into a temporal instant
 * @sqlfunc tbool_inst(), tint_inst(), tfloat_inst(), ttext_inst()
 */
PGDLLEXPORT Datum
Temporal_to_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_to_tinstant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tdiscseq);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transform a temporal value into a temporal sequence with discrete
 * interpolation
 * @sqlfunc tbool_discseq(), tint_discseq(), tfloat_discseq(), ttext_discseq()
 */
PGDLLEXPORT Datum
Temporal_to_tdiscseq(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_to_tdiscseq(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tsequence);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transform a temporal value into a temporal sequence
 * @sqlfunc tbool_seq(), tint_seq(), tfloat_seq(), ttext_seq()
 */
PGDLLEXPORT Datum
Temporal_to_tsequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_to_tsequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_to_tsequenceset);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transform a temporal value into a temporal sequence set
 * @sqlfunc tbool_seqset(), tint_seqset(), tfloat_seqset(), ttext_seqset()
 */
PGDLLEXPORT Datum
Temporal_to_tsequenceset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_to_tsequenceset(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tempstep_to_templinear);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Transform a temporal value with continuous base type from stepwise
 * to linear interpolation
 * @sqlfunc toLinear  ()
 */
PGDLLEXPORT Datum
Tempstep_to_templinear(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_step_to_linear(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_shift);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value a shifted by an interval
 * @sqlfunc shift()
 */
PGDLLEXPORT Datum
Temporal_shift(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_tscale(temp, shift, NULL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_tscale);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value scaled by an interval
 * @sqlfunc tscale()
 */
PGDLLEXPORT Datum
Temporal_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_tscale(temp, NULL, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_shift_tscale);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value shifted and scaled by the intervals
 * @sqlfunc shiftTscale()
 */
PGDLLEXPORT Datum
Temporal_shift_tscale(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Temporal *result = temporal_shift_tscale(temp, shift, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_append_tinstant);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Append an instant to the end of a temporal value
 * @sqlfunc appendInstant()
 */
PGDLLEXPORT Datum
Temporal_append_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *inst = PG_GETARG_TINSTANT_P(1);
  Temporal *result = temporal_append_tinstant(temp, (TInstant *) inst, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(inst, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_merge);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Merge the two temporal values
 * @sqlfunc merge()
 */
PGDLLEXPORT Datum
Temporal_merge(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL_P(1);
  Temporal *result = temporal_merge(temp1, temp2);
  if (temp1)
    PG_FREE_IF_COPY(temp1, 0);
  if (temp2)
    PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_merge_array);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Merge the array of temporal values
 * @sqlfunc merge()
 */
PGDLLEXPORT Datum
Temporal_merge_array(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Temporal **temparr = temporalarr_extract(array, &count);
  Temporal *result = temporal_merge_array(temparr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @brief Restrict a temporal value to (the complement of) an array of base values
 */
static Datum
temporal_restrict_value_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Temporal *result = temporal_restrict_value(temp, value, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_value);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a base value
 * @sqlfunc atValue()
 */
PGDLLEXPORT Datum
Temporal_at_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_value);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a base value
 * @sqlfunc minusValue()
 */
PGDLLEXPORT Datum
Temporal_minus_value(PG_FUNCTION_ARGS)
{
  return temporal_restrict_value_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/* Helper macro to input a temporal and an array for restriction operations */
#define INPUT_REST_ARRAY(temp, array, count) \
  do { \
    temp = PG_GETARG_TEMPORAL_P(0);  \
    array = PG_GETARG_ARRAYTYPE_P(1); \
    /* Return NULL or a copy of a temporal value on empty array */ \
    count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)); \
    if (count == 0) \
    { \
      PG_FREE_IF_COPY(array, 1); \
      if (atfunc) \
      { \
        PG_FREE_IF_COPY(temp, 0); \
        PG_RETURN_NULL(); \
      } \
      else \
      { \
        Temporal *result = temporal_copy(temp); \
        PG_FREE_IF_COPY(temp, 0); \
        PG_RETURN_POINTER(result); \
      } \
    } \
  } while(0);


/**
 * @brief Restrict a temporal value to (the complement of) an array of base values
 */
static Datum
temporal_restrict_values_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp;
  ArrayType *array;
  int count;
  INPUT_REST_ARRAY(temp, array, count);
  Datum *values = datumarr_extract(array, &count);
  /* For temporal points the validity of values in the array is done in
   * bounding box function */
  Temporal *result = (count > 1) ?
    temporal_restrict_values(temp, values, count, atfunc) :
    temporal_restrict_value(temp, values[0], atfunc);

  pfree(values);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_values);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to an array of base values
 * @sqlfunc atValues()
 */
PGDLLEXPORT Datum
Temporal_at_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_values);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of an array of base values
 * @sqlfunc minusValues()
 */
PGDLLEXPORT Datum
Temporal_minus_values(PG_FUNCTION_ARGS)
{
  return temporal_restrict_values_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

static Datum
tnumber_restrict_span_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *span = PG_GETARG_SPAN_P(1);
  Temporal *result = tnumber_restrict_span(temp, span, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_span);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a span of base values
 * @sqlfunc atSpan()
 */
PGDLLEXPORT Datum
Tnumber_at_span(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_span_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_span);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a span of base values
 * @sqlfunc minusSpan()
 */
PGDLLEXPORT Datum
Tnumber_minus_span(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_span_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal value to (the complement of) an array of spans
 * of base values
 */
static Datum
tnumber_restrict_spans_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp;
  ArrayType *array;
  int count;
  INPUT_REST_ARRAY(temp, array, count);
  Span **spans = spanarr_extract(array, &count);
  Temporal *result = tnumber_restrict_spans(temp, spans, count, atfunc);
  pfree(spans);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_spans);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to an array of spans of base values
 * @sqlfunc atSpans()
 */
PGDLLEXPORT Datum
Tnumber_at_spans(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_spans_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_spans);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of an array of spans
 * of base values
 * @sqlfunc minusSpans()
 */
PGDLLEXPORT Datum
Tnumber_minus_spans(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_spans_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_min);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to its minimum base value
 * @sqlfunc atMin()
 */
PGDLLEXPORT Datum
Temporal_at_min(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_min);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of its minimum base value
 * @sqlfunc minusMin()
 */
PGDLLEXPORT Datum
Temporal_minus_min(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MIN, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_max);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to its maximum base value
 * @sqlfunc atMax()
 */
PGDLLEXPORT Datum
Temporal_at_max(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_AT);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_max);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of its maximum base value
 * @sqlfunc minusMax()
 */
PGDLLEXPORT Datum
Temporal_minus_max(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, GET_MAX, REST_MINUS);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal value to (the complement of) a temporal box
  */
static Datum
tnumber_restrict_tbox_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  Temporal *result = atfunc ? tnumber_at_tbox(temp, box) :
    tnumber_minus_tbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_at_tbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a temporal box
 * @sqlfunc atTbox()
 */
PGDLLEXPORT Datum
Tnumber_at_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tnumber_minus_tbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a temporal box
 * @sqlfunc minusTbox()
 */
PGDLLEXPORT Datum
Tnumber_minus_tbox(PG_FUNCTION_ARGS)
{
  return tnumber_restrict_tbox_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Restrict a temporal value to (the complement of) a timestamp
 */
static Datum
temporal_restrict_timestamp_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = temporal_restrict_timestamp(temp, t, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_timestamp);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a timestamp
 * @sqlfunc atTimestamp()
 */
PGDLLEXPORT Datum
Temporal_at_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_timestamp);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a timestamp
 * @sqlfunc minusTimestamp()
 */
PGDLLEXPORT Datum
Temporal_minus_timestamp(PG_FUNCTION_ARGS)
{
  return temporal_restrict_timestamp_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_value_at_timestamp);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base value of a temporal value at the timestamp
 * @sqlfunc valueAtTimestamp()
 */
PGDLLEXPORT Datum
Temporal_value_at_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Datum result;
  bool found = temporal_value_at_timestamp(temp, t, true, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_timestampset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a timestamp set
 * @sqlfunc atTimestampSet()
 */
PGDLLEXPORT Datum
Temporal_at_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_timestampset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a timestamp set
 * @sqlfunc minusTimestampSet()
 */
PGDLLEXPORT Datum
Temporal_minus_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  Temporal *result = temporal_restrict_timestampset(temp, ts, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

static Datum
temporal_restrict_period_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  Temporal *result = temporal_restrict_period(temp, p, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_at_period);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a period
 * @sqlfunc atPeriod()
 */
PGDLLEXPORT Datum
Temporal_at_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Temporal_minus_period);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a period
 * @sqlfunc minusPeriod()
 */
PGDLLEXPORT Datum
Temporal_minus_period(PG_FUNCTION_ARGS)
{
  return temporal_restrict_period_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_at_periodset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to a period set
 * @sqlfunc atPeriodSet()
 */
PGDLLEXPORT Datum
Temporal_at_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Temporal_minus_periodset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal value to the complement of a period set
 * @sqlfunc minusPeriodSet()
 */
PGDLLEXPORT Datum
Temporal_minus_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  Temporal *result = temporal_restrict_periodset(temp, ps, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_intersects_timestamp);
/**
 * @ingroup mobilitydb_temporal_time
 * @brief Return true if a temporal value intersects a timestamp
 * @sqlfunc intersectsTimestamp()
 */
PGDLLEXPORT Datum
Temporal_intersects_timestamp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = temporal_intersects_timestamp(temp, t);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_timestampset);
/**
 * @ingroup mobilitydb_temporal_time
 * @brief Return true if a temporal value intersects a timestamp set
 * @sqlfunc intersectsTimestampSet()
 */
PGDLLEXPORT Datum
Temporal_intersects_timestampset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = temporal_intersects_timestampset(temp, ts);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_period);
/**
 * @ingroup mobilitydb_temporal_time
 * @brief Return true if a temporal value intersects a period
 * @sqlfunc intersectsPeriod  ()
 */
PGDLLEXPORT Datum
Temporal_intersects_period(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = temporal_intersects_period(temp, p);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_intersects_periodset);
/**
 * @ingroup mobilitydb_temporal_time
 * @brief Return true if a temporal value intersects a period set
 * @sqlfunc intersectsPeriodSet()
 */
PGDLLEXPORT Datum
Temporal_intersects_periodset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = temporal_intersects_periodset(temp, ps);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_integral);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal number
 * @sqlfunc integral()
 */
PGDLLEXPORT Datum
Tnumber_integral(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_integral(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tnumber_twavg);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighted average of a temporal number
 * @sqlfunc twAvg()
 */
PGDLLEXPORT Datum
Tnumber_twavg(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_twavg(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_eq);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the temporal values are equal
 * @sqlfunc tbool_eq(), tint_eq(), tfloat_eq(), ttext_eq(),
 * tgeompoint_eq(), tgeogpoint_eq(), tnpoint_eq()
 * @sqlop =
 */
PGDLLEXPORT Datum
Temporal_eq(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_eq(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ne);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the temporal values are different
 * @sqlfunc tbool_ne(), tint_ne(), tfloat_ne(), ttext_ne(),
 * tgeompoint_ne(), tgeogpoint_ne(), tnpoint_ne()
 * @sqlop <>
 */
PGDLLEXPORT Datum
Temporal_ne(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_ne(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_cmp);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal, or greater than the second temporal value
 * @note Function used for B-tree comparison
 * @sqlfunc tbool_cmp(), tint_cmp(), tfloat_cmp(), ttext_cmp(),
 * tgeompoint_cmp(), tgeogpoint_cmp(), tnpoint_cmp()
 */
PGDLLEXPORT Datum
Temporal_cmp(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = temporal_cmp(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Temporal_lt);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the first temporal value is less than the second one
 * @sqlfunc tbool_lt(), tint_lt(), tfloat_lt(), ttext_lt(),
 * tgeompoint_lt(), tgeogpoint_lt(), tnpoint_lt()
 * @sqlop @p <
 */
PGDLLEXPORT Datum
Temporal_lt(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_lt(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_le);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 * @sqlfunc tbool_le(), tint_le(), tfloat_le(), ttext_le(),
 * tgeompoint_le(), tgeogpoint_le(), tnpoint_le()
 * @sqlop @p <=
 */
PGDLLEXPORT Datum
Temporal_le(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_le(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_ge);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 * @sqlfunc tbool_ge(), tint_ge(), tfloat_ge(), ttext_ge(),
 * tgeompoint_ge(), tgeogpoint_ge(), tnpoint_ge()
 * @sqlop =@p >=
 */
PGDLLEXPORT Datum
Temporal_ge(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_ge(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Temporal_gt);
/**
 * @ingroup mobilitydb_temporal_comp
 * @brief Return true if the first temporal value is greater than the second one
 * @sqlfunc tbool_gt(), tint_gt(), tfloat_gt(), ttext_gt(),
 * tgeompoint_gt(), tgeogpoint_gt(), tnpoint_gt()
 * @sqlop @p >
 */
PGDLLEXPORT Datum
Temporal_gt(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_gt(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Functions for defining hash index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_hash);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the hash value of a temporal value
 * @sqlfunc tbool_hash(), tint_hash(), tfloat_hash(), ttext_hash(),
 * tgeompoint_hash(), tgeogpoint_hash(), tnpoint_hash()
 */
PGDLLEXPORT Datum
Temporal_hash(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32 result = temporal_hash(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_UINT32(result);
}

/*****************************************************************************/
