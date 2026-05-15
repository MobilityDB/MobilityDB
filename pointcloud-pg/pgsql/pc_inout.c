/***********************************************************************
 * pc_inout.c
 *
 *  Input/output functions for points and patches in PgSQL.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "limits.h"

#include "pc_pgsql.h" /* Common PgSQL support for our type */

/* In/out functions */
Datum pcpoint_in(PG_FUNCTION_ARGS);
Datum pcpoint_out(PG_FUNCTION_ARGS);
Datum pcpatch_in(PG_FUNCTION_ARGS);
Datum pcpatch_out(PG_FUNCTION_ARGS);

/* Typmod support */
Datum pc_typmod_in(PG_FUNCTION_ARGS);
Datum pc_typmod_out(PG_FUNCTION_ARGS);
Datum pc_typmod_pcid(PG_FUNCTION_ARGS);
Datum pcpatch_enforce_typmod(PG_FUNCTION_ARGS);
Datum pcpoint_enforce_typmod(PG_FUNCTION_ARGS);

/* Other SQL functions */
Datum pcschema_is_valid(PG_FUNCTION_ARGS);
Datum pcschema_get_ndims(PG_FUNCTION_ARGS);
Datum pcpoint_from_double_array(PG_FUNCTION_ARGS);
Datum pcpoint_as_text(PG_FUNCTION_ARGS);
Datum pcpatch_as_text(PG_FUNCTION_ARGS);
Datum pcpoint_as_bytea(PG_FUNCTION_ARGS);
Datum pcpatch_envelope_as_bytea(PG_FUNCTION_ARGS);
Datum pcpatch_bounding_diagonal_as_bytea(PG_FUNCTION_ARGS);

static void pcid_consistent(const uint32 pcid, const uint32 column_pcid)
{
  if (column_pcid && pcid != column_pcid)
  {
    ereport(ERROR,
            (errcode(ERRCODE_DATATYPE_MISMATCH),
             errmsg("point/patch pcid (%u) does not match column pcid (%d)",
                    pcid, column_pcid)));
  }
}

PG_FUNCTION_INFO_V1(pcpoint_in);
Datum pcpoint_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  /* Datum pc_oid = PG_GETARG_OID(1); Not needed. */
  int32 typmod = 0;
  uint32 pcid = 0;
  PCPOINT *pt;
  SERIALIZED_POINT *serpt = NULL;

  if ((PG_NARGS() > 2) && (!PG_ARGISNULL(2)))
  {
    typmod = PG_GETARG_INT32(2);
    pcid = pcid_from_typmod(typmod);
  }

  /* Empty string. */
  if (str[0] == '\0')
  {
    ereport(ERROR, (errmsg("pcpoint parse error - empty string")));
  }

  /* Binary or text form? Let's find out. */
  if (str[0] == '0')
  {
    /* Hex-encoded binary */
    pt = pc_point_from_hexwkb(str, strlen(str), fcinfo);
    pcid_consistent(pt->schema->pcid, pcid);
    serpt = pc_point_serialize(pt);
    pc_point_free(pt);
  }
  else
  {
    ereport(
        ERROR,
        (errmsg("parse error - support for text format not yet implemented")));
  }

  if (serpt)
    PG_RETURN_POINTER(serpt);
  else
    PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(pcpoint_out);
Datum pcpoint_out(PG_FUNCTION_ARGS)
{
  PCPOINT *pcpt = NULL;
  PCSCHEMA *schema = NULL;
  SERIALIZED_POINT *serpt = NULL;
  char *hexwkb = NULL;

  serpt = PG_GETARG_SERPOINT_P(0);
  schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
  pcpt = pc_point_deserialize(serpt, schema);
  hexwkb = pc_point_to_hexwkb(pcpt);
  pc_point_free(pcpt);
  PG_RETURN_CSTRING(hexwkb);
}

