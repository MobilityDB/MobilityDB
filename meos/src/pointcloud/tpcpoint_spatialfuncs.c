/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Instant constructor and per-point spatial predicates for the
 *   tpcpoint temporal type.
 *
 * Provides the three entry-points needed for per-event streaming (e.g.
 * MobilityNebula NES operators): @ref tpointcloudinst_make,
 * @ref nad_tpcpoint_geo, and @ref eintersects_tpcpoint_geo.
 */

#include <assert.h>
#include <float.h>
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif

#include <pc_api.h>             /* PCSCHEMA (struct body with .srid) */

#include <meos.h>
#include <meos_geo.h>           /* GSERIALIZED, geompoint_make2d, geom_* */
#include <meos_internal.h>      /* tinstant_make, tinstant_value_p,
                                   VALIDATE_NOT_NULL */
#include <meos_pointcloud.h>    /* Pcpoint (typedef), PCSCHEMA (typedef),
                                   meos_pc_schema, pcpoint_get_x/y */
#include "temporal/temporal.h"  /* Temporal, TInstant */
#include "temporal/meos_catalog.h"  /* T_TPCPOINT */
#include "pointcloud/pcpoint.h" /* struct Pcpoint body (.pcid field) */

/*****************************************************************************
 * Instant constructor
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_constructor
 * @brief Return a tpcpoint instant from a pcpoint value and a timestamp
 * @param[in] pt Pcpoint value
 * @param[in] t Timestamp
 * @csqlfn #Tpointcloudinst_make()
 */
TInstant *
tpointcloudinst_make(const Pcpoint *pt, TimestampTz t)
{
  VALIDATE_NOT_NULL(pt, NULL);
  return tinstant_make(PointerGetDatum(pt), T_TPCPOINT, t);
}

/*****************************************************************************
 * Internal helpers
 *****************************************************************************/

/**
 * @brief Return a palloc'd 2D geometry point for the position in @p pt, or
 *   NULL when the schema lacks X or Y dimensions.
 */
static GSERIALIZED *
pcpoint_to_geompoint2d(const Pcpoint *pt)
{
  PCSCHEMA *schema = meos_pc_schema(pt->pcid);
  if (!schema)
    return NULL;
  double x = 0.0, y = 0.0;
  if (!pcpoint_get_x(pt, schema, &x) || !pcpoint_get_y(pt, schema, &y))
    return NULL;
  return geompoint_make2d((int32_t) schema->srid, x, y);
}

/**
 * @brief Return the Pcpoint payload of a tpcpoint instant.
 */
static const Pcpoint *
tpointcloudinst_pcpoint(const Temporal *temp)
{
  assert(temp->temptype == T_TPCPOINT);
  return (const Pcpoint *) DatumGetPointer(
    tinstant_value_p((const TInstant *) temp));
}

/*****************************************************************************
 * Spatial predicates and nearest-approach distance
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_ever
 * @brief Return true if the tpcpoint instant position intersects the geometry
 * @param[in] temp Temporal pointcloud value (single instant)
 * @param[in] gs Geometry
 * @csqlfn #Eintersects_tpcpoint_geo()
 */
bool
eintersects_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(temp, false);
  VALIDATE_NOT_NULL(gs, false);
  GSERIALIZED *probe = pcpoint_to_geompoint2d(tpointcloudinst_pcpoint(temp));
  if (!probe)
    return false;
  bool result = geom_intersects2d(probe, gs);
  pfree(probe);
  return result;
}

/**
 * @ingroup meos_pointcloud_dist
 * @brief Return the nearest-approach distance between a tpcpoint instant and
 *   a geometry
 * @param[in] temp Temporal pointcloud value (single instant)
 * @param[in] gs Geometry
 * @return @p DBL_MAX on error (missing X/Y dimensions or NULL input)
 * @csqlfn #NAD_tpcpoint_geo()
 */
double
nad_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(temp, DBL_MAX);
  VALIDATE_NOT_NULL(gs, DBL_MAX);
  GSERIALIZED *probe = pcpoint_to_geompoint2d(tpointcloudinst_pcpoint(temp));
  if (!probe)
    return DBL_MAX;
  double result = geom_distance2d(probe, gs);
  pfree(probe);
  return result;
}

/*****************************************************************************/
