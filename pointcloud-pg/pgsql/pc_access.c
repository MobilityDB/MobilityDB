/***********************************************************************
 * pc_access.c
 *
 *  Accessor/aggregate functions for points and patches in PgSQL.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_pgsql.h" /* Common PgSQL support for our type */
#include "utils/numeric.h"

#include "funcapi.h"
#include "lib/stringinfo.h"
#include "pc_api_internal.h" /* for pcpatch_summary */

/* cstring array utility functions */
const char **array_to_cstring_array(ArrayType *array, int *size);
void pc_cstring_array_free(const char **array, int nelems);

/* General SQL functions */
Datum pcpoint_get_value(PG_FUNCTION_ARGS);
Datum pcpoint_get_values(PG_FUNCTION_ARGS);
Datum pcpatch_from_pcpoint_array(PG_FUNCTION_ARGS);
Datum pcpatch_from_float_array(PG_FUNCTION_ARGS);
Datum pcpatch_from_pcpatch_array(PG_FUNCTION_ARGS);
Datum pcpatch_uncompress(PG_FUNCTION_ARGS);
Datum pcpatch_compress(PG_FUNCTION_ARGS);
Datum pcpatch_numpoints(PG_FUNCTION_ARGS);
Datum pcpatch_pointn(PG_FUNCTION_ARGS);
Datum pcpatch_range(PG_FUNCTION_ARGS);
Datum pcpatch_pcid(PG_FUNCTION_ARGS);
Datum pcpatch_summary(PG_FUNCTION_ARGS);
Datum pcpatch_compression(PG_FUNCTION_ARGS);
Datum pcpatch_intersects(PG_FUNCTION_ARGS);
Datum pcpatch_get_stat(PG_FUNCTION_ARGS);
Datum pcpatch_filter(PG_FUNCTION_ARGS);
Datum pcpatch_sort(PG_FUNCTION_ARGS);
Datum pcpatch_is_sorted(PG_FUNCTION_ARGS);
Datum pcpatch_size(PG_FUNCTION_ARGS);
Datum pcpoint_size(PG_FUNCTION_ARGS);
Datum pcpoint_pcid(PG_FUNCTION_ARGS);
Datum pc_version(PG_FUNCTION_ARGS);
Datum pc_pgsql_version(PG_FUNCTION_ARGS);
Datum pc_libxml2_version(PG_FUNCTION_ARGS);
Datum pc_lazperf_enabled(PG_FUNCTION_ARGS);

/* Generic aggregation functions */
Datum pointcloud_agg_transfn(PG_FUNCTION_ARGS);
Datum pointcloud_abs_in(PG_FUNCTION_ARGS);
Datum pointcloud_abs_out(PG_FUNCTION_ARGS);

/* Point finalizers */
Datum pcpoint_agg_final_pcpatch(PG_FUNCTION_ARGS);
Datum pcpoint_agg_final_array(PG_FUNCTION_ARGS);

/* Patch finalizers */
Datum pcpatch_agg_final_array(PG_FUNCTION_ARGS);
Datum pcpatch_agg_final_pcpatch(PG_FUNCTION_ARGS);

/* Deaggregation functions */
Datum pcpatch_unnest(PG_FUNCTION_ARGS);

/**
 * Read a named dimension from a PCPOINT
 * PC_Get(point pcpoint, dimname text) returns Numeric
 */
PG_FUNCTION_INFO_V1(pcpoint_get_value);
Datum pcpoint_get_value(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt = PG_GETARG_SERPOINT_P(0);
  text *dim_name = PG_GETARG_TEXT_P(1);
  char *dim_str;
  float8 double_result;

  PCSCHEMA *schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
  PCPOINT *pt = pc_point_deserialize(serpt, schema);
  if (!pt)
    PG_RETURN_NULL();

  dim_str = text_to_cstring(dim_name);
  if (!pc_point_get_double_by_name(pt, dim_str, &double_result))
  {
    pc_point_free(pt);
    elog(ERROR, "dimension \"%s\" does not exist in schema", dim_str);
  }
  pfree(dim_str);
  pc_point_free(pt);
  PG_RETURN_DATUM(
      DirectFunctionCall1(float8_numeric, Float8GetDatum(double_result)));
}

/**
 * Returns all the values of a point as a double precision array
 * PC_Get(point pcpoint) returns Float8[]
 */
PG_FUNCTION_INFO_V1(pcpoint_get_values);
Datum pcpoint_get_values(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt;
  ArrayType *result;
  PCSCHEMA *schema;
  PCPOINT *pt;
  Datum *elems;
  int i;
  double *vals;

  serpt = PG_GETARG_SERPOINT_P(0);
  schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
  pt = pc_point_deserialize(serpt, schema);
  if (!pt)
    PG_RETURN_NULL();

  elems = (Datum *)palloc(schema->ndims * sizeof(Datum));
  vals = pc_point_to_double_array(pt);
  i = schema->ndims;
  while (i--)
    elems[i] = Float8GetDatum(vals[i]);
  pcfree(vals);
  result = construct_array(elems, schema->ndims, FLOAT8OID, sizeof(float8),
                           FLOAT8PASSBYVAL, 'd');

  pc_point_free(pt);
  PG_RETURN_ARRAYTYPE_P(result);
}