PG_FUNCTION_INFO_V1(pcpatch_in);
Datum pcpatch_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  /* Datum geog_oid = PG_GETARG_OID(1); Not needed. */
  uint32 typmod = 0, pcid = 0;
  PCPATCH *patch;
  SERIALIZED_PATCH *serpatch = NULL;

  if ((PG_NARGS() > 2) && (!PG_ARGISNULL(2)))
  {
    typmod = PG_GETARG_INT32(2);
    pcid = pcid_from_typmod(typmod);
  }

  /* Empty string. */
  if (str[0] == '\0')
  {
    ereport(ERROR, (errmsg("pcpatch parse error - empty string")));
  }

  /* Binary or text form? Let's find out. */
  if (str[0] == '0')
  {
    /* Hex-encoded binary */
    patch = pc_patch_from_hexwkb(str, strlen(str), fcinfo);
    pcid_consistent(patch->schema->pcid, pcid);
    serpatch = pc_patch_serialize(patch, NULL);
    pc_patch_free(patch);
  }
  else
  {
    ereport(
        ERROR,
        (errmsg("parse error - support for text format not yet implemented")));
  }

  if (serpatch)
    PG_RETURN_POINTER(serpatch);
  else
    PG_RETURN_NULL();
}

PG_FUNCTION_INFO_V1(pcpatch_out);
Datum pcpatch_out(PG_FUNCTION_ARGS)
{
  PCPATCH *patch = NULL;
  SERIALIZED_PATCH *serpatch = NULL;
  char *hexwkb = NULL;
  PCSCHEMA *schema = NULL;

  serpatch = PG_GETARG_SERPATCH_P(0);
  schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);
  patch = pc_patch_deserialize(serpatch, schema);
  hexwkb = pc_patch_to_hexwkb(patch);
  pc_patch_free(patch);
  PG_RETURN_CSTRING(hexwkb);
}

PG_FUNCTION_INFO_V1(pcschema_is_valid);
Datum pcschema_is_valid(PG_FUNCTION_ARGS)
{
  bool valid;
  text *xml = PG_GETARG_TEXT_P(0);
  char *xmlstr = text_to_cstring(xml);
  PCSCHEMA *schema = pc_schema_from_xml(xmlstr);
  pfree(xmlstr);

  if (!schema)
  {
    PG_RETURN_BOOL(false);
  }

  valid = pc_schema_is_valid(schema);
  pc_schema_free(schema);
  PG_RETURN_BOOL(valid);
}

PG_FUNCTION_INFO_V1(pcschema_get_ndims);
Datum pcschema_get_ndims(PG_FUNCTION_ARGS)
{
  int ndims;
  uint32 pcid = PG_GETARG_INT32(0);
  PCSCHEMA *schema = pc_schema_from_pcid(pcid, fcinfo);

  if (!schema)
    elog(ERROR, "unable to load schema for pcid = %d", pcid);

  ndims = schema->ndims;
  PG_RETURN_INT32(ndims);
}

/**
 * pcpoint_from_double_array(integer pcid, float8[] returns PcPoint
 */
PG_FUNCTION_INFO_V1(pcpoint_from_double_array);
Datum pcpoint_from_double_array(PG_FUNCTION_ARGS)
{
  uint32 pcid = PG_GETARG_INT32(0);
  ArrayType *arrptr = PG_GETARG_ARRAYTYPE_P(1);
  int nelems;
  float8 *vals;
  PCPOINT *pt;
  PCSCHEMA *schema = pc_schema_from_pcid(pcid, fcinfo);
  SERIALIZED_POINT *serpt;

  if (!schema)
    elog(ERROR, "unable to load schema for pcid = %d", pcid);

  if (ARR_ELEMTYPE(arrptr) != FLOAT8OID)
    elog(ERROR, "array must be of float8[]");

  if (ARR_NDIM(arrptr) != 1)
    elog(ERROR, "float8[] must have only one dimension");

  if (ARR_HASNULL(arrptr))
    elog(ERROR, "float8[] must not have null elements");

  nelems = ARR_DIMS(arrptr)[0];
  if (nelems != schema->ndims || ARR_LBOUND(arrptr)[0] > 1)
    elog(ERROR, "array dimensions do not match schema dimensions of pcid = %d",
         pcid);

  vals = (float8 *)ARR_DATA_PTR(arrptr);
  pt = pc_point_from_double_array(schema, vals, 0, nelems);

  serpt = pc_point_serialize(pt);
  pc_point_free(pt);
  PG_RETURN_POINTER(serpt);
}

