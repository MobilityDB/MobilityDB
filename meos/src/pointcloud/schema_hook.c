/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Schema-lookup hook implementation. See
 *   @c meos/include/pointcloud/meos_schema_hook.h for the rationale.
 */

#include <postgres.h>
#include <meos.h>
#include "pointcloud/meos_schema_hook.h"

meos_pc_schema_fn_t meos_pc_schema_fn = NULL;

PCSCHEMA *
meos_pc_schema(uint32_t pcid)
{
  if (! meos_pc_schema_fn)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "PCSCHEMA hook not installed — call mobilitydb_init (or set "
      "meos_pc_schema_fn in a MEOS embedder) before using pgpointcloud "
      "schema-dependent paths");
    return NULL;
  }
  return meos_pc_schema_fn(pcid);
}