static inline bool array_get_isnull(const bits8 *nullbitmap, int offset)
{
  if (nullbitmap == NULL)
  {
    return false; /* assume not null */
  }
  if (nullbitmap[offset / 8] & (1 << (offset % 8)))
  {
    return false; /* not null */
  }
  return true;
}

static PCPATCH *
#if PGSQL_VERSION < 120
pcpatch_from_point_array(ArrayType *array, FunctionCallInfoData *fcinfo)
#else
pcpatch_from_point_array(ArrayType *array, FunctionCallInfo fcinfo)
#endif
{
  int nelems;
  bits8 *bitmap;
  size_t offset = 0;
  int i;
  uint32 pcid = 0;
  PCPATCH *pa;
  PCPOINTLIST *pl;
  PCSCHEMA *schema = 0;

  /* How many things in our array? */
  nelems = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));

  /* PgSQL supplies a bitmap of which array entries are null */
  bitmap = ARR_NULLBITMAP(array);

  /* Empty array? Null return */
  if (nelems == 0)
    return NULL;

  /* Make our holder */
  pl = pc_pointlist_make(nelems);

  offset = 0;
  bitmap = ARR_NULLBITMAP(array);
  for (i = 0; i < nelems; i++)
  {
    /* Only work on non-NULL entries in the array */
    if (!array_get_isnull(bitmap, i))
    {
      SERIALIZED_POINT *serpt =
          (SERIALIZED_POINT *)(ARR_DATA_PTR(array) + offset);
      PCPOINT *pt;

      if (!schema)
      {
        schema = pc_schema_from_pcid(serpt->pcid, fcinfo);
      }

      if (!pcid)
      {
        pcid = serpt->pcid;
      }
      else if (pcid != serpt->pcid)
      {
        elog(ERROR, "pcpatch_from_point_array: pcid mismatch (%d != %d)",
             serpt->pcid, pcid);
      }

      pt = pc_point_deserialize(serpt, schema);
      if (!pt)
      {
        elog(ERROR, "pcpatch_from_point_array: point deserialization failed");
      }

      pc_pointlist_add_point(pl, pt);

      offset += INTALIGN(VARSIZE(serpt));
    }
  }

  if (pl->npoints == 0)
    return NULL;

  pa = pc_patch_from_pointlist(pl);
  pc_pointlist_free(pl);
  return pa;
}

static PCPATCH *
#if PGSQL_VERSION < 120
pcpatch_from_patch_array(ArrayType *array, FunctionCallInfoData *fcinfo)
#else
pcpatch_from_patch_array(ArrayType *array, FunctionCallInfo fcinfo)
#endif
{
  int nelems;
  bits8 *bitmap;
  size_t offset = 0;
  int i;
  uint32 pcid = 0;
  PCPATCH *pa;
  PCPATCH **palist;
  int numpatches = 0;
  PCSCHEMA *schema = 0;

  /* How many things in our array? */
  nelems = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));

  /* PgSQL supplies a bitmap of which array entries are null */
  bitmap = ARR_NULLBITMAP(array);

  /* Empty array? Null return */
  if (nelems == 0)
    return NULL;

  /* Make our temporary list of patches */
  palist = pcalloc(nelems * sizeof(PCPATCH *));

  /* Read the patches out of the array and deserialize */
  offset = 0;
  bitmap = ARR_NULLBITMAP(array);
  for (i = 0; i < nelems; i++)
  {
    /* Only work on non-NULL entries in the array */
    if (!array_get_isnull(bitmap, i))
    {
      SERIALIZED_PATCH *serpatch =
          (SERIALIZED_PATCH *)(ARR_DATA_PTR(array) + offset);

      if (!schema)
      {
        schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);
      }

      if (!pcid)
      {
        pcid = serpatch->pcid;
      }
      else if (pcid != serpatch->pcid)
      {
        elog(ERROR, "pcpatch_from_patch_array: pcid mismatch (%d != %d)",
             serpatch->pcid, pcid);
      }

      pa = pc_patch_deserialize(serpatch, schema);
      if (!pa)
      {
        elog(ERROR, "pcpatch_from_patch_array: patch deserialization failed");
      }

      palist[numpatches++] = pa;

      offset += INTALIGN(VARSIZE(serpatch));
    }
  }

  /* Can't do anything w/ NULL */
  if (numpatches == 0)
    return NULL;

  /* Pass to the lib to build the output patch from the list */
  pa = pc_patch_from_patchlist(palist, numpatches);

  /* Free the temporary patch list */
  for (i = 0; i < numpatches; i++)
  {
    pc_patch_free(palist[i]);
  }
  pcfree(palist);

  return pa;
}

