/***********************************************************************
 * pc_editor.c
 *
 *  Editor functions for points and patches in PgSQL.
 *
 *  Copyright (c) 2017 Oslandia
 *
 ***********************************************************************/

#include "pc_pgsql.h" /* Common PgSQL support for our type */

Datum pcpatch_setpcid(PG_FUNCTION_ARGS);
Datum pcpatch_transform(PG_FUNCTION_ARGS);

static SERIALIZED_PATCH *pcpatch_set_schema(SERIALIZED_PATCH *serpa,
                                            PCSCHEMA *oschema,
                                            PCSCHEMA *nschema, float8 def)
{
  SERIALIZED_PATCH *serpatch;
  PCPATCH *paout;

  if (pc_schema_same_dimensions(oschema, nschema))
  {
    // oschema and nschema have the same dimensions at the same
    // positions, so we can take a fast path and avoid the
    // point-by-point dimension-by-dimension copying

    if (oschema->compression == nschema->compression)
    {
      // no need to deserialize the patch
      serpatch = palloc(serpa->size);
      if (!serpatch)
        return NULL;
      memcpy(serpatch, serpa, serpa->size);
      serpatch->pcid = nschema->pcid;
      return serpatch;
    }
    else
    {
      paout = pc_patch_deserialize(serpa, oschema);
      if (!paout)
        return NULL;
      paout->schema = nschema;
    }
  }
  else
  {
    PCPATCH *patch;

    patch = pc_patch_deserialize(serpa, oschema);
    if (!patch)
      return NULL;

    paout = pc_patch_set_schema(patch, nschema, def);

    if (patch != paout)
      pc_patch_free(patch);

    if (!paout)
      return NULL;
  }

  serpatch = pc_patch_serialize(paout, NULL);
  pc_patch_free(paout);

  return serpatch;
}

PG_FUNCTION_INFO_V1(pcpatch_setpcid);
Datum pcpatch_setpcid(PG_FUNCTION_ARGS)
{
  SERIALIZED_PATCH *serpatch;
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  int32 pcid = PG_GETARG_INT32(1);
  float8 def = PG_GETARG_FLOAT8(2);
  PCSCHEMA *oschema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCSCHEMA *nschema = pc_schema_from_pcid(pcid, fcinfo);

  serpatch = pcpatch_set_schema(serpa, oschema, nschema, def);
  if (!serpatch)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(serpatch);
}

PG_FUNCTION_INFO_V1(pcpatch_transform);
Datum pcpatch_transform(PG_FUNCTION_ARGS)
{
  PCPATCH *patch, *paout;
  SERIALIZED_PATCH *serpatch;
  SERIALIZED_PATCH *serpa = PG_GETARG_SERPATCH_P(0);
  int32 pcid = PG_GETARG_INT32(1);
  float8 def = PG_GETARG_FLOAT8(2);
  PCSCHEMA *oschema = pc_schema_from_pcid(serpa->pcid, fcinfo);
  PCSCHEMA *nschema = pc_schema_from_pcid(pcid, fcinfo);

  patch = pc_patch_deserialize(serpa, oschema);
  if (!patch)
    PG_RETURN_NULL();

  paout = pc_patch_transform(patch, nschema, def);

  pc_patch_free(patch);

  if (!paout)
    PG_RETURN_NULL();

  serpatch = pc_patch_serialize(paout, NULL);
  pc_patch_free(paout);

  PG_RETURN_POINTER(serpatch);
}
