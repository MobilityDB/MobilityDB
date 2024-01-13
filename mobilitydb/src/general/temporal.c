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

#include "pg_general/temporal.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <access/heaptoast.h>
  #include <access/detoast.h>
#else
  #include <access/tuptoaster.h>
#endif
#include <libpq/pqformat.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/tbox.h"
#include "general/temporal.h"
#include "general/temporal_boxops.h"
#include "general/type_out.h"
#include "general/type_util.h"
#include "point/tpoint.h"
/* MobilityDB */
#include "pg_general/doxygen_mobilitydb_api.h"
#include "pg_general/meos_catalog.h"
#include "pg_general/type_util.h"
#include "pg_point/tpoint_spatialfuncs.h"

/* To avoid including fmgrprotos.h */
extern PGDLLEXPORT Datum timestamp_mi(PG_FUNCTION_ARGS);
extern PGDLLEXPORT Datum interval_cmp(PG_FUNCTION_ARGS);

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
  mobilitydb_init();
  return;
}

/*****************************************************************************
 * Miscellaneous
 *****************************************************************************/

/**
 * @brief Return the size in bytes to read from toast to get the basic
 * information from a variable-length time type: Time struct (i.e., Set
 * or SpanSet) and bounding box size
*/
uint32_t
time_max_header_size(void)
{
  return DOUBLE_PAD(Max(sizeof(Set), sizeof(SpanSet)));
}

/*****************************************************************************
 * Global variables
 *****************************************************************************/

/**
 * @brief Global variable that saves the PostgreSQL fcinfo
 *
 * This is needed when we need to change the PostgreSQL context, for example,
 * in PostGIS functions such as #transform, #geography_distance, or
 * #geography_azimuth that need to access the proj cache
 */
FunctionCallInfo _FCINFO;

/**
 * @brief Fetch from the cache the fcinfo of the external function
 */
FunctionCallInfo
fetch_fcinfo()
{
  assert(_FCINFO);
  return _FCINFO;
}

/**
 * @brief Store in the cache the fcinfo of the external function
 */
void
store_fcinfo(FunctionCallInfo fcinfo)
{
  _FCINFO = fcinfo;
  return;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that the array is not empty
 * @note Used for the constructor functions
 */
bool
ensure_not_empty_array(ArrayType *array)
{
  if (ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array)) == 0)
  {
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("The input array cannot be empty")));
    return false;
  }
  return true;
}

/*****************************************************************************
 * Typmod functions
 *****************************************************************************/

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

PGDLLEXPORT Datum Temporal_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_typmod_in);
/**
 * @brief Input typmod information for temporal types
 */
Datum
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

PGDLLEXPORT Datum Temporal_typmod_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_typmod_out);
/**
 * @brief Output typmod information for temporal types
 */
Datum
Temporal_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  int16 subtype = TYPMOD_GET_SUBTYPE(typmod);
  /* No type? Then no typmod at all. Return empty string.  */
  if (typmod < 0 || ! subtype)
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  sprintf(str, "(%s)", tempsubtype_name(subtype));
  PG_RETURN_CSTRING(s);
}