PG_FUNCTION_INFO_V1(pcpatch_from_pcpatch_array);
Datum pcpatch_from_pcpatch_array(PG_FUNCTION_ARGS)
{
  ArrayType *array;
  PCPATCH *pa;
  SERIALIZED_PATCH *serpa;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  array = DatumGetArrayTypeP(PG_GETARG_DATUM(0));
  pa = pcpatch_from_patch_array(array, fcinfo);
  if (!pa)
    PG_RETURN_NULL();

  serpa = pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  PG_RETURN_POINTER(serpa);
}

PG_FUNCTION_INFO_V1(pcpatch_from_pcpoint_array);
Datum pcpatch_from_pcpoint_array(PG_FUNCTION_ARGS)
{
  ArrayType *array;
  PCPATCH *pa;
  SERIALIZED_PATCH *serpa;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();

  array = DatumGetArrayTypeP(PG_GETARG_DATUM(0));
  pa = pcpatch_from_point_array(array, fcinfo);
  if (!pa)
    PG_RETURN_NULL();

  serpa = pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  PG_RETURN_POINTER(serpa);
}

PG_FUNCTION_INFO_V1(pcpatch_from_float_array);
Datum pcpatch_from_float_array(PG_FUNCTION_ARGS)
{
  int i, ndims, nelems, npoints;
  float8 *vals;
  PCPATCH *pa;
  PCPOINTLIST *pl;
  SERIALIZED_PATCH *serpa;
  uint32 pcid = PG_GETARG_INT32(0);
  ArrayType *arrptr = PG_GETARG_ARRAYTYPE_P(1);
  PCSCHEMA *schema = pc_schema_from_pcid(pcid, fcinfo);

  if (!schema)
    elog(ERROR, "unable to load schema for pcid = %d", pcid);

  if (ARR_ELEMTYPE(arrptr) != FLOAT8OID)
    elog(ERROR, "array must be of float8[]");

  if (ARR_NDIM(arrptr) != 1)
    elog(ERROR, "float8[] must have one dimension");

  if (ARR_HASNULL(arrptr))
    elog(ERROR, "float8[] must not have null elements");

  ndims = schema->ndims;
  nelems = ARR_DIMS(arrptr)[0];

  if (nelems % ndims != 0)
  {
    elog(ERROR, "array dimensions do not match schema dimensions of pcid = %d",
         pcid);
  }

  npoints = nelems / ndims;

  vals = (float8 *)ARR_DATA_PTR(arrptr);
  pl = pc_pointlist_make(nelems);

  for (i = 0; i < npoints; ++i)
  {

    PCPOINT *pt = pc_point_from_double_array(schema, vals, i * ndims, ndims);
    pc_pointlist_add_point(pl, pt);
  }

  pa = pc_patch_from_pointlist(pl);
  pc_pointlist_free(pl);
  if (!pa)
    PG_RETURN_NULL();

  serpa = pc_patch_serialize(pa, NULL);

  pc_patch_free(pa);
  PG_RETURN_POINTER(serpa);
}

typedef struct
{
  ArrayBuildState *s;
} abs_trans;

PG_FUNCTION_INFO_V1(pointcloud_abs_in);
Datum pointcloud_abs_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                  errmsg("function pointcloud_abs_in not implemented")));
  PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(pointcloud_abs_out);
Datum pointcloud_abs_out(PG_FUNCTION_ARGS)
{
  ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                  errmsg("function pointcloud_abs_out not implemented")));
  PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(pointcloud_agg_transfn);
Datum pointcloud_agg_transfn(PG_FUNCTION_ARGS)
{
  Oid arg1_typeid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  MemoryContext aggcontext;
  abs_trans *a;
  ArrayBuildState *state;
  Datum elem;

  if (arg1_typeid == InvalidOid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                    errmsg("could not determine input data type")));

  if (!AggCheckCallContext(fcinfo, &aggcontext))
  {
    /* cannot be called directly because of dummy-type argument */
    elog(ERROR, "pointcloud_agg_transfn called in non-aggregate context");
    aggcontext = NULL; /* keep compiler quiet */
  }

  if (PG_ARGISNULL(0))
  {
    a = (abs_trans *)palloc(sizeof(abs_trans));
    a->s = NULL;
  }
  else
  {
    a = (abs_trans *)PG_GETARG_POINTER(0);
  }
  state = a->s;
  elem = PG_ARGISNULL(1) ? (Datum)0 : PG_GETARG_DATUM(1);
  state =
      accumArrayResult(state, elem, PG_ARGISNULL(1), arg1_typeid, aggcontext);
  a->s = state;

  PG_RETURN_POINTER(a);
}

static Datum pointcloud_agg_final(abs_trans *a, MemoryContext mctx,
                                  FunctionCallInfo fcinfo)
{
  ArrayBuildState *state;
  int dims[1];
  int lbs[1];
  state = a->s;
  dims[0] = state->nelems;
  lbs[0] = 1;
  return makeMdArrayResult(state, 1, dims, lbs, mctx, false);
}