PG_FUNCTION_INFO_V1(pcpoint_as_text);
Datum pcpoint_as_text(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt = PG_GETARG_SERPOINT_P(0);
  text *txt;
  char *str;
  PCSCHEMA *schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
  PCPOINT *pt = pc_point_deserialize(serpt, schema);
  if (!pt)
    PG_RETURN_NULL();

  str = pc_point_to_string(pt);
  pc_point_free(pt);
  txt = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(txt);
}

PG_FUNCTION_INFO_V1(pcpatch_as_text);
Datum pcpatch_as_text(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpatch = PG_GETARG_SERPATCH_P(0);
  text *txt;
  char *str;
  PCSCHEMA *schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);
  PCPATCH *patch = pc_patch_deserialize(serpatch, schema);
  if (!patch)
    PG_RETURN_NULL();

  str = pc_patch_to_string(patch);
  pc_patch_free(patch);
  txt = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(txt);
}

PG_FUNCTION_INFO_V1(pcpoint_as_bytea);
Datum pcpoint_as_bytea(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt = PG_GETARG_SERPOINT_P(0);
  uint8 *bytes;
  size_t bytes_size;
  bytea *wkb;
  size_t wkb_size;
  PCSCHEMA *schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
  PCPOINT *pt = pc_point_deserialize(serpt, schema);

  if (!pt)
    PG_RETURN_NULL();

  bytes = pc_point_to_geometry_wkb(pt, &bytes_size);
  wkb_size = VARHDRSZ + bytes_size;
  wkb = palloc(wkb_size);
  memcpy(VARDATA(wkb), bytes, bytes_size);
  SET_VARSIZE(wkb, wkb_size);

  pc_point_free(pt);
  pfree(bytes);

  PG_RETURN_BYTEA_P(wkb);
}

PG_FUNCTION_INFO_V1(pcpatch_envelope_as_bytea);
Datum pcpatch_envelope_as_bytea(PG_FUNCTION_ARGS)
{
  uint8 *bytes;
  size_t bytes_size;
  bytea *wkb;
  size_t wkb_size;
  SERIALIZED_PATCH *serpatch = PG_GETHEADER_SERPATCH_P(0);
  PCSCHEMA *schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);

  bytes = pc_patch_to_geometry_wkb_envelope(serpatch, schema, &bytes_size);
  wkb_size = VARHDRSZ + bytes_size;
  wkb = palloc(wkb_size);
  memcpy(VARDATA(wkb), bytes, bytes_size);
  SET_VARSIZE(wkb, wkb_size);

  pfree(bytes);

  PG_RETURN_BYTEA_P(wkb);
}

PG_FUNCTION_INFO_V1(pcpatch_bounding_diagonal_as_bytea);
Datum pcpatch_bounding_diagonal_as_bytea(PG_FUNCTION_ARGS)
{
  static const size_t stats_size_guess = 400;
  SERIALIZED_PATCH *serpatch;
  PCSCHEMA *schema;
  PCSTATS *stats;
  uint8 *bytes;
  size_t bytes_size;
  bytea *wkb;
  size_t wkb_size;

  serpatch = PG_GETHEADERX_SERPATCH_P(0, stats_size_guess);
  schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);

  if (schema->zdim || schema->mdim)
  {
    if (stats_size_guess < pc_stats_size(schema))
      serpatch = PG_GETHEADERX_SERPATCH_P(0, pc_stats_size(schema));

    stats = pc_patch_stats_deserialize(schema, serpatch->data);
    if (!stats)
      PG_RETURN_NULL();

    bytes = pc_bounding_diagonal_wkb_from_stats(stats, &bytes_size);

    pc_stats_free(stats);
  }
  else
  {
    bytes = pc_bounding_diagonal_wkb_from_bounds(&serpatch->bounds, schema,
                                                 &bytes_size);
  }

  wkb_size = VARHDRSZ + bytes_size;
  wkb = palloc(wkb_size);
  memcpy(VARDATA(wkb), bytes, bytes_size);
  SET_VARSIZE(wkb, wkb_size);

  pcfree(bytes);

  PG_RETURN_BYTEA_P(wkb);
}