PGDLLEXPORT Datum Temporal_enforce_typmod(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal types
 */
Datum
Temporal_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if temporal typmod is consistent with the supplied one */
  temp = temporal_valid_typmod(temp, typmod);
  PG_RETURN_TEMPORAL_P(temp);
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Peek into a temporal datum to find the bounding box
 * @note If the datum needs to be detoasted, extract only the header and not
 * the full object
 */
void
temporal_bbox_slice(Datum tempdatum, void *box)
{
  Temporal *tempslice = NULL;
  int need_detoast = PG_DATUM_NEEDS_DETOAST((struct varlena *) tempdatum);
  if (need_detoast)
    tempslice = (Temporal *) PG_DETOAST_DATUM_SLICE(tempdatum, 0,
      temporal_max_header_size());
  else
    tempslice = (Temporal *) tempdatum;
  if (need_detoast && tempslice->subtype == TINSTANT)
  {
    /* TInstant subtype of Temporal DOES NOT keep the bounding box, so
     * we now detoast it completely */
    tempslice = (Temporal *) PG_DETOAST_DATUM(tempdatum);
  }
  temporal_set_bbox(tempslice, box);
  return;
}

/*****************************************************************************
 * Version functions
 *****************************************************************************/

PGDLLEXPORT Datum Mobilitydb_version(PG_FUNCTION_ARGS __attribute__((unused)));
PG_FUNCTION_INFO_V1(Mobilitydb_version);
/**
 * @ingroup mobilitydb_misc
 * @brief Return the version of the MobilityDB extension
 * @sqlfn mobilitydb_version()
 */
Datum
Mobilitydb_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char *version = mobilitydb_version();
  text *result = cstring2text(version);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Mobilitydb_full_version(PG_FUNCTION_ARGS __attribute__((unused)));
PG_FUNCTION_INFO_V1(Mobilitydb_full_version);
/**
 * @ingroup mobilitydb_misc
 * @brief Return the versions of the MobilityDB extension and its dependencies
 * @sqlfn mobilitydb_full_version()
 */
Datum
Mobilitydb_full_version(PG_FUNCTION_ARGS __attribute__((unused)))
{
  char *version = mobilitydb_full_version();
  text *result = cstring2text(version);
  pfree(version);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Send and receive functions
 * The send and receive functions are needed for temporal aggregation
 *****************************************************************************/

/**
 * @brief Return a temporal instant from its binary representation read from
 * a buffer
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_recv(StringInfo buf, meosType temptype)
{
  TimestampTz t = call_recv(T_TIMESTAMPTZ, buf);
  int size = pq_getmsgint(buf, 4);
  StringInfoData buf2 =
  {
    .cursor = 0,
    .len = size,
    .maxlen = size,
    .data = buf->data + buf->cursor
  };
  meosType basetype = temptype_basetype(temptype);
  Datum value = call_recv(basetype, &buf2);
  buf->cursor += size;
  return tinstant_make(value, temptype, t);
}

/**
 * @brief Write the binary representation of a temporal instant into a buffer
 * @param[in] inst Temporal instant
 * @param[in] buf Buffer
 */
void
tinstant_write(const TInstant *inst, StringInfo buf)
{
  meosType basetype = temptype_basetype(inst->temptype);
  bytea *bt = call_send(T_TIMESTAMPTZ, TimestampTzGetDatum(inst->t));
  bytea *bv = call_send(basetype, tinstant_val(inst));
  pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
  pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
  return;
}

/*****************************************************************************/

/**
 * @brief Return a temporal sequence from its binary representation read from
 * a buffer
 * @param[in] buf Buffer
 * @param[in] temptype Temporal type
 */
TSequence *
tsequence_recv(StringInfo buf, meosType temptype)
{
  int count = (int) pq_getmsgint(buf, 4);
  bool lower_inc = (char) pq_getmsgbyte(buf);
  bool upper_inc = (char) pq_getmsgbyte(buf);
  interpType interp = (char) pq_getmsgbyte(buf);
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
    instants[i] = tinstant_recv(buf, temptype);
  return tsequence_make_free(instants, count, lower_inc, upper_inc, interp,
    NORMALIZE);
}

/**
 * @brief Write the binary representation of a temporal sequence into a buffer
 * @param[in] seq Temporal sequence
 * @param[in] buf Buffer
 */
void
tsequence_write(const TSequence *seq, StringInfo buf)
{
  pq_sendint32(buf, seq->count);
  pq_sendbyte(buf, seq->period.lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, seq->period.upper_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, (uint8) MEOS_FLAGS_GET_INTERP(seq->flags));
  for (int i = 0; i < seq->count; i++)
    tinstant_write(TSEQUENCE_INST_N(seq, i), buf);
  return;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal value from its Well-Known Text (WKT) representation
 * @sqlfn int_in(), tfloat_in(), ...
 */
Datum
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
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_out);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal value
 * @sqlfn tint_out(), tfloat_out(), ...
 */
Datum
Temporal_out(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *result = temporal_out(temp, OUT_DEFAULT_DECIMAL_DIGITS);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_CSTRING(result);
}

/**
 * @brief Return a temporal value from its binary representation read from
 * a buffer
 * @note Function needed for temporal aggregation and thus only instant and
 * sequence subtypes must be considered
 */
Temporal *
temporal_recv(StringInfo buf)
{
  uint8 temptype = (uint8) pq_getmsgbyte(buf);
  uint8 subtype = (uint8) pq_getmsgbyte(buf);
  Temporal *result;
  assert(subtype == TINSTANT || subtype == TSEQUENCE);
  if (subtype == TINSTANT)
    result = (Temporal *) tinstant_recv(buf, temptype);
  else /* subtype == TSEQUENCE */
    result = (Temporal *) tsequence_recv(buf, temptype);
  return result;
}

/**
 * @brief Write the binary representation of a temporal value into a buffer
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

PGDLLEXPORT Datum Temporal_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_recv);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal value from its Well-Known Binary (WKB)
 * representation
 * @sqlfn tint_recv(), tfloat_recv(), ...
 */
Datum
Temporal_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Temporal *result = temporal_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_send);
/*
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal value
 * @sqlfn tint_send(), tfloat_send(), ...
 */
Datum
Temporal_send(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Add SRID to binary representation */
  uint8_t variant = WKB_EXTENDED;
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

PGDLLEXPORT Datum Tinstant_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tinstant_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal instant from a value and a timestamptz
 * @sqlfn tint_inst(), tfloat_inst(), ...
 */
Datum
Tinstant_constructor(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  PG_RETURN_TINSTANT_P(tinstant_make(value, temptype, t));
}

PGDLLEXPORT Datum Tsequence_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequence_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal sequence from an array of temporal instants
 * @sqlfn tint_seq(), tfloat_seq(), ...
 */
Datum
Tsequence_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 1 && !PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  bool lower_inc = true, upper_inc = true;
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
    lower_inc = PG_GETARG_BOOL(2);
  if (PG_NARGS() > 3 && !PG_ARGISNULL(3))
    upper_inc = PG_GETARG_BOOL(3);
  int count;
  TInstant **instants = (TInstant **) temparr_extract(array, &count);
  TSequence *result = tsequence_make((const TInstant **) instants, count,
    lower_inc, upper_inc, interp, NORMALIZE);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Tsequenceset_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequenceset_constructor);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal sequences
 * @sqlfn tint_seqset(), tfloat_seqset(), ...
 */
Datum
Tsequenceset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  int count;
  TSequence **sequences = (TSequence **) temparr_extract(array, &count);
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    count, NORMALIZE);
  pfree(sequences);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_TSEQUENCESET_P(result);
}