PG_FUNCTION_INFO_V1(pcpoint_agg_final_array);
Datum pcpoint_agg_final_array(PG_FUNCTION_ARGS)
{
  abs_trans *a;
  Datum result = 0;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL(); /* returns null iff no input values */

  a = (abs_trans *)PG_GETARG_POINTER(0);

  result = pointcloud_agg_final(a, CurrentMemoryContext, fcinfo);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(pcpatch_agg_final_array);
Datum pcpatch_agg_final_array(PG_FUNCTION_ARGS)
{
  abs_trans *a;
  Datum result = 0;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL(); /* returns null iff no input values */

  a = (abs_trans *)PG_GETARG_POINTER(0);

  result = pointcloud_agg_final(a, CurrentMemoryContext, fcinfo);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(pcpoint_agg_final_pcpatch);
Datum pcpoint_agg_final_pcpatch(PG_FUNCTION_ARGS)
{
  ArrayType *array;
  abs_trans *a;
  PCPATCH *pa;
  SERIALIZED_PATCH *serpa;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL(); /* returns null iff no input values */

  a = (abs_trans *)PG_GETARG_POINTER(0);

  array =
      DatumGetArrayTypeP(pointcloud_agg_final(a, CurrentMemoryContext, fcinfo));
  pa = pcpatch_from_point_array(array, fcinfo);
  if (!pa)
    PG_RETURN_NULL();

  serpa = pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  PG_RETURN_POINTER(serpa);
}

PG_FUNCTION_INFO_V1(pcpatch_agg_final_pcpatch);
Datum pcpatch_agg_final_pcpatch(PG_FUNCTION_ARGS)
{
  ArrayType *array;
  abs_trans *a;
  PCPATCH *pa;
  SERIALIZED_PATCH *serpa;

  if (PG_ARGISNULL(0))
    PG_RETURN_NULL(); /* returns null iff no input values */

  a = (abs_trans *)PG_GETARG_POINTER(0);

  array =
      DatumGetArrayTypeP(pointcloud_agg_final(a, CurrentMemoryContext, fcinfo));
  pa = pcpatch_from_patch_array(array, fcinfo);
  if (!pa)
    PG_RETURN_NULL();

  serpa = pc_patch_serialize(pa, NULL);
  pc_patch_free(pa);
  PG_RETURN_POINTER(serpa);
}

PG_FUNCTION_INFO_V1(pcpatch_unnest);
Datum pcpatch_unnest(PG_FUNCTION_ARGS)
{
  typedef struct
  {
    int nextelem;
    int numelems;
    PCPOINTLIST *pointlist;
  } pcpatch_unnest_fctx;

  FuncCallContext *funcctx;
  pcpatch_unnest_fctx *fctx;
  MemoryContext oldcontext;

  /* stuff done only on the first call of the function */
  if (SRF_IS_FIRSTCALL())
  {
    PCPATCH *patch;
    SERIALIZED_PATCH *serpatch;

    /* create a function context for cross-call persistence */
    funcctx = SRF_FIRSTCALL_INIT();

    /*
     * switch to memory context appropriate for multiple function calls
     */
    oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /*
     * Get the patch value and detoast if needed.  We can't do this
     * earlier because if we have to detoast, we want the detoasted copy
     * to be in multi_call_memory_ctx, so it will go away when we're done
     * and not before.      (If no detoast happens, we assume the originally
     * passed array will stick around till then.)
     */
    serpatch = PG_GETARG_SERPATCH_P(0);

    /* The schema cache is not initialized at that time but we need the
     * constants cache
     */
    pointcloud_init_constants_cache();
    patch = pc_patch_deserialize(serpatch,
                                 pc_schema_from_pcid_uncached(serpatch->pcid));

    /* allocate memory for user context */
    fctx = (pcpatch_unnest_fctx *)palloc(sizeof(pcpatch_unnest_fctx));

    /* initialize state */
    fctx->nextelem = 0;
    fctx->numelems = patch->npoints;
    fctx->pointlist = pc_pointlist_from_patch(patch);

    /* save user context, switch back to function context */
    funcctx->user_fctx = fctx;
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  fctx = funcctx->user_fctx;

  if (fctx->nextelem < fctx->numelems)
  {
    Datum elem;
    PCPOINT *pt = pc_pointlist_get_point(fctx->pointlist, fctx->nextelem);
    SERIALIZED_POINT *serpt = pc_point_serialize(pt);
    fctx->nextelem++;
    elem = PointerGetDatum(serpt);
    SRF_RETURN_NEXT(funcctx, elem);
  }
  else
  {
    /* do when there is no more left */
    SRF_RETURN_DONE(funcctx);
  }
}

PG_FUNCTION_INFO_V1(pcpatch_uncompress);
Datum pcpatch_uncompress(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  PCSCHEMA *schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCPATCH *patch = pc_patch_deserialize(serpa, schema);
  SERIALIZED_PATCH *serpa_out = pc_patch_serialize_to_uncompressed(patch);
  pc_patch_free(patch);
  PG_RETURN_POINTER(serpa_out);
}

PG_FUNCTION_INFO_V1(pcpatch_compress);
Datum pcpatch_compress(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  text *compr_in_text = PG_GETARG_TEXT_P(1);
  char *compr_in = text_to_cstring(compr_in_text);
  text *config_in_text = PG_GETARG_TEXT_P(2);
  char *config_in = text_to_cstring(config_in_text);
  PCSCHEMA *schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCPATCH *patch_in = pc_patch_deserialize(serpa, schema);
  PCPATCH *pa = patch_in;
  SERIALIZED_PATCH *serpa_out;
  PCDIMSTATS *stats = NULL;
  int i;

  /* Uncompress first */
  if (patch_in->type != PC_NONE)
  {
    pa = pc_patch_uncompress(patch_in);
  }

  schema = pc_schema_clone(schema); /* we're going to modify it */

  /* Set compression scheme */
  if (*compr_in == '\0' || strcasecmp(compr_in, "auto") == 0)
  {
    /* keep schema defined compression */
  }
  else if (strcmp(compr_in, "dimensional") == 0)
  {
    {
      char *ptr = config_in;
      PCPATCH_DIMENSIONAL *pdl =
          pc_patch_dimensional_from_uncompressed((PCPATCH_UNCOMPRESSED *)pa);
      schema->compression = PC_DIMENSIONAL;
      stats = pc_dimstats_make(schema);
      pc_dimstats_update(stats, pdl);
      /* make sure to avoid stat updates (not sure if needed) */
      stats->total_points = PCDIMSTATS_MIN_SAMPLE + 1;

      /* Fill in per-dimension compression */
      if (*ptr)
        for (i = 0; i < stats->ndims; ++i)
        {
          PCDIMSTAT *stat = &(stats->stats[i]);
          /*pcinfo("ptr: %s", ptr);*/
          if (*ptr == ',' || strncmp(ptr, "auto", strlen("auto")) == 0)
          {
            /* leave auto-determined compression */
          }
          else if (strncmp(ptr, "rle", strlen("rle")) == 0)
          {
            stat->recommended_compression = PC_DIM_RLE;
          }
          else if (strncmp(ptr, "sigbits", strlen("sigbits")) == 0)
          {
            stat->recommended_compression = PC_DIM_SIGBITS;
          }
          else if (strncmp(ptr, "zlib", strlen("zlib")) == 0)
          {
            stat->recommended_compression = PC_DIM_ZLIB;
          }
          else
          {
            elog(ERROR,
                 "Unrecognized dimensional compression '%s'. Please specify "
                 "'auto', 'rle', 'sigbits' or 'zlib'",
                 ptr);
          }
          while (*ptr && *ptr != ',')
            ++ptr;
          if (!*ptr)
            break;
          else
            ++ptr;
        }

      if (pa != patch_in)
        pc_patch_free(pa);
      pa = (PCPATCH *)pc_patch_dimensional_compress(pdl, stats);
      pc_patch_dimensional_free(pdl);
    }
  }
  else if (strcmp(compr_in, "laz") == 0)
  {
    schema->compression = PC_LAZPERF;
  }
  else
  {
    elog(ERROR,
         "Unrecognized compression '%s'. Please specify 'auto','dimensional' "
         "or 'laz'",
         compr_in);
  }

  pa->schema = schema; /* install overridden schema */
  serpa_out = pc_patch_serialize(pa, stats);

  if (pa != patch_in)
    pc_patch_free(pa);
  pc_patch_free(patch_in);
  pc_schema_free(schema);

  PG_RETURN_POINTER(serpa_out);
}

PG_FUNCTION_INFO_V1(pcpatch_numpoints);
Datum pcpatch_numpoints(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETHEADER_SERPATCH_P(0);
  PG_RETURN_INT32(serpa->npoints);
}

PG_FUNCTION_INFO_V1(pcpatch_pointn);
Datum pcpatch_pointn(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt;
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  int32 n = PG_GETARG_INT32(1);
  PCSCHEMA *schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCPATCH *patch = pc_patch_deserialize(serpa, schema);
  PCPOINT *pt = NULL;
  if (patch)
  {
    pt = pc_patch_pointn(patch, n);
    pc_patch_free(patch);
  }
  if (!pt)
    PG_RETURN_NULL();
  serpt = pc_point_serialize(pt);
  pc_point_free(pt);
  PG_RETURN_POINTER(serpt);
}

PG_FUNCTION_INFO_V1(pcpatch_range);
Datum pcpatch_range(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpaout;
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  int32 first = PG_GETARG_INT32(1);
  int32 count = PG_GETARG_INT32(2);
  PCSCHEMA *schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCPATCH *patch = pc_patch_deserialize(serpa, schema);
  PCPATCH *patchout = NULL;
  if (patch)
  {
    patchout = pc_patch_range(patch, first, count);
    if (patchout != patch)
      pc_patch_free(patch);
  }
  if (!patchout)
    PG_RETURN_NULL();
  serpaout = pc_patch_serialize(patchout, NULL);
  pc_patch_free(patchout);
  PG_RETURN_POINTER(serpaout);
}

PG_FUNCTION_INFO_V1(pcpatch_pcid);
Datum pcpatch_pcid(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETHEADER_SERPATCH_P(0);
  PG_RETURN_INT32(serpa->pcid);
}

PG_FUNCTION_INFO_V1(pcpatch_summary);
Datum pcpatch_summary(PG_FUNCTION_ARGS)
{
  const int stats_size_guess = 400;
  SERIALIZED_PATCH *serpa;
  PCSCHEMA *schema;
  PCSTATS *stats;
  PCPATCH *patch = NULL;
  StringInfoData strdata;
  text *ret;
  const char *comma = "";
  int i;

  serpa = PG_GETHEADERX_SERPATCH_P(0, stats_size_guess);
  schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  if (serpa->compression == PC_DIMENSIONAL)
  {
    /* need full data to inspect per-dimension compression */
    /* NOTE: memory usage could be optimized to only fetch slices
     *       at specific offsets, but doesn't seem worth at this time
     *       See
     * https://github.com/pgpointcloud/pointcloud/pull/51#issuecomment-83592363
     */
    serpa = PG_GETARG_SERPATCH_P(0);
    patch = pc_patch_deserialize(serpa, schema);
  }
  else if (stats_size_guess < pc_stats_size(schema))
  {
    /* only need stats here */
    serpa = PG_GETHEADERX_SERPATCH_P(0, pc_stats_size(schema));
  }
  stats = pc_patch_stats_deserialize(schema, serpa->data);

  initStringInfo(&strdata);
  /* Make space for VARSIZ, see SET_VARSIZE below */
  appendStringInfoSpaces(&strdata, VARHDRSZ);

  appendStringInfo(&strdata,
                   "{"
                   "\"pcid\":%d, \"npts\":%d, \"srid\":%d, "
                   "\"compr\":\"%s\",\"dims\":[",
                   serpa->pcid, serpa->npoints, schema->srid,
                   pc_compression_name(serpa->compression));

  for (i = 0; i < schema->ndims; ++i)
  {
    PCDIMENSION *dim = schema->dims[i];
    PCBYTES bytes;
    double val;
    appendStringInfo(&strdata,
                     "%s{\"pos\":%d,\"name\":\"%s\",\"size\":%d"
                     ",\"type\":\"%s\"",
                     comma, dim->position, dim->name, dim->size,
                     pc_interpretation_string(dim->interpretation));

    /* Print per-dimension compression (if dimensional) */
    if (serpa->compression == PC_DIMENSIONAL)
    {
      bytes = ((PCPATCH_DIMENSIONAL *)patch)->bytes[i];
      switch (bytes.compression)
      {
      case PC_DIM_RLE:
        appendStringInfoString(&strdata, ",\"compr\":\"rle\"");
        break;
      case PC_DIM_SIGBITS:
        appendStringInfoString(&strdata, ",\"compr\":\"sigbits\"");
        break;
      case PC_DIM_ZLIB:
        appendStringInfoString(&strdata, ",\"compr\":\"zlib\"");
        break;
      case PC_DIM_NONE:
        appendStringInfoString(&strdata, ",\"compr\":\"none\"");
        break;
      default:
        appendStringInfo(&strdata, ",\"compr\":\"unknown(%d)\"",
                         bytes.compression);
        break;
      }
    }

    if (stats)
    {
      pc_point_get_double_by_name(&(stats->min), dim->name, &val);
      appendStringInfo(&strdata, ",\"stats\":{\"min\":%g", val);
      pc_point_get_double_by_name(&(stats->max), dim->name, &val);
      appendStringInfo(&strdata, ",\"max\":%g", val);
      pc_point_get_double_by_name(&(stats->avg), dim->name, &val);
      appendStringInfo(&strdata, ",\"avg\":%g}", val);
    }
    appendStringInfoString(&strdata, "}");
    comma = ",";
  }

  appendStringInfoString(&strdata, "]}");

  ret = (text *)strdata.data;
  SET_VARSIZE(ret, strdata.len);
  PG_RETURN_TEXT_P(ret);
}

PG_FUNCTION_INFO_V1(pcpatch_compression);
Datum pcpatch_compression(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETHEADER_SERPATCH_P(0);
  PG_RETURN_INT32(serpa->compression);
}

PG_FUNCTION_INFO_V1(pcpatch_intersects);
Datum pcpatch_intersects(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa1 = PG_GETHEADER_SERPATCH_P(0);
  SERIALIZED_PATCH *serpa2 = PG_GETHEADER_SERPATCH_P(1);

  if (serpa1->pcid != serpa2->pcid)
    elog(ERROR, "%s: pcid mismatch (%d != %d)", __func__, serpa1->pcid,
         serpa2->pcid);

  if (pc_bounds_intersects(&(serpa1->bounds), &(serpa2->bounds)))
  {
    PG_RETURN_BOOL(true);
  }
  PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(pcpatch_size);
Datum pcpatch_size(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  PG_RETURN_INT32(VARSIZE(serpa));
}

PG_FUNCTION_INFO_V1(pcpoint_size);
Datum pcpoint_size(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt = PG_GETARG_SERPOINT_P(0);
  PG_RETURN_INT32(VARSIZE(serpt));
}

PG_FUNCTION_INFO_V1(pcpoint_pcid);
Datum pcpoint_pcid(PG_FUNCTION_ARGS)
{
  SERIALIZED_POINT *serpt = PG_GETARG_SERPOINT_P(0);
  PG_RETURN_INT32(serpt->pcid);
}

PG_FUNCTION_INFO_V1(pc_version);
Datum pc_version(PG_FUNCTION_ARGS)
{
  text *version_text;
  char version[64];
  snprintf(version, 64, "%s", POINTCLOUD_VERSION);
  version_text = cstring_to_text(version);
  PG_RETURN_TEXT_P(version_text);
}

PG_FUNCTION_INFO_V1(pc_pgsql_version);
Datum pc_pgsql_version(PG_FUNCTION_ARGS)
{
  text *version_text;
  char version[12];
  snprintf(version, 12, "%d", PGSQL_VERSION);
  version_text = cstring_to_text(version);
  PG_RETURN_TEXT_P(version_text);
}

PG_FUNCTION_INFO_V1(pc_libxml2_version);
Datum pc_libxml2_version(PG_FUNCTION_ARGS)
{
  text *version_text;
  char version[64];
  snprintf(version, 64, "%s", LIBXML2_VERSION);
  version_text = cstring_to_text(version);
  PG_RETURN_TEXT_P(version_text);
}

PG_FUNCTION_INFO_V1(pc_lazperf_enabled);
Datum pc_lazperf_enabled(PG_FUNCTION_ARGS)
{
#ifdef HAVE_LAZPERF
  PG_RETURN_BOOL(true);
#else
  PG_RETURN_BOOL(false);
#endif
}

/**
 * Read a named dimension statistic from a PCPATCH
 * PC_PatchMax(patch pcpatch, dimname text) returns Numeric
 * PC_PatchMin(patch pcpatch, dimname text) returns Numeric
 * PC_PatchAvg(patch pcpatch, dimname text) returns Numeric
 * PC_PatchMax(patch pcpatch) returns PcPoint
 * PC_PatchMin(patch pcpatch) returns PcPoint
 * PC_PatchAvg(patch pcpatch) returns PcPoint
 */
PG_FUNCTION_INFO_V1(pcpatch_get_stat);
Datum pcpatch_get_stat(PG_FUNCTION_ARGS)
{
  static int stats_size_guess = 400;
  SERIALIZED_PATCH *serpa = PG_GETHEADERX_SERPATCH_P(0, stats_size_guess);
  PCSCHEMA *schema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  int32 statno = PG_GETARG_INT32(1);
  char *dim_str = 0;
  PCSTATS *stats;
  const PCPOINT *pt;
  SERIALIZED_POINT *serpt = NULL;
  float8 double_result;
  int rv = 1;

  if (PG_NARGS() > 2)
  {
    /* TODO: only get small slice ? */
    dim_str = text_to_cstring(PG_GETARG_TEXT_P(2));
  }

  if (stats_size_guess < pc_stats_size(schema))
  {
    serpa = PG_GETHEADERX_SERPATCH_P(0, pc_stats_size(schema));
  }

  stats = pc_patch_stats_deserialize(schema, serpa->data);

  if (!stats)
    PG_RETURN_NULL();

  /* Min */
  if (0 == statno)
    pt = &(stats->min);
  /* Max */
  else if (1 == statno)
    pt = &(stats->max);
  /* Avg */
  else if (2 == statno)
    pt = &(stats->avg);
  /* Unsupported */
  else
    elog(ERROR, "stat number \"%d\" is not supported", statno);

  /* empty dim string means we want the whole point */
  if (!dim_str)
  {
    serpt = pc_point_serialize(pt);
    pc_stats_free(stats);
    PG_RETURN_POINTER(serpt);
  }
  else
  {
    rv = pc_point_get_double_by_name(pt, dim_str, &double_result);
    pc_stats_free(stats);
    if (!rv)
    {
      elog(ERROR, "dimension \"%s\" does not exist in schema", dim_str);
      PG_RETURN_NULL();
    }
    pfree(dim_str);
    PG_RETURN_DATUM(
        DirectFunctionCall1(float8_numeric, Float8GetDatum(double_result)));
  }
}

/**
 * PC_FilterLessThan(patch pcpatch, dimname text, value) returns PcPatch
 * PC_FilterGreaterThan(patch pcpatch, dimname text, value) returns PcPatch
 * PC_FilterEquals(patch pcpatch, dimname text, value) returns PcPatch
 * PC_FilterBetween(patch pcpatch, dimname text, value1, value2) returns PcPatch
 */
PG_FUNCTION_INFO_V1(pcpatch_filter);
Datum pcpatch_filter(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpatch = PG_GETARG_SERPATCH_P(0);
  PCSCHEMA *schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);
  char *dim_name = text_to_cstring(PG_GETARG_TEXT_P(1));
  float8 value1 = PG_GETARG_FLOAT8(2);
  float8 value2 = PG_GETARG_FLOAT8(3);
  int32 mode = PG_GETARG_INT32(4);
  PCPATCH *patch;
  PCPATCH *patch_filtered = NULL;
  SERIALIZED_PATCH *serpatch_filtered;

  patch = pc_patch_deserialize(serpatch, schema);
  if (!patch)
  {
    elog(ERROR, "failed to deserialize patch");
    PG_RETURN_NULL();
  }

  switch (mode)
  {
  case 0:
    patch_filtered = pc_patch_filter_lt_by_name(patch, dim_name, value1);
    break;
  case 1:
    patch_filtered = pc_patch_filter_gt_by_name(patch, dim_name, value1);
    break;
  case 2:
    patch_filtered = pc_patch_filter_equal_by_name(patch, dim_name, value1);
    break;
  case 3:
    patch_filtered =
        pc_patch_filter_between_by_name(patch, dim_name, value1, value2);
    break;
  default:
    elog(ERROR, "unknown mode \"%d\"", mode);
  }

  pc_patch_free(patch);
  PG_FREE_IF_COPY(serpatch, 0);

  if (!patch_filtered)
  {
    elog(ERROR, "dimension \"%s\" does not exist", dim_name);
  }
  pfree(dim_name);

  /* Always treat zero-point patches as SQL NULL */
  if (patch_filtered->npoints <= 0)
  {
    pc_patch_free(patch_filtered);
    PG_RETURN_NULL();
  }

  serpatch_filtered = pc_patch_serialize(patch_filtered, NULL);
  pc_patch_free(patch_filtered);

  PG_RETURN_POINTER(serpatch_filtered);
}

const char **array_to_cstring_array(ArrayType *array, int *size)
{
  int i, j, offset = 0;
  int nelems = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  const char **cstring = nelems ? pcalloc(nelems * sizeof(char *)) : NULL;
  bits8 *bitmap = ARR_NULLBITMAP(array);
  for (i = j = 0; i < nelems; ++i)
  {
    text *array_text;
    if (array_get_isnull(bitmap, i))
      continue;

    array_text = (text *)(ARR_DATA_PTR(array) + offset);
    cstring[j++] = text_to_cstring(array_text);
    offset += INTALIGN(VARSIZE(array_text));
  }
  if (size)
    *size = j;
  return cstring;
}

void pc_cstring_array_free(const char **array, int nelems)
{
  int i;
  if (!array)
    return;
  for (i = 0; i < nelems; ++i)
    pfree((void *)array[i]);
  pcfree((void *)array);
}

/**
 * PC_Sort(patch pcpatch, dimname text[]) returns PcPatch
 */
PG_FUNCTION_INFO_V1(pcpatch_sort);
Datum pcpatch_sort(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpatch = PG_GETARG_SERPATCH_P(0);
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  PCSCHEMA *schema = NULL;
  PCPATCH *patch = NULL;
  PCPATCH *patch_sorted = NULL;
  SERIALIZED_PATCH *serpatch_sorted = NULL;

  int ndims;
  const char **dim_name = array_to_cstring_array(array, &ndims);
  if (!ndims)
  {
    pc_cstring_array_free(dim_name, ndims);
    PG_RETURN_POINTER(serpatch);
  }

  schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);

  patch = pc_patch_deserialize(serpatch, schema);
  if (patch)
    patch_sorted = pc_patch_sort(patch, dim_name, ndims);

  pc_cstring_array_free(dim_name, ndims);
  if (patch)
    pc_patch_free(patch);
  PG_FREE_IF_COPY(serpatch, 0);

  if (!patch_sorted)
    PG_RETURN_NULL();

  serpatch_sorted = pc_patch_serialize(patch_sorted, NULL);
  pc_patch_free(patch_sorted);
  PG_RETURN_POINTER(serpatch_sorted);
}

/** True/false if the patch is sorted on dimension */
PG_FUNCTION_INFO_V1(pcpatch_is_sorted);
Datum pcpatch_is_sorted(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  bool strict = PG_GETARG_BOOL(2);
  PCSCHEMA *schema = NULL;
  SERIALIZED_PATCH *serpatch = NULL;
  PCPATCH *patch = NULL;
  int ndims;
  uint32_t res;
  const char **dim_name = array_to_cstring_array(array, &ndims);
  if (!ndims)
  {
    pc_cstring_array_free(dim_name, ndims);
    PG_RETURN_BOOL(PC_TRUE);
  }
  serpatch = PG_GETARG_SERPATCH_P(0);
  schema = pc_schema_from_pcid(serpatch->pcid, fcinfo);
  patch = pc_patch_deserialize(serpatch, schema);

  res = pc_patch_is_sorted(patch, dim_name, ndims, strict);

  pc_cstring_array_free(dim_name, ndims);
  pc_patch_free(patch);

  if (res == PC_FAILURE - 1)
    elog(ERROR, "PC_IsSorted failed");

  PG_RETURN_BOOL(res);
}