PG_FUNCTION_INFO_V1(pc_typmod_in);
Datum pc_typmod_in(PG_FUNCTION_ARGS)
{
  uint32 typmod = 0;
  Datum *elem_values;
  int n = 0;
  int i = 0;
  ArrayType *arr = (ArrayType *)DatumGetPointer(PG_GETARG_DATUM(0));

  if (ARR_ELEMTYPE(arr) != CSTRINGOID)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
                    errmsg("typmod array must be type cstring[]")));

  if (ARR_NDIM(arr) != 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
                    errmsg("typmod array must be one-dimensional")));

  if (ARR_HASNULL(arr))
    ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
                    errmsg("typmod array must not contain nulls")));

  if (ArrayGetNItems(ARR_NDIM(arr), ARR_DIMS(arr)) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
                    errmsg("typmod array must have one element")));

  deconstruct_array(arr, CSTRINGOID, -2, false,
                    'c', /* hardwire cstring representation details */
                    &elem_values, NULL, &n);

  for (i = 0; i < n; i++)
  {
    if (i == 0) /* PCID */
    {
      char *s = DatumGetCString(elem_values[i]);
      char *endp;

      errno = 0;
      typmod = (uint32)strtol(s, &endp, 10);

      if (s == endp)
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type uint32: \"%s\"", s)));

      if (errno == ERANGE || typmod < 0 || typmod > UINT_MAX)
        ereport(ERROR,
                (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
                 errmsg("value \"%s\" is out of range for type uint32", s)));

      if (*endp != '\0')
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("invalid input syntax for type uint32: \"%s\"", s)));
    }
  }

  PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(pc_typmod_out);
Datum pc_typmod_out(PG_FUNCTION_ARGS)
{
  char *str = (char *)palloc(64);
  uint32 typmod = PG_GETARG_INT32(0);
  uint32 pcid = pcid_from_typmod(typmod);

  /* No PCID value? Then no typmod at all. Return empty string. */
  if (!pcid)
  {
    str[0] = '\0';
    PG_RETURN_CSTRING(str);
  }
  else
  {
    sprintf(str, "(%u)", pcid);
    PG_RETURN_CSTRING(str);
  }
}

PG_FUNCTION_INFO_V1(pc_typmod_pcid);
Datum pc_typmod_pcid(PG_FUNCTION_ARGS)
{
  uint32 typmod = PG_GETARG_INT32(0);
  uint32 pcid = pcid_from_typmod(typmod);
  if (!pcid)
    PG_RETURN_NULL();
  else
    PG_RETURN_INT32(pcid);
}

PG_FUNCTION_INFO_V1(pcpatch_enforce_typmod);
Datum pcpatch_enforce_typmod(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *arg = PG_GETARG_SERPATCH_P(0);
  uint32 typmod = PG_GETARG_INT32(1);
  uint32 pcid = pcid_from_typmod(typmod);
  /* We don't need to have different behavior based on explicitness. */
  /* bool isExplicit = PG_GETARG_BOOL(2); */

  /* Check if column typmod is consistent with the object */
  if (pcid != arg->pcid)
    elog(ERROR, "column pcid (%d) and patch pcid (%d) are not consistent", pcid,
         arg->pcid);

  PG_RETURN_POINTER(arg);
}

PG_FUNCTION_INFO_V1(pcpoint_enforce_typmod);
Datum pcpoint_enforce_typmod(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *arg = PG_GETARG_SERPOINT_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  uint32 pcid = pcid_from_typmod(typmod);
  /* We don't need to have different behavior based on explicitness. */
  /* bool isExplicit = PG_GETARG_BOOL(2); */

  /* Check if column typmod is consistent with the object */
  if (pcid != arg->pcid)
    elog(ERROR, "column pcid (%d) and point pcid (%d) are not consistent", pcid,
         arg->pcid);

  PG_RETURN_POINTER(arg);
}