PGDLLEXPORT Datum Tsequenceset_constructor_gaps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequenceset_constructor_gaps);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal sequence set from an array of temporal instants
 * accounting for potential gaps
 * @note The SQL function is not strict
 * @sqlfn tint_seqset_gaps(), tfloat_seqset_gaps(), ...
 */
Datum
Tsequenceset_constructor_gaps(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  double maxdist = -1.0;
  Interval *maxt = NULL;
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    maxt = PG_GETARG_INTERVAL_P(1);
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    maxdist = PG_GETARG_FLOAT8(2);
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    text *interp_txt = PG_GETARG_TEXT_P(3);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  /* Store fcinfo into a global variable */
  /* Needed for the distance function for temporal geography points */
  store_fcinfo(fcinfo);
  /* Extract the array of instants */
  int count;
  TInstant **instants = (TInstant **) temparr_extract(array, &count);
  TSequenceSet *result = tsequenceset_make_gaps((const TInstant **) instants,
    count, interp, maxt, maxdist);
  pfree(instants);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_TSEQUENCESET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tsequence_from_base_tstzset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequence_from_base_tstzset);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal discrete sequence from a base value and a
 * timestamptz set
 * @sqlfn tint_discseq(), tfloat_discseq(), ...
 */
Datum
Tsequence_from_base_tstzset(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  TSequence *result = tsequence_from_base_tstzset(value, temptype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Tsequence_from_base_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequence_from_base_tstzspan);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal sequence from a base value and a timestamptz span
 * @sqlfn tint_seq(), tfloat_seq(), ...
 */
Datum
Tsequence_from_base_tstzspan(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
  {
    text *interp_txt = PG_GETARG_TEXT_P(2);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  PG_RETURN_TSEQUENCE_P(tsequence_from_base_tstzspan(value, temptype, s,
    interp));
}

PGDLLEXPORT Datum Tsequenceset_from_base_tstzspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tsequenceset_from_base_tstzspanset);
/**
 * @ingroup mobilitydb_temporal_constructor
 * @brief Return a temporal sequence set from a base value and a
 * timestamptz set
 * @sqlfn tint_seqset(), tfloat_seqset(), ...
 */
Datum
Tsequenceset_from_base_tstzspanset(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_ANYDATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  interpType interp = temptype_continuous(temptype) ? LINEAR : STEP;
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
  {
    text *interp_txt = PG_GETARG_TEXT_P(2);
    char *interp_str = text2cstring(interp_txt);
    interp = interptype_from_string(interp_str);
    pfree(interp_str);
  }
  TSequenceSet *result = tsequenceset_from_base_tstzspanset(value, temptype,
    ss, interp);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_TSEQUENCESET_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tint_to_tfloat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_to_tfloat);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal integer converted to a temporal float
 * @sqlfn tfloat()
 * @sqlop @p ::
 */
Datum
Tint_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tint_to_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloat_to_tint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_to_tint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal float converted to a temporal integer
 * @sqlfn tint()
 * @sqlop @p ::
 */
Datum
Tfloat_to_tint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_to_tint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_to_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_to_tstzspan);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal value converted to a timestamptz span
 * @sqlfn timeSpan()
 * @sqlop @p ::
 */
Datum
Temporal_to_tstzspan(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Tnumber_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_to_span);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal number converted to a value span
 * @sqlfn valueSpan()
 * @sqlop @p ::
 */
Datum
Tnumber_to_span(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *result = tnumber_to_span(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Tnumber_to_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_to_tbox);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal number converted to a temporal box
 * @sqlfn tbox()
 * @sqlop @p ::
 */
Datum
Tnumber_to_tbox(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  TBox *result = palloc(sizeof(TBox));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_subtype(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_subtype);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the subtype of a temporal value as a string
 * @sqlfn tempSubtype()
 */
Datum
Temporal_subtype(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  const char *str = temporal_subtype(temp);
  text *result = cstring2text(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Temporal_interp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_interp);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the interpolation of a temporal value as a string
 * @sqlfn interp()
 */
Datum
Temporal_interp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  const char *str = temporal_interp(temp);
  text *result = cstring2text(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Temporal_mem_size(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_mem_size);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the memory size in bytes of a temporal value
 * @sqlfn memSize()
 */
Datum
Temporal_mem_size(PG_FUNCTION_ARGS)
{
  Datum result = toast_raw_datum_size(PG_GETARG_DATUM(0));
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Tinstant_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tinstant_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base value of a temporal instant
 * @sqlfn getValue()
 */
Datum
Tinstant_value(PG_FUNCTION_ARGS)
{
  TInstant *inst = PG_GETARG_TINSTANT_P(0);
  /* Ensure validity of arguments */
  ensure_temporal_isof_subtype((Temporal *) inst, TINSTANT);

  Datum result = tinstant_value(inst);
  PG_FREE_IF_COPY(inst, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Temporal_valueset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_valueset);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base values of a temporal value as a set
 * @sqlfn getValues()
 */
Datum
Temporal_valueset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Datum *values = temporal_vals(temp, &count);
  meosType basetype = temptype_basetype(temp->temptype);
  /* Currently, there is no boolset type */
  if (temp->temptype == T_TBOOL)
  {
    ArrayType *result = datumarr_to_array(values, count, basetype);
    pfree(values);
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_ARRAYTYPE_P(result);
  }
  Set *result = set_make_free(values, count, basetype, ORDERED);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Tnumber_valuespans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_valuespans);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the value span set of a temporal number
 * @sqlfn getValues()
 */
Datum
Tnumber_valuespans(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *result = tnumber_valuespans(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Temporal_start_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_start_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start base value of a temporal value
 * @sqlfn startValue()
 */
Datum
Temporal_start_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_start_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Temporal_end_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_end_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end base value of a temporal value
 * @sqlfn endValue()
 */
Datum
Temporal_end_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_end_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Temporal_min_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_min_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the minimum base value of a temporal value
 * @sqlfn minValue()
 */
Datum
Temporal_min_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_min_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Temporal_max_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_max_value);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the maximum base value of a temporal value
 * @sqlfn maxValue()
 */
Datum
Temporal_max_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = temporal_max_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Temporal_min_instant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_min_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the instant with the minimum base value of a temporal value
 * @sqlfn minInstant()
 */
Datum
Temporal_min_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = temporal_min_instant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Temporal_max_instant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_max_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the instant with maximum base value of a temporal value
 * @sqlfn maxInstant()
 */
Datum
Temporal_max_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = temporal_max_instant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Tinstant_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tinstant_timestamptz);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the timestamptz of a temporal instant
 * @sqlfn getTimestamp()
 */
Datum
Tinstant_timestamptz(PG_FUNCTION_ARGS)
{
  TInstant *inst = PG_GETARG_TINSTANT_P(0);
  /* Ensure validity of arguments */
  ensure_temporal_isof_subtype((Temporal *) inst, TINSTANT);

  TimestampTz result = inst->t;
  PG_FREE_IF_COPY(inst, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Temporal_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_time);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the time on which a temporal value is defined as a span set
 * @sqlfn getTime()
 */
Datum
Temporal_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *result = temporal_time(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Temporal_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_duration);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the duration of a temporal value
 * @sqlfn duration()
 */
Datum
Temporal_duration(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool boundspan = PG_GETARG_BOOL(1);
  Interval *result = temporal_duration(temp, boundspan);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INTERVAL_P(result);
}

PGDLLEXPORT Datum Temporal_num_sequences(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_num_sequences);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of sequences of a temporal sequence (set)
 * @sqlfn numSequences()
 */
Datum
Temporal_num_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_sequences(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Temporal_start_sequence(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_start_sequence);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start sequence of a temporal sequence (set)
 * @sqlfn startSequence()
 */
Datum
Temporal_start_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_start_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Temporal_end_sequence(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_end_sequence);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end sequence of a temporal sequence (set)
 * @sqlfn endSequence()
 */
Datum
Temporal_end_sequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *result = temporal_end_sequence(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Temporal_sequence_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_sequence_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th sequence of a temporal sequence (set)
 * @sqlfn sequenceN()
 */
Datum
Temporal_sequence_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TSequence *result = temporal_sequence_n(temp, n);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Temporal_sequences(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_sequences);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of sequences of a temporal sequence (set)
 * @sqlfn sequences()
 */
Datum
Temporal_sequences(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  const TSequence **sequences = temporal_seqs(temp, &count);
  ArrayType *result = temparr_to_array((Temporal **) sequences, count, FREE);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Temporal_segments(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_segments);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of segments of a temporal sequence (set)
 * @sqlfn segments()
 */
Datum
Temporal_segments(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TSequence **segments = temporal_segments(temp, &count);
  ArrayType *result = temparr_to_array((Temporal **) segments, count,
    FREE_ALL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Temporal_num_instants(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_num_instants);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of distinct instants of a temporal value
 * @sqlfn numInstants()
 */
Datum
Temporal_num_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_instants(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Temporal_lower_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_lower_inc);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return true if the start instant of a temporal value is inclusive
 * @sqlfn startInstant()
 */
Datum
Temporal_lower_inc(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool result = (temporal_lower_inc(temp) == 1) ? true : false;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_upper_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_upper_inc);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return true if the end instant of a temporal value is inclusive
 * @sqlfn startInstant()
 */
Datum
Temporal_upper_inc(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool result = (temporal_upper_inc(temp) == 1) ? true : false;
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_start_instant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_start_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start instant of a temporal value
 * @sqlfn startInstant()
 */
Datum
Temporal_start_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = temporal_start_instant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Temporal_end_instant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_end_instant);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end instant of a temporal value
 * @sqlfn endInstant()
 */
Datum
Temporal_end_instant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = temporal_end_instant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Temporal_instant_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_instant_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th instant of a temporal value
 * @sqlfn instantN()
 */
Datum
Temporal_instant_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TInstant *result = temporal_instant_n(temp, n);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Temporal_instants(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_instants);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of distinct instants of a temporal value
 * @sqlfn instants()
 */
Datum
Temporal_instants(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  const TInstant **instants = temporal_insts(temp, &count);
  ArrayType *result = temparr_to_array((Temporal **) instants, count, FREE);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Temporal_num_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_num_timestamps);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the number of distinct timestamps of a temporal value
 * @sqlfn numTimestamps()
 */
Datum
Temporal_num_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = temporal_num_timestamps(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Temporal_start_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_start_timestamptz);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the start timestamptz of a temporal value
 * @sqlfn startTimestamp()
 */
Datum
Temporal_start_timestamptz(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_start_timestamptz(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Temporal_end_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_end_timestamptz);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the end timestamptz of a temporal value
 * @sqlfn endTimestamp()
 */
Datum
Temporal_end_timestamptz(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz result = temporal_end_timestamptz(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Temporal_timestamptz_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_timestamptz_n);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the n-th distinct timestamptz of a temporal value
 * @sqlfn timestampN()
 */
Datum
Temporal_timestamptz_n(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = temporal_timestamptz_n(temp, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Temporal_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_timestamps);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of distinct timestamps of a temporal value
 * @sqlfn timestamps()
 */
Datum
Temporal_timestamps(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  TimestampTz *times = temporal_timestamps(temp, &count);
  ArrayType *result = tstzarr_to_array(times, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_stops(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_stops);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the constant segments of a temporal value
 * @sqlfn stops()
 */
Datum
Temporal_stops(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double maxdist = PG_GETARG_FLOAT8(1);
  Interval *minduration = PG_GETARG_INTERVAL_P(2);
  /* Store fcinfo into a global variable */
  /* Needed for the distance function for temporal geography points */
  store_fcinfo(fcinfo);
  TSequenceSet *result = temporal_stops(temp, maxdist, minduration);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TSEQUENCESET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_value_at_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_value_at_timestamptz);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the base value of a temporal value at a timestamptz
 * @sqlfn valueAtTimestamp()
 */
Datum
Temporal_value_at_timestamptz(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Datum result;
  bool found = temporal_value_at_timestamptz(temp, t, true, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tfloat_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal float with the values set to a number of decimal
 * places
 * @sqlfn round()
 */
Datum
Tfloat_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int size = PG_GETARG_INT32(1);
  Temporal *result = tfloat_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloatarr_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloatarr_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return an array of temporal floats with the precision of the values
 * set to a number of decimal places
 * @sqlfn asText()
 */
Datum
Tfloatarr_round(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int maxdd = PG_GETARG_INT32(1);

  Temporal **temparr = temparr_extract(array, &count);
  Temporal **resarr = tfloatarr_round((const Temporal **) temparr, count,
    maxdd);
  ArrayType *result = temparr_to_array(resarr, count, FREE_ALL);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Temporal_to_tinstant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_to_tinstant);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value transformed to a temporal instant
 * @sqlfn  tint_inst(), tfloat_inst(), ...
 */
Datum
Temporal_to_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = temporal_to_tinstant(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum Temporal_to_tsequence(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_to_tsequence);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value transformed to a temporal sequence
 * @note The SQL function is not strict
 * @sqlfn tint_seq(), tfloat_seq(), ...
 */
Datum
Temporal_to_tsequence(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *interp_str = NULL;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    interp_str = text2cstring(interp_txt);
  }
  TSequence *result = temporal_to_tsequence(temp, interp_str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCE_P(result);
}

PGDLLEXPORT Datum Temporal_to_tsequenceset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_to_tsequenceset);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value transformed to a temporal sequence set
 * @note The SQL function is not strict
 * @sqlfn tint_seqset(), tfloat_seqset(), ...
 */
Datum
Temporal_to_tsequenceset(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *interp_str = NULL;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
  {
    text *interp_txt = PG_GETARG_TEXT_P(1);
    interp_str = text2cstring(interp_txt);
  }
  TSequenceSet *result = temporal_to_tsequenceset(temp, interp_str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TSEQUENCESET_P(result);
}

PGDLLEXPORT Datum Temporal_set_interp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_set_interp);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value transformed to an interpolation
 * @sqlfn setInterp()
 */
Datum
Temporal_set_interp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *interp_txt = PG_GETARG_TEXT_P(1);
  char *interp_str = text2cstring(interp_txt);
  interpType interp = interptype_from_string(interp_str);
  pfree(interp_str);
  Temporal *result = temporal_set_interp(temp, interp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tnumber_shift_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_shift_value);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value shifted by an interval
 * @sqlfn shiftValue()
 */
Datum
Tnumber_shift_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Temporal *result = tnumber_shift_scale_value(temp, shift, 0, true, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_scale_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_scale_value);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value scaled by an interval
 * @sqlfn scaleValue()
 */
Datum
Tnumber_scale_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum duration = PG_GETARG_DATUM(1);
  Temporal *result = tnumber_shift_scale_value(temp, 0, duration, false, true);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_shift_scale_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_shift_scale_value);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value shifted and scaled by two intervals
 * @sqlfn shiftScaleValue()
 */
Datum
Tnumber_shift_scale_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum duration = PG_GETARG_DATUM(2);
  Temporal *result = tnumber_shift_scale_value(temp, shift, duration, true, true);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_shift_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_shift_time);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value shifted by an interval
 * @sqlfn shiftTime()
 */
Datum
Temporal_shift_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_scale_time(temp, shift, NULL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_scale_time);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value scaled by an interval
 * @sqlfn scaleTime()
 */
Datum
Temporal_scale_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_shift_scale_time(temp, NULL, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_shift_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_shift_scale_time);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal value shifted and scaled by two intervals
 * @sqlfn shiftScaleTime()
 */
Datum
Temporal_shift_scale_time(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Temporal *result = temporal_shift_scale_time(temp, shift, duration);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] temp Temporal value
 */
TempUnnestState *
temporal_unnest_state_make(const Temporal *temp)
{
  TempUnnestState *state = palloc0(sizeof(TempUnnestState));
  int count;
  Datum *values = temporal_values(temp, &count);
  /* Fill in state */
  state->done = false;
  state->i = 0;
  state->count = count;
  state->values = values;
  state->temp = temporal_cp(temp);
  return state;
}

/**
 * @brief Increment the current state to the next unnest value
 * @param[in] state State to increment
 */
void
temporal_unnest_state_next(TempUnnestState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

PGDLLEXPORT Datum Temporal_unnest(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_unnest);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return the list of values and associated span sets of a temporal value
 */
Datum
Temporal_unnest(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  TempUnnestState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    ensure_nonlinear_interp(temp->flags);
    /* Create function state */
    funcctx->user_fctx = temporal_unnest_state_make(temp);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all buckets */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    // pfree(state->values);
    // pfree(state->temp);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Get value */
  tuple_arr[0] = state->values[state->i];
  /* Get span set */
  Temporal *rest = temporal_restrict_value(state->temp,
    state->values[state->i], REST_AT);
  if (!rest)
    elog(ERROR, "Unexpected error with temporal value %s",
      temporal_out(state->temp, OUT_DEFAULT_DECIMAL_DIGITS));
  tuple_arr[1] = PointerGetDatum(temporal_time(rest));
  pfree(rest);
  /* Advance state */
  temporal_unnest_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************
 * Append and merge functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_append_tinstant(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_append_tinstant);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Append an instant to a temporal value
 * @sqlfn appendInstant()
 */
Datum
Temporal_append_tinstant(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *inst = PG_GETARG_TINSTANT_P(1);
  Temporal *result = temporal_append_tinstant(temp, inst, 0.0, NULL, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(inst, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_append_tsequence(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_append_tsequence);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Append a sequence to a temporal value
 * @sqlfn appendSequence()
 */
Datum
Temporal_append_tsequence(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TSequence *seq = PG_GETARG_TSEQUENCE_P(1);
  Temporal *result = temporal_append_tsequence(temp, seq, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(seq, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_merge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_merge);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Merge two temporal values
 * @sqlfn merge()
 */
Datum
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
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_merge_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_merge_array);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Merge an array of temporal values
 * @sqlfn merge()
 */
Datum
Temporal_merge_array(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  int count;
  Temporal **temparr = temparr_extract(array, &count);
  Temporal *result = temporal_merge_array(temparr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) an array of
 * base values
 */
static Datum
Temporal_restrict_value(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_ANYDATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Temporal *result = temporal_restrict_value(temp, value, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  DATUM_FREE_IF_COPY(value, basetype, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_value);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a base value
 * @sqlfn atValue()
 */
Datum
Temporal_at_value(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_value(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_value);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a base value
 * @sqlfn minusValue()
 */
Datum
Temporal_minus_value(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_value(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) an array of
 * base values
 */
static Datum
Temporal_restrict_values(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *set = PG_GETARG_SET_P(1);
  /* For temporal points the validity of values in the array is done in
   * bounding box function */
  Temporal *result = (set->count > 1) ?
    temporal_restrict_values(temp, set, atfunc) :
    temporal_restrict_value(temp, SET_VAL_N(set, 0), atfunc);

  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(set, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_values);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to an array of base values
 * @sqlfn atValues()
 */
Datum
Temporal_at_values(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_values(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_values);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of an array of
 * base values
 * @sqlfn minusValues()
 */
Datum
Temporal_minus_values(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_values(fcinfo, REST_MINUS);
}

/*****************************************************************************/

static Datum
Tnumber_restrict_span(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *span = PG_GETARG_SPAN_P(1);
  Temporal *result = tnumber_restrict_span(temp, span, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_at_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_at_span);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a span of base values
 * @sqlfn atSpan()
 */
Datum
Tnumber_at_span(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_span(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnumber_minus_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_minus_span);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a span of
 * base values
 * @sqlfn minusSpan()
 */
Datum
Tnumber_minus_span(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_span(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) a span set
 * of base values
 */
static Datum
Tnumber_restrict_spanset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  Temporal *result = tnumber_restrict_spanset(temp, ss, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_at_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_at_spanset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a span set of base values
 * @sqlfn atSpanset()
 */
Datum
Tnumber_at_spanset(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_spanset(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnumber_minus_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_minus_spanset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a span set of
 * base values
 * @sqlfn minusSpanset()
 */
Datum
Tnumber_minus_spanset(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_spanset(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Generic function for restricting a temporal value to its 
 * minimum/maximum base value
 * @sqlfn atMin(), minusMin(), atMax(), minusMax(),
 */
Datum
Temporal_restrict_minmax(FunctionCallInfo fcinfo, bool min, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_restrict_minmax(temp, min, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_min(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_min);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to its minimum base value
 * @sqlfn atMin()
 */
Datum
Temporal_at_min(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_minmax(fcinfo, GET_MIN, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_min(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_min);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of its minimum
 * base value
 * @sqlfn minusMin()
 */
Datum
Temporal_minus_min(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_minmax(fcinfo, GET_MIN, REST_MINUS);
}

PGDLLEXPORT Datum Temporal_at_max(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_max);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to its maximum base value
 * @sqlfn atMax()
 */
Datum
Temporal_at_max(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_minmax(fcinfo, GET_MAX, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_max(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_max);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of its maximum
 * base value
 * @sqlfn minusMax()
 */
Datum
Temporal_minus_max(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_minmax(fcinfo, GET_MAX, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) a temporal
 * box
  */
static Datum
Tnumber_restrict_tbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBox *box = PG_GETARG_TBOX_P(1);
  Temporal *result = atfunc ? tnumber_at_tbox(temp, box) :
    tnumber_minus_tbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_at_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_at_tbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a temporal box
 * @sqlfn atTbox()
 */
Datum
Tnumber_at_tbox(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_tbox(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnumber_minus_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_minus_tbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a temporal
 * box
 * @sqlfn minusTbox()
 */
Datum
Tnumber_minus_tbox(PG_FUNCTION_ARGS)
{
  return Tnumber_restrict_tbox(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) a
 * timestamptz
 */
static Datum
Temporal_restrict_timestamptz(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = temporal_restrict_timestamptz(temp, t, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_timestamptz);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz
 * @sqlfn atTime()
 */
Datum
Temporal_at_timestamptz(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_timestamptz(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_timestamptz);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * @sqlfn minusTime()
 */
Datum
Temporal_minus_timestamptz(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_timestamptz(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to a timestamptz set
 * @sqlfn atTime()
 */
Datum
Temporal_restrict_tstzset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  Temporal *result = temporal_restrict_tstzset(temp, s, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_tstzset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_tstzset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz set
 * @sqlfn atTime()
 */
Datum
Temporal_at_tstzset(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzset(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_tstzset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_tstzset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * set
 * @sqlfn minusTime()
 */
Datum
Temporal_minus_tstzset(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzset(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) a
 * timestamptz span
 */
static Datum
Temporal_restrict_tstzspan(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  Temporal *result = temporal_restrict_tstzspan(temp, s, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_tstzspan);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz span
 * @sqlfn atTime()
 */
Datum
Temporal_at_tstzspan(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzspan(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_tstzspan);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a timestamptz
 * span
 * @sqlfn minusTime()
 */
Datum
Temporal_minus_tstzspan(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzspan(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to a timestamptz span set
 * @sqlfn atTime()
 */
Datum
Temporal_restrict_tstzspanset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_at_tstzspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_at_tstzspanset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to a timestamptz span set
 * @sqlfn atTime()
 */
Datum
Temporal_at_tstzspanset(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzspanset(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Temporal_minus_tstzspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_minus_tstzspanset);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal value restricted to the complement of a span set
 * @sqlfn minusTime()
 */
Datum
Temporal_minus_tstzspanset(PG_FUNCTION_ARGS)
{
  return Temporal_restrict_tstzspanset(fcinfo, REST_MINUS);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_insert(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_insert);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Insert the second temporal value into the first one
 * @sqlfn insert()
 */
Datum
Temporal_insert(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_insert(temp1, temp2, connect);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_update(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_update);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Update the first temporal value with the second one
 * @sqlfn update()
 */
Datum
Temporal_update(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_update(temp1, temp2, connect);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_delete_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_delete_timestamptz);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Delete a timestamptz from a temporal value
 * @sqlfn deleteTime()
 */
Datum
Temporal_delete_timestamptz(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_delete_timestamptz(temp, t, connect);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_delete_tstzset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_delete_tstzset);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Delete a timestamptz set from a temporal value
 * @sqlfn deleteTime()
 */
Datum
Temporal_delete_tstzset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_delete_tstzset(temp, s, connect);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_delete_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_delete_tstzspan);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Delete a timestamptz span from a temporal value
 * @sqlfn deleteTime()
 */
Datum
Temporal_delete_tstzspan(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_delete_tstzspan(temp, s, connect);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_delete_tstzspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_delete_tstzspanset);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Delete a timestamptz span set from a temporal value
 * @sqlfn deleteTime()
 */
Datum
Temporal_delete_tstzspanset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool connect = PG_GETARG_BOOL(2);
  Temporal *result = temporal_delete_tstzspanset(temp, ss, connect);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_integral(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_integral);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the integral (area under the curve) of a temporal number
 * @sqlfn integral()
 */
Datum
Tnumber_integral(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_integral(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tnumber_twavg(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_twavg);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighted average of a temporal number
 * @sqlfn twAvg()
 */
Datum
Tnumber_twavg(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnumber_twavg(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_eq);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if two temporal values are equal
 * @sqlfn tint_eq(), tfloat_eq(), ...
 * @sqlop =
 */
Datum
Temporal_eq(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_eq(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_ne);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if two temporal values are different
 * @sqlfn tint_ne(), tfloat_ne(), ...
 * @sqlop <>
 */
Datum
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

PGDLLEXPORT Datum Temporal_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_cmp);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return -1, 0, or 1 depending on whether the first temporal value
 * is less than, equal to, or greater than the second temporal value
 * @sqlfn tint_cmp(), tfloat_cmp(), ...
 */
Datum
Temporal_cmp(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = temporal_cmp(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Temporal_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_lt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first temporal value is less than the second one
 * @sqlfn tint_lt(), tfloat_lt(), ...
 * @sqlop @p <
 */
Datum
Temporal_lt(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_lt(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_le);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first temporal value is less than or equal to
 * the second one
 * @sqlfn tint_le(), tfloat_le(), ...
 * @sqlop @p <=
 */
Datum
Temporal_le(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_le(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_ge);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than or equal to
 * the second one
 * @sqlfn tint_ge(), tfloat_ge(), ...
 * @sqlop =@p >=
 */
Datum
Temporal_ge(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = temporal_ge(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Temporal_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_gt);
/**
 * @ingroup mobilitydb_temporal_comp_trad
 * @brief Return true if the first temporal value is greater than the second one
 * @sqlfn tint_gt(), tfloat_gt(), ...
 * @sqlop @p >
 */
Datum
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

PGDLLEXPORT Datum Temporal_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_hash);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the hash value of a temporal value
 * @sqlfn tint_hash(), tfloat_hash(), ...
 */
Datum
Temporal_hash(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint32 result = temporal_hash(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_UINT32(result);
}

/*****************************************************************************/
